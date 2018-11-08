classdef ExampleProcessor < extras.AsyncMex.AsyncProcessor
% Example AsyncProcessor Implementation

    %% Create
    methods
        function this = ExampleProcessor()
            this@extras.AsyncMex.AsyncProcessor(@extras.AsyncMex.ExampleProcessorMex)
            this.Name = 'ExampleProcessor'; %Change Name
        end
    end
end
