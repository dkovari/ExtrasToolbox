classdef PI_E665_Piezo < extras.hardware.TargetValueDevice & extras.hardware.SerialDevice
%% Class for interfacing with a PI E-665 Piezo controller via PI's C API
% Calls to the API are made using MATLAB's legacy calllib() functions
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.

    %% redefine TargetValue here because we want a custom set method
    properties (SetObservable=true) %allow setting Target to same Target, that way the command gets sent again
        Target = 0;
    end
    methods %set Target value
        function set.Target(this,val)
            if this.SetTargetInternal
                this.Target = val;
                return;
            end
            
            assert(isnumeric(val)&&isscalar(val),'Target must be numeric scalar');
            val = max(this.Limits(1),min(val,this.Limits(2)));
            
            if ~this.connected
                warning('Device is not connected. Cannot set Target');
                this.Target = NaN;
                return;
            end
            
            status = calllib(this.LibName,'E816_MOV',this.ID_e816,this.AxisID,val);
            if ~status
                warning('Could not set E816 to target position');
                this.Target = NaN;
                return;
            end
            
            stop(this.ValueTimer);
            try
            start(this.OnTargetTimer);
            catch
            end
            
            this.Target = val;
            
        end
    end
    properties (SetAccess=protected,SetObservable=true,AbortSet=true)
        Value = NaN
    end
   
