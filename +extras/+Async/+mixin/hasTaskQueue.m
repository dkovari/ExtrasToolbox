classdef hasTaskQueue < extras.SessionManager.Session
%% hasTaskQueue - Mixin class defining mex Session object which has an internal task queue.
% This class provides the interface for checking if there are tasks in an
% mex object task queue. Upon destruction, this class will ask the user if
% they want to clear the queue or wait for processing to complete.


    %% constructor/destructor
    methods
        function this = hasTaskQueue(MEX_NAME,varargin)
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
            
            
        end
        function delete(this)
            %% prompt to clear tasks
            this.uiPromptToClearTasks()
        end
    end
    
    %% Task List Management Properties & Methods
    
    properties (Dependent)
        RemainingTasks %number of remaining tasks
    end
    methods
        function val = get.RemainingTasks(this)
            val = this.runMethod('remainingTasks');
        end
    end
    
    
    methods(Access=protected)
        function uiPromptToClearTasks(this)
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
                    hWB = waitbar(comp/nRem,sprintf('%s: Processing (%d/%d)\nPress Cancel to skip remaining.',this.Name,comp,nRem),'CreateCancelBtn',@(~,~) this.cancelRemainingTasks());
                elseif ishghandle(hWB)
                    waitbar(comp/nRem,hWB,sprintf('%s: Processing (%d/%d)\nPress Cancel to skip remaining.',this.Name,comp,nRem));
                end
                pause(0.5);
            end
            try
                delete(hWB)
            catch
            end
        end
    end
    
    methods
        function pause(this)
        % pause the processing thread, but don't clear task queue
            
            this.runMethod('pause');
            
        end

        function resume(this)
        % resume the processing thread
        
            if ~isvalid(this) %cancel if object has been deleted
                return;
            end
        
            this.runMethod('resume');
        end
        
        function cancelRemainingTasks(this)
        % cancel remaining tasks
            this.runMethod('cancelRemainingTasks');
        end
        
        function cancelNextTasks(this)
        % cancel remaining tasks
            this.runMethod('cancelNextTask');
        end
    end
end