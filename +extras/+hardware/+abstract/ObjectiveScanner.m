classdef (Abstract) ObjectiveScanner < extras.hardware.TargetValueDevice
% Base Class for all Objective Scanners
% If you are defining an Objective Scanner interface class, derive from
% this class

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