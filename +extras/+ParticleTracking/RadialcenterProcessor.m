classdef RadialcenterProcessor < extras.Async.PersistentArgsProcessor %extras.Async.AsyncProcessor %
% Asynchronous Radialcenter image processor

    %% Create
    methods
        function this = RadialcenterProcessor()
            this@extras.Async.PersistentArgsProcessor(@extras.ParticleTracking.radialcenterAsync)
            this.Name = 'RadialcenterProcessor'; %Change Name
        end
    end
end
