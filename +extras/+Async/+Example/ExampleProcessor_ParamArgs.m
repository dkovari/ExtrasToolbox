classdef ExampleProcessor_ParamArgs < extras.Async.ParameterProcessor
% Example AsyncProcessor Implementation

    %% Create
    methods
        function this = ExampleProcessor_ParamArgs()
            this@extras.Async.ParameterProcessor(@extras.Async.Example.EP_PA_mex)
            this.Name = 'ExampleProcessor_ParamArgs'; %Change Name
        end
    end
end
