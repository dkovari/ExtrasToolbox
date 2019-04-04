classdef RoiTracker < extras.Async.ParameterProcessor 
% Asynchronous Radialcenter image processor

    %% Create
    methods
        function this = RoiTracker()
            this@extras.Async.ParameterProcessor(@extras.ParticleTracking.RoiTracker.RoiTracker_mex)
            this.Name = 'RoiTracker'; %Change Name
            this.ResultsCheckTimerPeriod = 1/20;
            this.ErrorCheckTimerPeriod = 1/20;
        end
    end

    %% IncludeImage
    properties(Dependent)
        IncludeImageInResult (1,1) logical;
    end
    methods
        function tf = get.IncludeImageInResult(this)
            tf = this.runMethod('IncludeImageData');
        end
        function set.IncludeImageInResult(this,val)
            this.runMethod('IncludeImageData',val);
        end
    end

end
