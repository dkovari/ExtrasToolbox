classdef ResultsDisplay_LUT <  handle & extras.GraphicsChild & extras.widgets.mixin.ObjectDependentLifetime
    
    
    %% Create
    methods
        function this = ResultsDisplay_LUT(varargin)
            
            %% Parse Inputs
            iH = extras.inputHandler();
            iH.addOptionalVariable('Parent',[],@(x) isgraphics(x)&&isvalid(x),true);
            iH.addRequiredVariable('Tracker',@(x) isa(x,'extras.ParticleTracking.RoiTracker.RoiTracker'),true);
            iH.addRequiredVariable('LUT',@(x) isa(x,'extras.roi.LUTobject'),true);
            
            iH.parse(varargin{:});
            
            Parent = iH.Results.Parent;
            Tracker = iH.Results.Tracker;
            LUT = iH.Results.LUT;
            
            
            
            %% Link object lifetime
            this@extras.widgets.mixin.ObjectDependentLifetime({Tracker,LUT});
            
            %% Graphic Child
            this@extras.GraphicsChild(
            
        end
    end
end