classdef DiffractionTracker < extras.Async.PersistentArgsProcessor %extras.Async.AsyncProcessor %
% Asynchronous Radialcenter image processor

    %% Create
    methods
        function this = DiffractionTracker()
            this@extras.Async.PersistentArgsProcessor(@extras.ParticleTracking.DiffractionTracker.DiffractionTracker_mex)
            this.Name = 'DiffractionTracker'; %Change Name
            this.ResultsCheckTimerPeriod = 1/20;
            this.ErrorCheckTimerPeriod = 1/20;
        end
    end
end
