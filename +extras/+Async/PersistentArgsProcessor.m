classdef PersistentArgsProcessor < extras.Async.AsyncProcessorWithWriter
% Example AsyncProcessor Implementation

    %% create
    methods
        function this = PersistentArgsProcessor(MEX_FUNCTION)
            this@extras.Async.AsyncProcessorWithWriter(MEX_FUNCTION)
            this.Name = 'PersistentArgsProcessor'; %Change Name
        end
    end
    %% persistentArgs methods
    properties (AbortSet=true,SetObservable=true)
        PersistentArgs %user-accessible 
    end
    properties(Access=protected)
        internal_setPersistentArgs = false; %lock specifying if using internal call to set.PersistentArgs
    end
    methods
        function set.PersistentArgs(this,ArgCell)
        % change the persistent args
        % external calls are forwarded to setPersistentArgs() or
        % clearPersistentArgs()
        
            if this.internal_setPersistentArgs %setting internally, just change values
                this.PersistentArgs = ArgCell;
            else
                assert(iscell(ArgCell),'PersistentArgs must be set to a cell array');
                if isempty(ArgCell)
                    this.clearPersistentArgs();
                else
                    this.setPersistentArgs(ArgCell{:});
                end
            end
        end
        
        function setPersistentArgs(this,varargin)
        % set persistent arguments
        % those arguments will be appended to the arguments passed by pushTask
        
            this.runMethod('setPersistentArgs',varargin{:});
            
            %% Get new value of Persistent args
            this.internal_setPersistentArgs = true;
            try
                this.PersistentArgs = this.runMethod('getPersistentArgs');
            catch ME
                this.internal_setPersistentArgs = false;
                rethrow(ME)
            end
            this.internal_setPersistentArgs = false;
        end

        function clearPersistentArgs(this)
        % clear persistent arguments
        
            this.runMethod('clearPersistentArgs');
            this.internal_setPersistentArgs = true;
            this.PersistentArgs = {};
            this.internal_setPersistentArgs = false;
        end
    end
end
