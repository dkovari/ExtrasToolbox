classdef MercuryDevice < extras.hardware.TargetValueDevice
    
    %% Inherited from TargetValueDevice
    properties (SetObservable=true) %allow setting Target to same Target, that way message gets sent again
        Target = NaN %TargetPosition
    end
    properties (Access=protected)
        Internal_setTarget = false;
    end
    properties (SetAccess=protected,SetObservable=true,AbortSet=true)
        Value = NaN %Position
    end
    
    %% Mercury Device Communication
    properties (SetAccess=protected)
        BoardID
        Hub
    end
    properties (Access=protected)
        HubConnectedListener
        HubDeleteListener
    end
    
    %% Internal
    properties %(Access=protected)
        ValueTimer =timer('ObjectVisibility','off');%Timer used to periodically check Position value of the device
        LastPositionReadTime = -Inf;
    end
    properties (Constant, Access=protected)
        ValueTimerPeriod = 0.1;
        PositionMinimumPeriod = 0.09;
    end

    %% Device Properties
    properties (SetObservable=true,AbortSet=true,SetAccess=protected)
        OnTarget = true;
    end
    properties (SetObservable=true,AbortSet = true)
        AxisType
        
        %Motion properties
        Velocity = 0;
        Acceleration = 1;
    end
    properties (Access=protected)
        Internal_setVelocity = false;
        Internal_setAcceleration = false;
    end
    properties (SetAccess=protected,SetObservable=true,AbortSet = true)
        IsRotary = false;
        HasLimitSwitch = false;
        MotorScale = 1; %steps/unit
    end
    
    %% create/delete
    methods
        function this = MercuryDevice(HubPort,BoardID,AxisType)
            if isa(HubPort,'extras.hardware.PI.MercuryHub')
                this.Hub = HubPort;
            else
                this.Hub = extras.hardware.PI.MercuryHub.findHub(HubPort);
            end
            %this.Hub.IncrementReferenceCount;
            this.BoardID = BoardID;
            
            this.HubDeleteListener = addlistener(this.Hub,'ObjectBeingDestroyed',@(~,~) delete(this));
            
            %% add self to Hub list of devices
            if isKey(this.Hub.DeviceMap,this.BoardID)
                error('Board: %d is already associated with the MercuryHub connected to %s',this.BoardID,this.Hub.Port);
            end
            
            if ~ismember(this.BoardID,this.Hub.BoardList)
                warning('Board: % is not listed in the BoardList for MercuryHub connected to %s',this.BoardID,this.Hub.Port);
            end
            this.Hub.DeviceMap(this.BoardID) = this;
            
            %% Setup timer for checking position
            delete(this.ValueTimer);
            
            this.ValueTimer = timer('ObjectVisibility','off',...
                 'BusyMode','drop',...
                 'ExecutionMode','fixedRate',...
                 'Period',this.ValueTimerPeriod,...
                 'TimerFcn',@(~,~) this.ValueTimerCallback);
             
            %% Add listener for Hub connection changes
            this.HubConnectedListener = addlistener(this.Hub,'connected','PostSet',@(~,~) this.ConnectChanged);

            if this.Hub.connected
                
                %% Get Status
                this.Hub.sendCommand(this.BoardID,'TS,TP'); %tell status
                this.Hub.sendCommand(this.BoardID,'TT,TL,TY');
                
                %% if Hub is connected, start ValueTimer
                start(this.ValueTimer);
            end
            
            %% Set Axis Type
            if exist('AxisType','var')
                this.AxisType = AxisType;
            end
        end
        
        function delete(this)
            try
                stop(this.ValueTimer)
            catch
            end
            delete(this.ValueTimer);
            %% delete listeners
            delete(this.HubConnectedListener);
            delete(this.HubDeleteListener);
            
            %% delete self from list of devices
            this.Hub.DeviceMap.remove(this.BoardID);
            this.Hub.DecrementReferenceCount; %decrease Hub refererence count. If RefCount<1 hub will delete itself
        end
    end
    
    %% Set from Serial Parser
    methods (Hidden)
        function updateAcceleration(this,val)
            this.Internal_setAcceleration = true;
            this.Acceleration = val/this.MotorScale;
        	this.Internal_setAcceleration = false;
        end
        function updatePosition(this,val)
            this.Value = val/this.MotorScale;
            this.LastPositionReadTime = now;
        end
        function updateTarget(this,val)
            this.Internal_setTarget = true;
            this.Target = val/this.MotorScale;
            this.Internal_setTarget = false;
        end
        function updateProgrammedVelocity(this,val)
            this.Internal_setVelocity = true;
            this.Velocity = val/this.MotorScale;
        	this.Internal_setVelocity = false;
        end
        function updateStatus(this,status)
            status = regexp(status,'[0-9A-F]{2}','match');
            assert(numel(status)==6,'status must have 6 byte codes specified as hex chars');
            
            %% 1F LM629 Status
            %Bit 0
            %Bit 1
            %Bit 2: Trajectory Complete
            %ontarget = bitget(hex2dec(status{1}),3)
                this.OnTarget = bitget(hex2dec(status{1}),3); %appears to be OnTarget signal
            %Bit 3: Index Pulse recieved   
            %Bit 4
            %Bit 5
            %Bit 6
            %Bit 7
            
            %% 2F Internal operation flags
            
            %% 3F Motor Loop Flags
            
            %% 4F Signal Lines Status
            
            %% 5F Signal Lines Inputs
            
            %% 6F Error Codes

        end
    end

    %% Internal Use serial commands
    methods (Access=protected)
        function readPosition(this)
            if ~this.Hub.connected
                warning('Hub is not connected. Cannot read Position');
                return;
            end
            
            this.Hub.sendCommand(this.BoardID,'TP,TS'); %tell position, tell status
            
        end
    end
    
    %% Callbacks
    methods (Hidden)
        function ValueTimerCallback(this)
            if ~isvalid(this)
                return;
            end
            
            if ~this.Hub.connected
                stop(this.ValueTimer);
                return;
            end
            %(this.LastPositionReadTime)
            if (now-this.LastPositionReadTime)*24*3600 >= this.PositionMinimumPeriod
                this.readPosition;
            end
        end
        function ConnectChanged(this)
            if ~this.Hub.connected
                stop(this.ValueTimer);
            else
                if ~strcmpi(this.ValueTimer.Running,'on')
                    this.Hub.sendCommand(this.BoardID,'TS,TP'); %tell status
                    this.Hub.sendCommand(this.BoardID,'TT,TL,TY');
                    start(this.ValueTimer);
                end
            end
        end
    end
    
    %% Property get/set methods
    methods
        function set.Target(this,val)
            if this.Internal_setTarget
                this.Target = val;
                return;
            end
            
            this.Target = max(this.Limits(1),min(val,this.Limits(2)));
            if this.Hub.connected
                this.Hub.sendCommand(this.BoardID,[sprintf('MA%0.0f',this.Target*this.MotorScale),',TT']);
            end
        end
        function set.Velocity(this,val)
            this.Velocity =val;
            if ~this.Internal_setVelocity && this.Hub.connected
                this.Hub.sendCommand(this.BoardID,[sprintf('SV%0.0f',this.Velocity*this.MotorScale),',TY']);
            end
        end
        function set.Acceleration(this,val)
            this.Acceleration =val;
            if ~this.Internal_setAcceleration && this.Hub.connected
                this.Hub.sendCommand(this.BoardID,[sprintf('SA%0.0f',this.Acceleration*this.MotorScale),',TL']);
            end
        end
        
        function set.AxisType(this,AxType)
            switch upper(AxType)
                case 'M-126.PD2'
                    this.IsRotary = false;
                    this.MotorScale = 200000/20.0; %steps/mm
                    this.Units = 'mm';
                    this.Limits = [0,20];
                    this.AxisType = 'M-126.PD2';
                    this.HasLimitSwitch = true;
                case 'C-150.PD'
                    this.IsRotary = true;
                    this.MotorScale = 10000/(2.5*360); %steps/degree
                    this.Units = 'degrees';
                    this.Limits = [-Inf,Inf];
                    this.AxisType = 'C-150.PD';
                    this.HasLimitSwitch = false;
                otherwise
                    this.IsRotary = false;
                    this.MotorScale = 1;
                    this.Units = 'steps';
                    this.Limits = [-Inf,Inf];
                    this.AxisType = AxType;
                    this.HasLimitSwitch = true;
            end
            if this.Hub.connected
                this.Hub.sendCommand(this.BoardID,'TS,TP'); %tell status
                this.Hub.sendCommand(this.BoardID,'TT,TL,TY');
            end
        end
    end
    %% Public Methods
    methods
        function WaitForOnTarget(this)
            if ~this.Hub.connected
                return;
            end
            if isnan(this.OnTarget)
                error('OnTarget is NaN, wait canceled');
            end
            
            if ~ismember(this.BoardID,this.Hub.BoardList)
                error('Board: % is not listed in the BoardList for MercuryHub connected to %s. OnTarget will never change. Aborting Wait',this.BoardID,this.Hub.Port);
            end
            
            %stop ValueTimer
            stop(this.ValueTimer);
            
            while ~this.OnTarget
                this.Hub.sendCommand(this.BoardID,'TS,TP');
                drawnow limitrate; %let matlab process draw updated and callbacks
                pause(0.1); %give the message some time to be processed
            end
            if isnan(this.OnTarget)
                error('OnTarget is NaN, wait canceled');
            end
            
            %restart valueTimer
            start(this.ValueTimer);
  
        end
        function Reference(this)
            if ~this.Hub.connected
                return;
            end
            if this.HasLimitSwitch
                this.Hub.sendCommand(this.BoardID,'FE1');
            end
            this.WaitForOnTarget;
            this.Hub.sendCommand(this.BoardID,'DH');
        end
        function StopMotor(this)
            if ~this.Hub.connected
                return;
            end
            this.Hub.sendCommand(this.BoardID,'AB');
        end
    end
end