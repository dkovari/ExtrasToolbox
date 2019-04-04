classdef RoiTracker3D < extras.ParticleTracker.RoiTracker %extras.ParticleTracker.RoiTracker %
% Asynchronous Radialcenter image processor with 3D tracking via splineroot

    %% Create
    methods
        function this = RoiTracker3D()
            this.change_MEX_FUNCTION(@extras.ParticleTracker.RoiTracker3D_mex);
            this.Name = 'RoiTracker3D'; %Change Name
        end
    end
end
