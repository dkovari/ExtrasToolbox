classdef SerialDevice < matlab.mixin.SetGet & extras.widgets.mixin.HasDeviceName & matlab.mixin.CustomDisplay
%% simple class for managing a single serial device
% extend this class to add functionality
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.
        
    %% Customize Object Display
    methods (Access=protected)
        function pg = getPropertyGroups(this)
            if ~isscalar(this)
                pg = getPropertyGroups@matlab.mixin.CustomDisplay(this);
            else
                %% Basic:
                pg = getPropertyGroups@matlab.mixin.CustomDisplay(this);
                [pg.Title] = deal(...
                            sprintf(['\tProperties (Public Visible):',...
                                   '\n\t----------------------------']));
                %% Serial Interface Properties
                titleTxt = sprintf(['\n\tSerial Interface Properties (Internal Use):',...
                                    '\n\t-------------------------------------------']);
                pg = [pg,matlab.mixin.util.PropertyGroup({...
                    'BaudRate',...
                    'DataBits',...
                    'StopBits',...
                    'Parity',...
                    'Terminator',...
                    'BytesAvailableFcnMode',...
                    'BytesAvailableFcnCount',...
                    'Timeout',...
                    'ByteOrder',...
                    'InputBufferSize',...
                    'ReadAsyncMode',...
                    'OutputBufferSize'},titleTxt)];
            end
        end
    end

    %% Properties
    properties (SetAccess=protected)
        Port = '';
    end
    
    %% scom properties
    properties (SetAccess=protected,Hidden=true)
        scom = []; %internal serial com object, not pubically editable
    end
    properties (Hidden=true) %% SCOM interface props, should only be changed if you know what you're doing
        BaudRate (1,1) double {mustBePositive,mustBeFinite,mustBeInteger}= 9600; %Baud rate of serial connection
        DataBits (1,1) double {mustBeMember(DataBits,[5,6,7,8])}= 8; %data bit length of serial connection
        StopBits (1,1) double {mustBeMember(StopBits,[1,1.5,2])}= 1; %number of stop bits 
        Parity (1,:) char {mustBeMember(Parity,{'none','odd','even','mark','space'})} = 'none'; %parity 
        Terminator (1,:) char = 'CR';
        
        BytesAvailableFcnMode (1,:) char {mustBeMember(BytesAvailableFcnMode,{'terminator','byte'})}= 'terminator';
        BytesAvailableFcnCount (1,1) double {mustBePositive,mustBeFinite,mustBeInteger}= 48;
        Timeout (1,1) double {mustBePositive} = 10;
        ByteOrder (1,:) char {mustBeMember(ByteOrder,{'littleEndian','bigEndian'})} = 'littleEndian';
        
        InputBufferSize (1,1) double {mustBePositive,mustBeFinite,mustBeInteger}= 512;
        ReadAsyncMode (1,:) char {mustBeMember(ReadAsyncMode,{'continuous','manual'})} = 'continuous';
        
        OutputBufferSize (1,1) double {mustBePositive,mustBeFinite,mustBeInteger}= 512;
        
    end
    methods %set methods for scom properties
        function set.BaudRate(this,value)
            if ~isempty(this.scom)
                this.scom.BaudRate = value;
            end
            this.BaudRate = value;
        end
        function set.DataBits(this,value)
            if ~isempty(this.scom)
                this.scom.DataBits = value;
            end
            this.DataBits = value;
        end
        function set.StopBits(this,value)
            if ~isempty(this.scom)
                this.scom.StopBits = value;
            end
            this.StopBits = value;
        end
        function set.Parity(this,value)
            if ~isempty(this.scom)
                this.scom.Parity = value;
            end
            this.Parity = value;
        end
        function set.Terminator(this,value)
            if ~isempty(this.scom)
                this.scom.Terminator = value;
            end
            this.Terminator = value;
        end
        function set.BytesAvailableFcnMode(this,value)
            if ~isempty(this.scom)
                this.scom.BytesAvailableFcnMode = value;
            end
            this.BytesAvailableFcnMode = value;
        end
        function set.BytesAvailableFcnCount(this,value)
            if ~isempty(this.scom)
                this.scom.BytesAvailableFcnCount = value;
            end
            this.BytesAvailableFcnCount = value;
        end
        function set.Timeout(this,value)
            if ~isempty(this.scom)
                this.scom.Timeout = value;
            end
            this.Timeout = value;
        end
        function set.ByteOrder(this,value)
            if ~isempty(this.scom)
                this.scom.ByteOrder = value;
            end
            this.ByteOrder = value;
        end
        function set.InputBufferSize(this,value)
            if ~isempty(this.scom)
                this.scom.InputBufferSize = value;
            end
            this.InputBufferSize = value;
        end
        function set.ReadAsyncMode(this,value)
            if ~isempty(this.scom)
                this.scom.ReadAsyncMode = value;
            end
            this.ReadAsyncMode = value;
        end
        function set.OutputBufferSize(this,value)
            if ~isempty(this.scom)
                this.scom.OutputBufferSize = value;
            end
            this.OutputBufferSize = value;
        end
    end
    
    properties (Dependent)
        BytesAvailable
    end
    
    properties (Access=protected)
        BytesAvailableFcn;
    end
    
    events
        DataReceived
    end
    
    properties (SetAccess=protected,SetObservable=true,AbortSet=true)
        connected = false;
    end
    
    %% default object
    methods (Static, Sealed, Access = protected)
       function default_object = getDefaultScalarElement
           default_object = SerialDevice;
       end
    end
    
    %% Create/Delete
    methods
        function this = SerialDevice(port,varargin)
            % obj = SerialDevice(port)
            %       SerialDevice(port,'Name',value,...)
            %       SerialDevice('Port',Portvalue,...)
            
            this.connected = false;
            
            this.DeviceName = 'SerialDevice';
            
            %% Look for port in input arguments
            if nargin<1
                return;
            end
            
            if nargin == 1
                assert(ischar(port),'Specified Port must be a char array');
                this.Port = port;
            elseif ischar(port) && ~strcmpi(port,'port')
                this.Port = port;
            else
                assert(ischar(port),'first argument must be char array, either string specifying port value (e.g. ''COM1'') or atart of Name-Value pair');
                varargin = [port,varargin];
            end
            
            %% set other arguments
            set(this,varargin{:});
            
            %% If port specified, start serial
            if ~isempty(this.Port)
                this.ConnectCOM(this.Port);
            end
        end
        
        function delete(this)
            this.DisconnectCOM();
        end
        
    end
    
    %% Get Methods
    methods
        function val = get.BytesAvailable(this)
            if ~isvalid(this.scom)
                warning('serial device has not been initiated, cannot get BytesAvailable');
                val = 0;
                return;
            end
            val = this.scom.BytesAvailable;
        end
    end
    
    %% Set Methods
    methods
        function set.Port(this,port)
            assert(~this.connected,'Cannot change port while device is connected. DisconnectCOM then set port');
            assert(ischar(port),'Port must be a char array specifying com port name (e.g. ''COM4'')');
            this.Port = port;
        end
    end
    
    %% Overload f___() functions
    methods (Hidden) %Hidden because we don't want to advertise they are here
        function varargout = fgetl(this,varargin)
            [varargout{1:nargout}] = fgetl(this.scom,varargin{:});
        end
        function varargout = fgets(this,varargin)
            [varargout{1:nargout}] = fgets(this.scom,varargin{:});
        end
        function varargout = fread(this,varargin)
            [varargout{1:nargout}] = fread(this.scom,varargin{:});
        end
        function varargout = fscanf(this,varargin)
            [varargout{1:nargout}] = fscanf(this.scom,varargin{:});
        end
        function fwrite(this,varargin)
            fwrite(this.scom,varargin{:});
        end
        function fprintf(this,varargin)
            fprintf(this.scom,varargin{:});
        end
    end
    
    %% Connect/Disconnect Functions
    methods
        function ConnectCOM(this,PORT)
            if this.connected
                %already connected, disconnect and reconnect
                disp('Serial Port alread connected, trying to disconnect and reconnect');
                try
                    fclose(this.scom);
                catch
                    error('could not close scom');
                end
                this.connected = false;  
            end
            
            if nargin<2
                PORT = this.Port;
            end
            
            if isempty(PORT) || ~ischar(PORT)
                error('PORT must be char array specifying valid serial port (e.g. ''COM4'')');
            end
            
            %create serial port object
            delete(this.scom);
            this.scom = serial(PORT,...
                'Baudrate',this.BaudRate,...
                'DataBits',this.DataBits,...
                'StopBits',this.StopBits,...
                'Parity',this.Parity,...
                'Terminator',this.Terminator,...
                'BytesAvailableFcnMode',this.BytesAvailableFcnMode,...
                'BytesAvailableFcnCount',this.BytesAvailableFcnCount,...
                'Timeout',this.Timeout,...
                'ByteOrder',this.ByteOrder,...
                'InputBufferSize',this.InputBufferSize,...
                'ReadAsyncMode',this.ReadAsyncMode,...
                'OutputBufferSize',this.OutputBufferSize,...
                'BytesAvailableFcn',@(so,evt) this.evalSerialCallback(so,evt));

            try
                fopen(this.scom); %open com
            catch
                try
                   fclose(instrfind('Port',PORT));
                   fopen(this.scom);
                   assert(strcmpi(this.scom.Status,'open'));
                catch
                    this.connected = false;
                    %status = -1;
                    disp('could not connect to serial port');
                    return
                end
            end

            if strcmpi(this.scom.Status,'closed')
                this.connected = false;
                %status = -1;
                disp('could not connect to serial port');
                return                
            end
            this.Port = PORT;
            
            %% Call overloadable validation function
            try
                this.validateConnection();
            catch ME
                this.connected = false;
                try
                    fclose(this.scom);
                catch
                end
                %status = -1;
                disp('validation function threw an error');
                rethrow(ME);
            end
            
            %% statue good
            this.connected = true;
        end
        
        function DisconnectCOM(this)
            try
                fclose(this.scom);
                delete(this.scom);
                this.scom = [];
            catch
            end
            this.connected = false;
        end
    end
    
    %% Serial data callback
    methods (Hidden)
        function evalSerialCallback(this,so,evt)
            if ~isvalid(this)
                return;
            end
            
            %% Try to evaluate BytesAvailableFcn
            try
                hgfeval(this.BytesAvailableFcn,this,evt);
            catch ME
                disp(ME.getReport)
                warning('Error occured in serial callback, catching and throwing warning so that serial object does not disable callbacks');
            end
            
            notify(this,'DataReceived',extras.hardware.serialevent(so,evt))
            
        end
    end
    
    %% static
    methods (Static)
        function lCOM_Port = serialportlist()
        % serialportlist - List available serial ports
        %   Function should work for older version of matlab
        % Output:
        %   Returns cell array of available serial ports.
        %   If none are available returns empty cell
        
        lCOM_Port = convertStringsToChars(seriallist);
        if ischar(lCOM_Port)
            lCOM_Port = {lCOM_Port};
        end
        
        %% OLD CODE
