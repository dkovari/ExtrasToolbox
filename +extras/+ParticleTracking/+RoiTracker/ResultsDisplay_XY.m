classdef ResultsDisplay_XY < handle & extras.GraphicsChild & extras.widgets.mixin.ObjectDependentLifetime
%Display scatter plot of points on axes showing realtime results for XY
%coordinates
    
    %% Props
    properties(SetAccess=private,SetObservable,AbortSet)
        Tracker; %handle to tracker object
    end
    
    %% Create
    methods
        function this = ResultsDisplay_XY(Parent,Tracker)
            % Create Display
            % Inputs:
            %   Parent Axes
            %   Tracker Object
            
            %% Obj Dep
            assert(isa(Tracker,'extras.ParticleTracking.RoiTracker.RoiTracker'),'Tracker must be valid RoiTracker');
            this@extras.widgets.mixin.ObjectDependentLifetime(Tracker);
            %% Setup GraphicsParent type
            this@extras.GraphicsChild(@() gca(figure()) );
            this.AllowableParentType = 'axes'; %set allowed parent type so that current axes are captured if user specifies figure
            this.CheckParentInput(Parent);
            
            %% set internal props
            this.Tracker = Tracker;
        end
    end

end