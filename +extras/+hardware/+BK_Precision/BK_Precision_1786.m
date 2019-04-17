classdef BK_Precision_1786 < matlab.mixin.SetGet & extras.hardware.SerialDevice & extras.hardware.TargetValueDevice
    
    
    properties(Access=private)
        ResponseTimeout = 0.5; %response timeout in seconds
        ValueCheckTimer = timer();
        ValueTimerPeriod = 0.75;
    end
    
    %% redefine TargetValue here because we want a custom set method
    properties (SetAccess=protected,SetObservable=true,AbortSet=true)
        Value;
    end
    properties (SetObservable=true) %allow setting Target to same Target, that way wr message gets sent again
        Target = [0,0];
    end
    properties(SetObservable,AbortSet)
        TargetVoltage = 0;
        TargetCurrent = 0;
    end
    properties(Access=private)
        internal_TargetVoltage = false;
        internal_TargetCurrent = false;

    end
    properties (SetAccess=private,SetObservable,AbortSet)
        ActualVoltage = 0;
        ActualCurrent = 0;
        
        CurrentLimited = false;
        VoltageLimited = false;
        ControlState = 'CV';
    end
    methods
        function set.TargetVoltage(this,val)
            if this.internal_TargetVoltage
                this.TargetVoltage = val;
            end
            this.Target(1) = val;
        end
        function set.TargetCurrent(this,val)
            if this.internal_TargetCurrent
                this.TargetCurrent = val;
            end
            this.Target(2) = val;
        end
    end
    methods(Access=private)
        function internal_updateTarget(this)
            this.internal_TargetVoltage = true;
            this.TargetVoltage = this.Target(1);
            this.internal_TargetVoltage = false;
            
            this.internal_TargetCurrent = true;
            this.TargetCurrent = this.Target(2);
            this.internal_TargetCurrent = false;
        end
        function internal_updateValue(this)
            this.ActualVoltage = this.Value(1);
            this.ActualCurrent = this.Value(2);
        end
    end
    
    %% Constructor/delete
    methods
        function this = BK_Precision_1786(Port)
            if ~exist('Port','var')
                Port = '';
            end
            %% Set Serial Properties
            this.BaudRate = 9600;
            this.DataBits = 8;
            this.StopBits = 1;
            this.Parity = 'none';
            this.Terminator = 'CR';
            
            this.BytesAvailableFcn = @(~,~) this.ProcessSerialBuffer();
            
            %% Devicename
            this.DeviceName = 'BK Precision 1786 - Power Supply';
            
            %% Value Properties
            set(this,'ValueSize',[1,2],...
                'ValueLabel',{'Voltage','Current'},...
                'Limits',{[0,30],[0,30]},...
                'Units',{'Volts','Amps'},...
                'Value',[0,0],...
                'Target',[0,0]);
            
            %% listeners
            addlistener(this,'Target','PostSet',@(~,~) this.internal_updateTarget());
            addlistener(this,'Value','PostSet',@(~,~) this.internal_updateValue());
            addlistener(this,'connected','PostSet',@(~,~) this.connection_changed());
            
            %% create value timer
            delete(this.ValueCheckTimer);
            this.ValueCheckTimer = timer('ObjectVisibility','off',...
                 'BusyMode','drop',...
                 'ExecutionMode','fixedRate',...
                 'Name','BK_Precision_1786_ValueTimer',...
                 'Period',this.ValueTimerPeriod,...
                 'TimerFcn',@(~,~) this.ValueTimerCallback);
             
             %% connect if needed
             if ~isempty(Port)
                 this.ConnectCOM(Port);
             end
        end
        
        function delete(this)
            stop(this.ValueCheckTimer)
            delete(this.ValueCheckTimer);
        end
    end
    
    %% Set Target value
    methods
        function set.Target(this,val)
            assert(numel(val)==2);
            if ~this.connected
                this.Target = [0,0];
                return;
            end
            
            %% set volt
            TV = round(val(1)*10);
            outv = sprintf('VOLT%03d',TV);
            this.fprintf(outv);
            this.waitforserial();
            
            this.Target(1) = TV/10;
            
            %% set current
            TC = round(val(2)*100);
            
            outc = sprintf('CURR%03d',TC);
            this.fprintf(outc);
            this.waitforserial();
            
            this.Target(2) = TC/100;
            
            
        end
    end
    
    %%
    methods (Hidden)
        function ProcessSerialBuffer(this)
        %called whenever a terminator is read in the serial buffer
        
            if ~isvalid(this)
                return;
            end
        
            if ~this.connected
                return;
            end
            
            if this.scom.BytesAvailable<=0
                return;
            end
            
            %% check response
            persistent last_resp;
            resp = this.fgetl(); %get current line
           
            switch(resp)
                case {'GETD','GMAX','OK'}
                    %do nothing
                otherwise %recieved numbers
                    if contains(resp,'VOLT')
                        %resp
                        TV = sscanf(resp,'VOLT%3d');
                        assert(numel(TV)==1,'Could not interpret target voltage. Revieved: %s',resp);
                        this.TargetVoltage = TV/10;
                    elseif contains(resp,'CURR')
                        %resp
                        TC = sscanf(resp,'CURR%3d');
                        assert(numel(TC)==1,'Could not interpret target current. Revieved: %s',resp);
                        this.TargetCurrent = TC/100;
                    else
                        switch last_resp
                            case 'GETD'
                                LL = sscanf(resp,'%3d%3d%1d',3);
                                assert(numel(LL)==3,'Incorrect number of arguments revieved while processing GETD. Revieved: %s',resp);
                                VV = [LL(1)/10,LL(2)/100];
                                CL = logical(LL(3));
                                this.Value = VV;
                                this.CurrentLimited = CL;
                                this.VoltageLimited = ~CL;
                                if CL
                                    this.ControlState = 'CC';
                                else
                                    this.ControlState = 'CV';
                                end
                            case 'GMAX'
                                LL = sscanf(resp,'%3d%3d',2);
                                assert(numel(LL)==2,'Incorrect number of arguments revieved while processing GMAX. Revieved: %s',resp);

                                this.Limits = {[0,LL(1)/10],[0,LL(2)/100]};
                            otherwise
                                error('Cannot process message: %s with last_resp: %s',resp,last_resp);
                        end
                    end
            end
            last_resp = resp;
            
                    
            
           
            
        end
    end
    
    %% overload validateConnection (Protected)
    methods (Access=protected)
        function validateConnection(this)
            %% request max
            this.fprintf('GMAX');
            this.waitforserial();

            %% Send Voltage
            this.fprintf('VOLT000');
            this.waitforserial();
            
            %% Send Current
            this.fprintf('CURR000');
            this.waitforserial();
            
            this.Target = [0,0];
        end
    end
    
    methods(Access=private)
        function waitforserial(this)
            t1=tic;
            while this.scom.BytesAvailable<=0
                if toc(t1)>this.ResponseTimeout
                    break;
                end
            end
        end
        function ValueTimerCallback(this)
            if ~isvalid(this)
                return;
            end
            if ~this.connected
                stop(this.ValueCheckTimer)
            end
            
            this.fprintf('GETD');
        end
        function connection_changed(this)
            if this.connected
                while this.scom.BytesAvailable > 0
                    this.ProcessSerialBuffer();
                end
                start(this.ValueCheckTimer);
            else
                stop(this.ValueCheckTimer);
            end
        end
    end
end