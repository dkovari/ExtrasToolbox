classdef RadialCenter2 < extras.Async.PersistentArgsProcessor %extras.Async.AsyncProcessor %
% Asynchronous Radialcenter image processor

    %% Create
    methods
        function this = RadialCenter2()
            this@extras.Async.PersistentArgsProcessor(@extras.ParticleTracking.RadialCenter2_mex)
            this.Name = 'RadialCenter2'; %Change Name
        end
    end
end