%             try
%                 s=serial('IMPOSSIBLE_NAME_ON_PORT');fopen(s); 
%             catch MExcept
%                 %disp(MExcept)
%                 lErrMsg = MExcept.message;
%             end
% 
%             %Start of the COM available port
%             lIndex1 = strfind(lErrMsg,'COM');
%             %End of COM available port
%             lIndex2 = strfind(lErrMsg,'Use')-3;
% 
%             lComStr = lErrMsg(lIndex1:lIndex2);
% 
%             %Parse the resulting string
%             lIndexDot = strfind(lComStr,',');
% 
%             % If no Port are available
%             if isempty(lIndex1)
%                 lCOM_Port = {};
%                 return;
%             end
% 
%             % If only one Port is available
%             if isempty(lIndexDot)
%                 lCOM_Port{1}=lComStr;
%                 return;
%             end
% 
%             lCOM_Port = cell(numel(lIndexDot)+1,1);%lComStr(1:lIndexDot(1)-1);
% 
%             for i=1:numel(lIndexDot)+1
%                 % First One
%                 if (i==1)
%                     lCOM_Port{1} = lComStr(1:lIndexDot(i)-1);
%                 % Last One
%                 elseif (i==numel(lIndexDot)+1)
%                     lCOM_Port{i} = lComStr(lIndexDot(i-1)+2:end);       
%                 % Others
%                 else
%                     lCOM_Port{i} = lComStr(lIndexDot(i-1)+2:lIndexDot(i)-1);
%                 end
%             end
         end
    end
    
    %% overload these functions to change behavior
    methods (Access=protected)
        function validateConnection(this)
        % Called when ConnectCOM has successfully created com port
        % connection, but before this.connected is set to true.
        % overload this function to throw errors if there is a problem with
        % the serial device (e.g. it doesn't respond to a certain command,
        % etc.)
            %do nothing
        end
    end
    
%     %% Sealed
%     methods (Sealed)
%         function tf = eq(A,B)
%             tf = eq@handle(A,B);
%         end
%         function tf = ne(A,B)
%             tf = ne@handle(A,B);
%         end
%     end
end