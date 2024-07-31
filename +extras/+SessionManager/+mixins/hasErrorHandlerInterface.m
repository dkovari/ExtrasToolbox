classdef hasErrorHandlerInterface < extras.SessionManager.Session
%% HASERRORHANDLERINTERFACE mixin class for including interface to error handler
%   A mixin class for including the interface for mex objects which have
%   error handler capabilities (see
%   extras\SessionManager\mexErrorHandlerInterface.hpp)
%
%   NOTE: Mixin classes are typically used in a diamon inheritance
%   structure, therefore this consturctor also accepts no arguments.
%   If constructed without arguments, the MEX_FUNCTION will not be set and
%   the mex object will not be created.

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
            stop(this.ErrorCheckTimer);
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
            
            if this.runMethod('wasErrorThrown')
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
            errMsg = this.runMethod('getError');
            if ~isempty(errMsg)
                this.LastErrorMessage = errMsg;
                notify(this,'ErrorOccured',extras.SessionManager.mixins.SessionError(errMsg,func2str(this.MEX_function)));
            end
            this.runMethod('clearError');
        end
    end
    methods (Hidden) %ErrorCheckTimer callback
        function ErrorCheckTimerCallback(this)
            if ~isvalid(this) %cancel if object has been deleted
                return;
            end
            
            if ~this.SESSION_OBJECT_FULLY_CONSTRUCTED %object has not been created yet, do nothing
                return;
            end

            if this.runMethod('wasErrorThrown') %check for pending errors and dispatch
                this.DispatchErrorMessage();
            end

            %% Stop timer if task thread has stopped or there are no more tasks
            if this.RemainingTasks < 1 || ~this.Running
                stop(this.ErrorCheckTimer);
            end
        end
    end

    %% Constructor
    methods
        function this = hasErrorHandlerInterface(MEX_NAME,varargin)
        % hasErrorHandlerInterface Construct instance of this class, and creates
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
            
            
            %% setup error timer
            try
                delete(this.ErrorCheckTimer);
            catch
            end
            this.ErrorCheckTimer = timer('ObjectVisibility','off',...
                                     'BusyMode','drop',...
                                     'ExecutionMode','fixedSpacing',...
                                     'Name','ErrorHandlerInterface-ErrorCheckTimer',...
                                     'Tag','ErrorHandlerInterface-ErrorCheckTimer',...
                                     'Period',this.ErrorCheckTimerPeriod,...
                                     'ErrorFcn',@(~,err) disp(err),...
                                     'TimerFcn',@(~,~) this.ErrorCheckTimerCallback);
            
        end
        
        function delete(this)
            try
            stop(this.ErrorCheckTimer)
            catch
            end
            delete(this.ErrorCheckTimer);
        end

    end
end