%% Inherited from SerialDevice
%     properties (SetObservable=true,AbortSet=true)
%         Port = '';
%         BaudRate = 9600;
%     end
%     methods %Set methods
%         function set.Port(this,val)
%             assert(~this.connected,'Cannot change Port while device is connected call DisconnectCOM first');
%             assert(ischar(val),'Port must be valid char array specifying port name (e.g. ''COM5'')');
%             this.Port = val;
%         end
%         function set.BaudRate(this,val)
%             assert(~this.connected,'Cannot change BaudRate while device is connected call DisconnectCOM first');
%             assert(isnumeric(val)&&isscalar(val)&&~isnan(val),'BaudRate must be numeric scalar');
%             this.BaudRate = val;
%         end
%     end
%     properties(SetAccess=protected,SetObservable=true,AbortSet=true)
%         connected = false;
%     end
    
    %% Dependent
    properties(Dependent)
        OnTargetState;
    end
    methods
        function ret = get.OnTargetState(this)
            if ~this.connected
                warning('Device is not connected, cannot get OnTargetState');
                ret = NaN;
                return;
            end
            if strcmpi(this.OnTargetTimer.Running,'on') %use internal state if timer is running
                ret = this.OnTargetState_I;
            else
                [status,~,ret] = calllib(this.LibName,'E816_qONT',this.ID_e816,this.AxisID,0);
                if ~status
                    warning('could not get updated E816 OnTargetStatus');
                    ret = NaN;
                end
            end
        end
    end
    
    events
        OnTargetAfterChange
    end
    
    %% internal properties
    properties(Access=protected)
        LibName = 'E816_DLL';
        ID_e816 = -1;
        AxisID = 'A';
        
        OnTargetTimer = timer;
        OnTargetTimerPeriod = 0.05;
        OnTargetState_I = false;
        
        ValueTimer = timer
        ValueTimerPeriod = 1;
        
        SetTargetInternal = false;
    end
    
    %% Create/Delete
    methods
        function this = PI_E665_Piezo(Port,BaudRate)
            
            %% Inherited properties that need settings
            this.Units = 'µm';
            this.Limits = [0,100];
            this.ValueSize = [1,1];
            this.DeviceName='E819 Piezo';
            this.ValueLabels = '';
            
            %% PORT info
            if exist('Port','var')
                this.Port = Port;
            end
            if exist('Baud','var')
                this.BaudRate = BaudRate;
            end
            
            %% Load Library
            thisdir = fileparts(mfilename('fullpath'));
            switch computer('arch')
                case 'win64'
                    libfile = fullfile(thisdir,'E816_DLL_x64.dll');
                case 'win32'
                    libfile = fullfile(thisdir,'E816_DLL.dll');
                otherwise
                    error('The E816class is designed to work with windows 32 or 64 bit os');
            end
            headerfile = fullfile(thisdir,'E816_DLL.h');
            
            if(~libisloaded(this.LibName))
                disp('Loading the E816 Library');
                loadlibrary(libfile,headerfile,'alias',this.LibName);
            end
            
            %% Setup OnTargetTimer
            delete(this.OnTargetTimer)
            this.OnTargetTimer = timer(...
                'BusyMode','drop',...
                'ExecutionMode','fixedRate',...
                'ObjectVisibility','off',...
                'Period',this.ValueTimerPeriod,...
                'TimerFcn',@(~,~) this.OnTargetTimerCallback);
            
            %% Setup ValueTimer
            delete(this.ValueTimer)
            this.ValueTimer = timer(...
                'BusyMode','drop',...
                'ExecutionMode','fixedRate',...
                'ObjectVisibility','off',...
                'Period',this.OnTargetTimerPeriod,...
                'TimerFcn',@(~,~) this.ValueTimerCallback,...
                'StartDelay',this.OnTargetTimerPeriod);
            
            %% Connect if port was specified
            if ~isempty(this.Port)
                this.ConnectCOM()
            end
        end
        
        function delete(this)
            
            %% Delete valuetimer
            stop(this.ValueTimer);
            delete(this.ValueTimer);
            
            %% Get Rid of on target timer
            stop(this.OnTargetTimer);
            delete(this.OnTargetTimer);
            
            %% Close connection to PI library
            if libisloaded(this.LibName) %wrap in if so that we don't do this if library wasnt loaded because of an error during construction
                this.DisconnectCOM();
            end
        end
    end
    
    %% Other Methods
    methods
        function ConnectCOM(this,Port,BaudRate)
            if this.connected %close if already connected
                %already connected, disconnect and reconnect
                disp('Closing connection to E816');
                calllib(this.LibName, 'E816_CloseConnection',this.ID_e816);
                this.connected = false;  
            end
            
            %% Set Port and baud rate if specified
            if exist('Port','var')
                this.Port = Port;
            end
            if exist('Baud','var')
                this.BaudRate = BaudRate;
            end
            
            if isempty(this.Port)
                error('Port must be specified');
            end
            
            %% Convert port name to number
            PORT = sscanf(upper(this.Port),'COM%d');
            if isempty(this.Port)
                error('Couldnt read port number for: %s',this.Port);
            end
            
            %% Attempt Connection
            fprintf('Attempting to connect to E816 on COM%d at BAUD=%d\n',PORT,this.BaudRate);
            this.ID_e816 = calllib(this.LibName, 'E816_ConnectRS232',PORT,this.BaudRate);
            
            if this.ID_e816 == -1
                error('Could not connect to E816 controller. Make sure it is plugged in. Check device manager to make sure COM%d is listed',PORT);
            end
            
            %% query the piezo controller to make sure everything is working
            % preload return variable
            idn = blanks(100);
            % query Identification string
            [~,idn] = calllib(this.LibName,'E816_qIDN',this.ID_e816,idn,100);
            fprintf('Connected to %s\n',idn);
            % query baud rate
            bdr = 0;
            [~,bdr] = calllib(this.LibName,'E816_qBDR',this.ID_e816,bdr);
            fprintf('Connection speed: %d baud\n',bdr);
            this.BaudRate = bdr;
            
            this.connected = true;
            
            %% Init Piezo
            this.InitPiezo();
        end
        
        function DisconnectCOM(this)
            
            try
            stop(this.OnTargetTimer);
            catch
            end

            try
                stop(this.ValueTimer);
            catch
            end
            
            try
                calllib(this.LibName,'E816_CloseConnection',this.ID_e816);
            catch
                warning('Could not close connection');
            end
            
            this.connected = false;
        end

        function WaitForOnTarget(this)
            ont = false;
            t=tic;
            while(~ont)
                if toc(t)>3 %time out after 3 sec
                    return;
                end
                ont=this.OnTargetState();
                if isnan(ont)
                    return;
                end
                pause(0.001);
            end
        end
    end
    
    %% Callbacks
    methods (Hidden)
        function OnTargetTimerCallback(this)
            if ~isvalid(this)
                return;
            end
            if ~this.connected
                warning('Device is not connected, cannot get OnTargetState');
                stop(this.OnTargetTimer);
                return;
            end
            [status,~,this.OnTargetState_I] = calllib(this.LibName,'E816_qONT',this.ID_e816,this.AxisID,0);
            if ~status
                warning('could not get updated E816 OnTargetStatus');
                stop(this.OnTargetTimer);
                this.OnTargetState_I = NaN;
                return;
            end

            if this.OnTargetState_I
                stop(this.OnTargetTimer);
                %check value
                [status,~,val] = calllib(this.LibName,'E816_qPOS',this.ID_e816,this.AxisID,0);
                if ~status
                    warning('could not get updated E816 position');
                    this.Value = NaN;
                else
                    this.Value = val;
                end
                notify(this,'OnTargetAfterChange');
                start(this.ValueTimer);
            end
        end
        
        function ValueTimerCallback(this)
            if ~isvalid(this)
                return;
            end
            
            if ~this.connected
                stop(this.ValueTimer);
                return;
            end
            
            [status,~,val] = calllib(this.LibName,'E816_qPOS',this.ID_e816,this.AxisID,0);
            if ~status
                warning('could not get updated E816 position');
                this.Value = NaN;
            end
            this.Value = val;
        end
    end

    %% internal methods
    methods(Access=protected)
        function InitPiezo(this)
            %servo on
            calllib(this.LibName,'E816_SVO',this.ID_e816,this.AxisID,true);
            
            [status,~,val] = calllib(this.LibName,'E816_qPOS',this.ID_e816,this.AxisID,0);
            if ~status
                warning('could not get updated E816 position');
                this.Value = NaN;
            end
            this.Value = val;
            
            this.SetTargetInternal = true;
            this.Target = this.Value;
            this.SetTargetInternal = false;

            %% start value timer
            start(this.ValueTimer);
        end

    end
end