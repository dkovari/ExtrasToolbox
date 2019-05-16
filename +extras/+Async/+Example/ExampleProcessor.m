classdef ExampleProcessor < extras.Async.AsyncProcessor
% Example AsyncProcessor Implementation

    %% Create
    methods
        function this = ExampleProcessor()
            this@extras.Async.AsyncProcessor(@extras.Async.Example.ExampleProcessorMex)
            this.Name = 'ExampleProcessor'; %Change Name
        end
    end
end
