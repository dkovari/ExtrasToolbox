classdef AsyncTestClass < extras.SessionManager.AsyncMexProcessor

    %% Create
    methods
        function this = AsyncTestClass()
            this@extras.AsyncMex.AsyncProcessor(@extras.AsyncMex.AsyncProcessorTest)
        end
    end
end
