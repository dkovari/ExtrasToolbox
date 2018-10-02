classdef AsyncTestClass < extras.SessionManager.AsyncMexProcessor
    
    %% Create
    methods
        function this = AsyncTestClass()
            this@extras.SessionManager.AsyncMexProcessor(@extras.SessionManager.AsyncProcessorTest)
        end
    end
end