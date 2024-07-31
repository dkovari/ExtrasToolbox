classdef hasResultsList < extras.SessionManager.Session & extras.QueueDispatcher
    %% constructor/destructor
    methods
        function this = hasResultsList(MEX_NAME,varargin)
        % hasResultsList Construct instance of this class, and creates
        % corresponding mex object using the mexInterface function
        % specified by MEX_NAME
        % Input Arguments:
        %   MEX_NAME - Funciton handle or char array
        %   [...] - Additional arguments are passed to mex object
        %   constructor
        % NOTE:
        %   DO NOT SPECIFY ARGUMENTS if inheriting in a diamond.
        %   If you have a "diamond" inheritance structure (e.g. D<B,C; B<A; C<A), MATLAB creates
        %   two instances of the initial base class (A in example);
        %   however, it does not delete the initially created extra object.
        %   To avoid problems with extra mex objects floating around,
        %   constructing Session without any arguments does not set the
        %   MEX_FUNCTION and DOES NOT create the mex object.
        
            %construct session with arguments, if MEX_NAME not specified,
            %dont pass any arguments
            if nargin<1
                args = {};
            else
                args = [MEX_NAME,varargin];
            end
            this@extras.SessionManager.Session(args{:});
            
            %% Setup the Results Timer
             try
                 delete(this.ResultsCheckTimer)
             catch
             end
             this.ResultsCheckTimer = timer('ObjectVisibility','off',...
                                        'BusyMode','drop',...
                                        'ExecutionMode','fixedSpacing',...
                                        'Name','AsyncProcessor-ResultsCheckTimer',...
                                        'Tag','AsyncProcessor-ResultsCheckTimer',...
                                        'Period',this.ResultsCheckTimerPeriod,...
                                        'ErrorFcn',@(~,err) disp(err),...
                                        'TimerFcn',@(~,~) this.ResultsCheckTimerCallback);
        end
        function delete(this)
            try
            stop(this.ResultsCheckTimer)
            catch
            end
            delete(this.ResultsCheckTimer);
        end
    end
    
    %% Results Related properties
    
    properties (Dependent)
        AvailableResults %number of available results
    end
    methods
        function val = get.AvailableResults(this)
            val = this.runMethod('availableResults');
        end
    end
    
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
    end
    
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
            if ~isvalid(this)
                return
            end
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
    
    methods(Access=protected)
        function sendResult(this,data)
            %overloadable function which is called in
            %ResultsCheckTimerCallback and is responsible for sending data
            %to the dispatch queue list
            %
            % You can modify this method to intercept the data before it is
            % dispatched to the queue list.
            %
            % In most cases the function should end with 
            % this.send(data)
            
            %% YOUR CODE HERE
            
            %% send data to queue list
            this.send(data);
        end
    end
    methods (Access=protected) %Timer callback
        function ResultsCheckTimerCallback(this)
        % called by results check timer, responsible for checking for
        % results and dispatching data using this.sendResult()
            
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
                    %this.send(out{1});
                    this.sendResult(out{1});
                else
                    this.sendResult(out);
                    %this.send(out);
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
    
end