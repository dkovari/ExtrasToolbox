classdef PersistentArgsProcessor < extras.Async.AsyncProcessor
% Example AsyncProcessor Implementation

    %% create
    methods
        function this = PersistentArgsProcessor(MEX_FUNCTION)
            this@extras.Async.AsyncProcessor(MEX_FUNCTION)
            this.Name = 'PersistentArgsProcessor'; %Change Name
        end
    end
    %% persistentArgs methods
    methods
        function setPersistentArgs(this,varargin)
        % set persistent arguments
        % those arguments will be appended to the arguments passed by pushTask

            this.runMethod('setPersistentArgs',varargin{:});

        end

        function clearPersistentArgs(this)
            this.runMethod('clearPersistentArgs');
        end
    end
end
