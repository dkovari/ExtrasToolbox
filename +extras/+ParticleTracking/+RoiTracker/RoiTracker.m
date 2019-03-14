classdef RoiTracker < extras.Async.ParameterProcessor %extras.Async.AsyncProcessor %
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
end
