classdef RoiTracker3D < extras.ParticleTracking.RoiTracker.RoiTracker
% Asynchronous Radialcenter image processor with 3D tracking via splineroot

    %% Create
    methods
        function this = RoiTracker3D()
            this.change_MEX_FUNCTION(@extras.ParticleTracking.RoiTracker.RoiTracker3D_mex);
            this.Name = 'RoiTracker3D'; %Change Name
        end
    end
end
