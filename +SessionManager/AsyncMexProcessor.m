classdef (Abstract) AsyncMexProcessor < extras.SessionManager.Session & extras.QueueDispatcher
% extras.SessionManager.AsyncMexProcessor
% Interface to Mex-based Asynchronous data processor
    
    %% Dependent Properties, linked to class
    properties (Dependent)
        RemainingTasks
        AvailableResults
        Running
    end
    methods
        function val = get.RemainingTasks(this)
            val = this.runMethod('remainingTasks');
        end
        function val = get.AvailableResults(this)
            val = this.runMethod('availableResults');
        end
        function val = get.Running(this)
            val = this.runMethod('running');
        end
    end

    %% Create/Delete
    methods
        function this = AsyncMexProcessor(MEX_NAME,varargin)
            this@extras.SessionManager.Session(MEX_NAME,varargin);
            
            %% setup error timer
            delete(this.ErrorCheckTimer);
            this.ErrorCheckTimer = ...
                timer('ObjectVisibility','off',...
                 'BusyMode','drop',...
                 'ExecutionMode','fixedSpacing',...
                 'Period',this.ErrorTimerPeriod,...
                 'TimerFcn',@(~,~) this.CheckErrorCallback);
             
             %% Setup the Results TImer
             delete(this.ResultsCheckTimer);
             this.ResultsCheckTimer = ...
                timer('ObjectVisibility','off',...
                 'BusyMode','drop',...
                 'ExecutionMode','fixedRate',...
                 'Period',this.ResultsTimerPeriod,...
                 'TimerFcn',@(~,~) this.ResultsTimerCallback);
        end
        
        function delete(this)
            %% stop and delete timers
            try
            stop(this.ResultsCheckTimer)
            catch
            end
            delete(this.ResultsCheckTimer);
            try
            stop(this.ErrorCheckTimer)
            catch
            end
            delete(this.ErrorCheckTimer);
        end
    end
    
    %% Task related methods
    methods
        function pushTask(this,varargin)
            this.runMethod('pushTask',varargin{:});
        end
        
        function n = numResultOutputArgs(this)
            n = this.runMethod('numResultOutputArgs');
        end
        
        function varargout = popResult(this)
            [varargout{1:nargout}] = this.runMethod('popResult');
        end
        
        function pauseProcessor(this)
            this.runMethod('pauseProcessor');
        end
        
        function resumeProcessor(this)
            this.runMethod('resumeProcessor');
            this.StartStopTimers();
        end
        
        function tf = errorThrown(this)
            tf = this.runMethod('errorThrown');
        end
        function err = getLastErrorMessage(this)
            err = this.runMethod('checkError');
        end
    end
    
    %% Error Checking
    events
        ErrorOccured
    end
    properties (SetAccess=protected,SetObservable=true,AbortSet = true)
        ErrorThrown = false;
    end
    properties(Access=protected)
        ErrorCheckTimer=timer();
        ErrorTimerPeriod = 0.2;
    end
    methods(Hidden)
        function CheckErrorCallback(this)
            if ~isvalid(this)
                return;
            end
            
            %% check errors
            if ~this.ErrorThrown
                if this.errorThrown()
                    this.ErrorThrown = true;
                    notify(this,'ErrorOccured');
                    stop(this.ErrorCheckTimer);
                end
            end
            
            if ~this.Running
                stop(this.ErrorCheckTimer);
            end
            
        end
    end
    
    %% Internal Usage Timer-related
    methods(Access=protected)
        function StartStopTimers(this)
            if isempty(this.QueueList)
                stop(this.ResultsCheckTimer);
                start(this.ErrorCheckTimer);
            else
                if ~strcmp(this.ResultsCheckTimer.running,'on')
                    start(this.ResultsCheckTimer);
                    stop(this.ErrorCheckTimer);
                end
            end
        end
    end
    
    %% QueueDispatcher Related & Overloads
    properties (Access=protected)
        ResultsCheckTimer = timer();
    end
    properties
        ResultsTimerPeriod = 0.05;
    end
    methods
        function set.ResultsTimerPeriod(this,val)
            stop(this.ResultsCheckTimer);
            this.ResultsCheckTimer.Period = val;
            this.ResultsTimerPeriod = val;
        end
        
        function registerQueue(this,queues)
            registerQueue@extras.QueueDispatcher(this,queues);
            
            this.StartStopTimers();
        end
        function deregisterQueue(this,queues)
            deregisterQueue@extras.QueueDispatcher(this,queues);
            
            this.StartStopTimers();
        end
    end
    methods(Hidden)
        function ResultsTimerCallback(this)
            if ~isvalid(this)
                return;
            end
            %% check errors
            if ~this.ErrorThrown
                if this.errorThrown
                    this.ErrorThrown = true;
                    notify(this,'ErrorOccured');
                    stop(this.ErrorCheckTimer);
                end
            end
            
            %% grab results
            nRes = this.AvailableResults;
            t = tic;
            for n=1:nRes
                
                narg = this.numResultOutputArgs();
                [out{1:narg}] = this.popResult;
                
                %dispatch data to queue list
                %note, if the associated queues are CallbackQueues then
                %each send command could take a while to process since
                %afterEach is immediately called.
                if narg==1
                    this.send(out{1});
                else
                    this.send(out); 
                end
                
                %only process for Timer Period duration
                if toc(t) > 0.9*this.ResultsTimerPeriod
                    break;
                end
            end
            
            if this.AvailableResults < 1 && ~this.Running
                stop(this.ResultsCheckTimer);
            end
        end
    end
    
    
end