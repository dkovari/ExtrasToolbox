classdef ExampleProcessor_PersistArgs < extras.Async.PersistentArgsProcessor
% Example AsyncProcessor Implementation

    %% Create
    methods
        function this = ExampleProcessor_PersistArgs()
            this@extras.Async.PersistentArgsProcessor(@extras.Async.ExampleProcessor_PersistArgsMex)
            this.Name = 'ExampleProcessor_PersistArgs'; %Change Name
        end
    end
end
