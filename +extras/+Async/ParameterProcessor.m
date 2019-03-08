classdef ParameterProcessor < extras.Async.AsyncProcessor
% Base class for all ParameterProcess type AsyncProcessors

    %% create
    methods
        function this = ParameterProcessor(MEX_FUNCTION)
            this@extras.Async.AsyncProcessor(MEX_FUNCTION)
            this.Name = 'ParameterProcessor'; %Change Name
        end
    end
    %% persistentArgs methods
    properties (AbortSet=true,SetObservable=true)
        Parameters %user-accessible 
    end
    properties(Access=protected)
        internal_setParameters = false; %lock specifying if using internal call to set.PersistentArgs
    end
    methods
        function set.Parameters(this,ArgCell)
        % change the persistent args
        % external calls are forwarded to setPersistentArgs() or
        % clearPersistentArgs()
        
            if this.internal_setParameters %setting internally, just change values
                this.Parameters = ArgCell;
            else
                assert(iscell(ArgCell),'PersistentArgs must be set to a cell array');
                if isempty(ArgCell)
                    this.clearParameters();
                else
                    this.setParameters(ArgCell{:});
                end
            end
        end
        
        function setParameters(this,varargin)
        % set persistent arguments
        % those arguments will be appended to the arguments passed by pushTask
        
            this.runMethod('setParameters',varargin{:});
            
            %% Get new value of Persistent args
            this.internal_setParameters = true;
            try
                this.Parameters = this.runMethod('getParameters');
            catch ME
                this.internal_setParameters = false;
                rethrow(ME)
            end
            this.internal_setParameters = false;
        end

        function clearParameters(this)
        % clear persistent arguments
        
            this.runMethod('clearParameters');
            this.internal_setParameters = true;
            this.Parameters = {};
            this.internal_setParameters = false;
        end
    end
end
