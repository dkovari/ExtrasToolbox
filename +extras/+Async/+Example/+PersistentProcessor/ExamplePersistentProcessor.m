classdef ExamplePersistentProcessor < extras.Async.PersistentArgsProcessor
% Example AsyncProcessor Implementation

    %% Create
    methods
        function this = ExamplePersistentProcessor()
            this@extras.Async.PersistentArgsProcessor(@extras.Async.PersistentProcessor.ExamplePersistentProcessorMex)
            this.Name = 'ExamplePersistentProcessor'; %Change Name
        end
    end
end
