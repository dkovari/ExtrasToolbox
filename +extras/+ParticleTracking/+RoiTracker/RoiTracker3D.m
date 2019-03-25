classdef RoiTracker3D < extras.ParticleTracker.RoiTracker %extras.ParticleTracker.RoiTracker %
% Asynchronous Radialcenter image processor with 3D tracking via splineroot

    %% Create
    methods
        function this = RoiTracker3D()
            this.change_MEX_FUNCTION(@extras.ParticleTracker.RoiTracker3D);
            this.Name = 'RoiTracker3D'; %Change Name
        end
    end
    
    %% Splineroot Parameters
    properties
        splineroot_TOL
        splineroot_minStep
        splineroot_maxItr
        splineroot_minR2frac
        splineroot_MaxR2
    end
    methods
        function set.splineroot_TOL(this,val)
            this.setParameters('splineroot_TOL',val);
            this.splineroot_TOL = this.Parameters.splineroot_TOL;
        end
        function set.splineroot_minStep(this,val)
            this.setParameters('splineroot_minStep',val);
            this.splineroot_minStep = this.Parameters.splineroot_minStep;
        end
        function set.splineroot_maxItr(this,val)
            this.setParameters('splineroot_maxItr',val);
            this.splineroot_maxItr = this.Parameters.splineroot_maxItr;
        end
        function set.splineroot_minR2frac(this,val)
            this.setParameters('splineroot_minR2frac',val);
            this.splineroot_minR2frac = this.Parameters.splineroot_minR2frac;
        end
        function set.splineroot_MaxR2(this,val)
            this.setParameters('splineroot_minR2frac',val);
            this.splineroot_MaxR2 = this.Parameters.splineroot_MaxR2;
        end
    end
    
end
