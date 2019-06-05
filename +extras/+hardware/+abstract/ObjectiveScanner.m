classdef (Abstract) ObjectiveScanner < extras.hardware.TargetValueDevice
% Base Class for all Objective Scanners
% If you are defining an Objective Scanner interface class, derive from
% this class

    %% Properties
    properties(SetAccess=protected,SetObservable,AbortSet)
        Orientation = 'Positive'; %'Positive' indicates positive values move objective up toward sample, "Negative" indicated positive values move objective down 
    end
    methods
        function set.Orientation(this,val)
            this.Orientation = validatestring(val,{'Positive','Negative'});
        end
    end
    methods(Hidden)
        function setOrientation(this,orientation)
            % use this method to change the objective orientation
            % 
            this.Orientation = orientation;
        end
        
    end
        
    %% Create
    methods
        function this = ObjectiveScanner()
            this.DeviceName = 'ObjectiveScanner';
            
            %% create notification forwarder
            addlistener(this,'Value','PostSet',@(~,d) notify(this,'PropertiesChanged',d));
            addlistener(this,'Target','PostSet',@(~,d) notify(this,'PropertiesChanged',d));
            addlistener(this,'Units','PostSet',@(~,d) notify(this,'PropertiesChanged',d));
            addlistener(this,'Limits','PostSet',@(~,d) notify(this,'PropertiesChanged',d));
        end
    end
    
    events
        PropertiesChanged %forwards notification about property changes
    end
end