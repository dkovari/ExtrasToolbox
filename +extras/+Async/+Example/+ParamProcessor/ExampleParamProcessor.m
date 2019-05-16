classdef ExampleParamProcessor < extras.Async.ParameterProcessor
% Example AsyncProcessor Implementation

    %% Create
    methods
        function this = ExampleParamProcessor()
            this@extras.Async.ParameterProcessor(@extras.Async.Example.ParamProcessor.ExampleParamProcessorMex)
            this.Name = 'ExampleParamProcessor'; %Change Name
        end
    end
end
