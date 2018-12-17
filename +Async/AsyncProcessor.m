classdef (Abstract) AsyncProcessor < extras.SessionManager.Session & extras.QueueDispatcher
%% extras.SessionManager.AsyncMexProcessor
% Interface to Mex-based Asynchronous data processor
%   This class creates the interface to a MEX SessionManager object
%   providing the thread processor.
%   
%  The associated MEX SessionManager object should be derived from
%   mex::AsyncProcessor defined in mexAsyncProcessor.h
%
%   Communication from MATLAB to the processor object is handed by
%   background timers.
%
%   Users can add tasks to the processor queue using pushTask()
%
%   Results can be retrieved via the extras.QueueDispatcher system.
%       Users should create an extras.Queue type object and subscribe to
%       events using the method
%           AsyncProcessor.registerQueue(YOUR_QUEUE)
%       See extras.QueueDispatcher for more details
%% Copyright 2018 Daniel T. Kovari, Emory University
%   All rights reserved.

    %% Public Properties
    properties(SetObservable=true,AbortSet=true)
        Name = 'AsyncProcessor'; %Human-readable name for the processor. (USER EDITABLE) The waitbar that is created when the object is delete before the tasks have finished will use this name in its message
    end

    %% Dependent Properties, linked to class
    properties (Dependent)
        RemainingTasks %number of remaining tasks
        AvailableResults %number of available results
        Running %T/F is thread running
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


    %% Error Related Items
    events
        ErrorOccured %Event thrown when error has occured in the processor
    end
    properties (SetObservable=true,AbortSet=true)
        ErrorCheckTimerPeriod = 0.75; %period at which to check for errors
    end
    methods %set period
        function set.ErrorCheckTimerPeriod(this,val)
            assert(isscalar(val)&&isnumeric(val)&&val>0,'ErrorCheckTimerPeriod must be numeric scalar with value>0');
            stop(this.ErrorCheckTimer)
            this.ErrorCheckTimer.Period = val;
            this.ErrorCheckTimerPeriod = val;
            this.restartErrorCheckTimer();
        end
    end
    properties(SetAccess=protected,SetObservable=true,AbortSet=true)
        LastErrorMessage %struct containing last error message
    end
    properties (Access=protected)
        ErrorCheckTimer = timer('ObjectVisibility','off',...
         'BusyMode','drop',...
         'ExecutionMode','fixedSpacing',...
         'Tag','AsyncProcessor-ErrorCheckTimer');
    end
    methods (Access=protected)
        function restartErrorCheckTimer(this)
        %internal use - Restart the error check timer
            
            if strcmpi(this.ErrorCheckTimer.Running,'on') %timer is already running
                return;
            end
            
            if this.wasErrorThrown()
                this.DispatchErrorMessage();
            end

            try
                start(this.ErrorCheckTimer);
            catch ME
                disp(ME.getReport);
                warning('could not restard ErrorCheckTimer');
            end
        end
        function DispatchErrorMessage(this)
        % internal use - check and dispatch error messages        
            errMsg = this.getError();
            if ~isempty(errMsg)
                this.LastErrorMessage = errMsg;
                notify(this,'ErrorOccured',extras.AsyncMex.AsyncProcessorError(errMsg));
            end
            this.clearError();
        end
    end
    methods (Hidden) %ErrorCheckTimer callback
        function ErrorCheckTimerCallback(this)
            if ~isvalid(this) %cancel if object has been deleted
                return;
            end

            if this.wasErrorThrown() %check for pending errors and dispatch
                this.DispatchErrorMessage();
            end

            %% Stop timer if task thread has stopped or there are no more tasks
            if this.RemainingTasks < 1 || ~this.Running
                stop(this.ErrorCheckTimer);
            end
        end
    end

    %% Results Related properties
    properties(Access=protected)
        ResultsCheckTimer = timer('ObjectVisibility','off',...
            'BusyMode','drop',...
            'ExecutionMode','fixedSpacing',...
            'Tag','AsyncProcessor-ResultsCheckTimer');
    end
    properties (SetObservable=true,AbortSet=true)
        ResultsCheckTimerPeriod = 0.75; %period at which to check for new results
    end
    methods %set period
        function set.ResultsCheckTimerPeriod(this,val)
            assert(isscalar(val)&&isnumeric(val)&&val>0,'ResultsCheckTimerPeriod must be numeric scalar with value>0');
            stop(this.ResultsCheckTimer)
            this.ResultsCheckTimer.Period = val;
            this.ResultsCheckTimerPeriod = val;
            this.restartResultsCheckTimer();
        end
    end
    methods %extend functionality of register and deregister queues
        function registerQueue(this,queues)
        % Register queues to listen to results
        %   registerQueue(QD, queuesToAdd)
        %       This will look through the array queuesToAdd and add queues that
        %       are not already in the list.

            %% if there aren't any listeners, clear existing results queue
            if isempty(this.QueueList)
                this.clearResults();
            end

            %% register listeners
            registerQueue@extras.QueueDispatcher(this,queues);

            %% start results timer if needed
            this.restartResultsCheckTimer();
        end
        function deregisterQueue(this,queues)
        % remove queues from the listening to results
        %   deregisterQueue(QD,queuesToRemove)
        %       Look through array queuesToRemove and remove any that are present
        %       in QueueList
            deregisterQueue@extras.QueueDispatcher(this,queues); %deregister queues

            if isempty(this.QueueList) %if no listeners to results, stop timer
                stop(this.ResultsCheckTimer);
            end
        end
    end
    methods (Access=protected)
        function restartResultsCheckTimer(this)
        %internal use - restart results timer
                   
            if isempty(this.QueueList) %if no listeners to results, don't start timer
                try
                    stop(this.ResultsCheckTimer);
                catch
                end
                return;
            end

            if strcmpi(this.ResultsCheckTimer.Running,'on') %timer is already running
                return;
            end
            try
                start(this.ResultsCheckTimer);
            catch ME
                disp(ME.getReport);
                warning('could not restard ResultsCheckTimer');
            end
        end
    end
    methods (Hidden) %Timer callback
        function ResultsCheckTimerCallback(this)
            if ~isvalid(this) %cancel if object has been deleted
                return;
            end

            %% check for new results
            nRes = this.AvailableResults;

            %% grab results and dispatch
            %only allow grabbing for one timer period
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
                if toc(t) > 0.9*this.ResultsCheckTimerPeriod
                    break;
                end
            end

            %% Stop timer if task thread has stopped or there are no more tasks
            if this.RemainingTasks < 1 || ~this.Running
                stop(this.ResultsCheckTimer);
            end
        end
    end

    %% Create/Delete
    methods
        function this = AsyncProcessor(MEX_NAME,varargin)
        % Create AsyncProcessor Object
        %   MEX_NAME should be a function handle to the mex program containing
        %   a valid AsyncProcessor object
        
            this@extras.SessionManager.Session(MEX_NAME,varargin);

            %% setup error timer
            try
                delete(this.ErrorCheckTimer);
            catch
            end
            this.ErrorCheckTimer = timer('ObjectVisibility','off',...
                                     'BusyMode','drop',...
                                     'ExecutionMode','fixedSpacing',...
                                     'Tag','AsyncProcessor-ErrorCheckTimer',...
                                     'Period',this.ErrorCheckTimerPeriod,...
                                     'ErrorFcn',@(~,err) disp(err),...
                                     'TimerFcn',@(~,~) this.ErrorCheckTimerCallback);
                

             %% Setup the Results TImer
             try
                 delete(this.ResultsCheckTimer)
             catch
             end
             this.ResultsCheckTimer = timer('ObjectVisibility','off',...
                                        'BusyMode','drop',...
                                        'ExecutionMode','fixedSpacing',...
                                        'Tag','AsyncProcessor-ResultsCheckTimer',...
                                        'Period',this.ResultsCheckTimerPeriod,...
                                        'ErrorFcn',@(~,err) disp(err),...
                                        'TimerFcn',@(~,~) this.ResultsCheckTimerCallback);
        end

        function delete(this)
            
            if this.RemainingTasks>0
            	this.resume(); %restart processor if it was stopped
            end
            
            hWB = [];
            nRem = this.RemainingTasks;
            last_comp = 0;
            while this.RemainingTasks>0
                RT = this.RemainingTasks;
                comp = nRem-RT;
                if comp<0 %more tasks were added
                    comp = last_comp;
                    nRem = RT;
                end
                
                if isempty(hWB)
                    hWB = waitbar(comp/nRem,sprintf('%s: Processing (%d/%d)\nPress Cancel to skip remaining.',this.Name,comp,nRem),'CreateCancelBtn',@(~,~) this.cancelRemainingTasks);
                elseif ishghandle(hWB)
                    waitbar(comp/nRem,hWB,sprintf('%s: Processing (%d/%d)\nPress Cancel to skip remaining.',this.Name,comp,nRem));
                end
                pause(0.5);
            end
            try
                delete(hWB)
            catch
            end
            
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

    %% Protected Task-related methods
    methods (Access=protected)
        function n = numResultOutputArgs(this)
            n = this.runMethod('numResultOutputArgs');
        end
        function varargout = popResult(this)
            [varargout{1:nargout}] = this.runMethod('popResult');
        end
        function clearResults(this)
            this.runMethod('clearResults');
        end
        function tf = wasErrorThrown(this)
        % T/F is error was thrown
            tf = this.runMethod('wasErrorThrown');
        end
        function err = getError(this)
        % Get error from processor, if no error was thrown err is empty
        % clears the error buffer after calling
            err = this.runMethod('getError');
        end
        function clearError(this)
        % clear error flag and error struct from buffer
            this.runMethod('clearError');
        end
    end

    %% Task related methods
    methods
        function pushTask(this,varargin)
        % Add task to the processing thread queue
        %   any number of arguments are accepted
        %   all arguments will be forwarded to the mex thread processor
        %   object
        %
        % Error and Results timers will be (re)started once tasks have been
        % added to the queue
            this.runMethod('pushTask',varargin{:});
            
            %% restart timers if needed
            this.restartErrorCheckTimer();
            this.restartResultsCheckTimer();
        end

        function pause(this)
        % pause the processing thread, but don't clear task queue
        
            this.runMethod('pause');
            
            %% stop timers
            stop(this.ErrorCheckTimer);
            stop(this.ResultsCheckTimer);
        end

        function resume(this)
        % resume the processing thread
        
            this.runMethod('resume');
            %% restart timers if needed
            this.restartErrorCheckTimer();
            this.restartResultsCheckTimer();
        end

        function cancelRemainingTasks(this)
        % cancel remaining tasks
            this.runMethod('cancelRemainingTasks');
        end
    end


end
