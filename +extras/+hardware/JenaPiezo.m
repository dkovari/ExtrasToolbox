classdef JenaPiezo < extras.hardware.abstract.ObjectiveScanner & extras.hardware.SerialDevice
%% Class for interactfacing with a JenaSystems Piezo Objective scanner
% Communication with the device is handeled entirely by MATLAB via the
% serial port
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.   

    %% redefine TargetValue here because we want a custom set method
    properties (SetObservable=true) %allow setting Target to same Target, that way wr message gets sent again
        Target = NaN
    end
    properties (SetAccess=protected,SetObservable=true,AbortSet=true)
        Value = NaN
        UpdatedAfterTargetChange = false;
    end
    
    %%
    properties (Access=protected,SetObservable=true,AbortSet=true)
        EnableSoftServo = false;
    end
    
    %% Internal Use
    properties (Access=protected)
        DataListener
        ConnectListener
        ValueTimer = timer;
        LastValueCheck = -Inf;
        AfterTargetChange = false;
    end
    
    %% Create/Delete
    methods
        function this = JenaPiezo(Port)
            
            if ~exist('Port','var')
                Port = '';
            end
            
            %default serialdevice construction (do not connect yet)
            this@extras.hardware.SerialDevice();
            
            this.Port = Port;
            %% Device Info
            this.DeviceName = 'Jena Piezo';
            this.Units = 'µm';
            this.Limits = [0,99.99];
            this.ValueSize = [1,1];
            
            %% Serial Com parameters
            this.BaudRate = 9600;
            this.DataBits = 8;
            this.StopBits = 1;
            this.Parity = 'none';
            this.Terminator = 'CR/LF';
            
            %% Subscribe to ByteAvailable Notifications
            this.DataListener = addlistener(this,'DataRecieved',@(~,~) this.ParseMessage);
            
            %% Subscribe to Connection changes
            this.ConnectListener = addlistener(this,'connected','PostSet', @(~,~) this.ConnectChange());
            
            %% Create Timer to check value
            this.ValueTimer = timer('BusyMode','drop',...
                'ExecutionMode','fixedSpacing',...
                'Period',1,...
                'StartDelay',2,...
                'ObjectVisibility','off',...
                'TimerFcn',@(~,~) this.TimerCallback);
            
            %% Connect if port specified
            if ~isempty(this.Port)
                this.ConnectCOM()
                
            end
            
            %% check connection
            this.ConnectChange();
        end
        
        function delete(this)
            try
                stop(this.ValueTimer)
            catch
            end
            delete(this.ValueTimer)
            
            delete(this.DataListener)
            delete(this.ConnectListener);
        end
    end
    
    %% Callbacks
    methods (Hidden)
        function ParseMessage(this)
            t = tic;
            while this.BytesAvailable > 1
                if toc(t)>10
                    error('Timed out while processing Serial messages')
                end
                
                msg = fscanf(this.scom);
                [rx,ex] = regexp(msg,'(err|rd),[-\d\.]+');
                for n=1:numel(rx)
                    if strcmpi('er',msg(rx(n):rx(n)+1)) %error
                        err = sscanf(msg(rx(n):ex(n)),'err,%d');
                        switch(err)
                            case 1
                                warning('Send Error: unknown command');
                            case 2
                                warning('Send Error: too many characters in the command');
                            case 3
                                warning('Send Error: too many characters in the parameter');
                            case 4
                                warning('Send Error: too many parameter');
                            case 5
                                warning('Send Error: wrong character in parameter');
                            case 6
                                warning('Send Error: wrong separator');
                            case 7
                                warning('Reciever Error: overload');
                            otherwise
                                warning('Error message was recieved but not intereted: %s',msg(rx(n):ex(n)));
                        end
                    else %read
                        val = sscanf(msg(rx(n):ex(n)),'rd,%f');
                        if isempty(val)
                            warning('Could not interpret read response: %s',msg(rx(n):ex(n)));
                        else
                            this.Value = val;
                            
                            if this.AfterTargetChange
                                this.UpdatedAfterTargetChange = true;
                            end
                        end
                    end
                end
            end
        end
        
        function ConnectChange(this)
            if ~this.connected %not connected
                this.Value = NaN;
                this.Target = NaN;
                
                try
                    stop(this.ValueTimer)
                catch
                end
            else
                this.fprintf('i1');
                this.fprintf('cl');
                this.fprintf('rd');
                this.LastValueCheck = now;
                pause(0.2);
                this.Target = this.Value;
                
                try
                    start(this.ValueTimer)
                catch ME
                    if ~strcmpi(ME.identifier,'MATLAB:timer:alreadystarted')
                        warning('Could not start ValueTimer');
                        disp(ME)
                    end
                end
            end
        end
        
        function TimerCallback(this)
            if ~isvalid(this)
                return;
            end
            
            if ~this.connected
                try
                    stop(this.ValueTimer);
                catch
                end
                return
            end
            
            if isempty(this.Target) && ~isempty(this.Value)
                this.Target = this.Value;
                return;
            end
            
            if (now-this.LastValueCheck)*24*3600 > 0.9
                this.fprintf('rd');
                this.LastValueCheck = now;
            end
            
            if this.EnableSoftServo
                if abs(this.Value-this.Target)>0.02
                    this.fprintf('wr,%0.2f',this.Target);
                end
            end
        end
    end
    
    %% Set Methods
    methods
        function set.Target(this,val)
            
            if ~this.connected
                this.Target = NaN;
                return;
            end
            
            if isempty(val) || isnan(val)
                warning('Target must be a number 0<=V<100');
                return;
            end
            
            val = round(max(0,min(val,99.99)),2);
            this.fprintf('wr,%0.2f',val);
            
            this.AfterTargetChange = true;
            this.UpdatedAfterTargetChange = false;
            
            this.fprintf('rd');
            this.LastValueCheck = now;
            
            this.Target = val;
        end
    end
    
    
    %% Override
    methods
        function S = struct(this)
        %convert public properties inherited from TargetValueDevice Class
        %into struct and include special feature of JenaPiezo
            S = struct@extras.hardware.abstract.ObjectiveScanner(this);
            for n=1:numel(this)
                S(n).EnableSoftServo = this(n).EnableSoftServo;
            end
        end
    end
    
    
end