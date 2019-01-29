classdef (Abstract) LabeledControl < extras.widgets.LabeledPanel & extras.widgets.mixin.abstract.HasEnable
    
    properties (Access=private)
        LabeledControl_IsConstructed
    end
    
    methods
        function this = LabeledControl(varargin)
            this@extras.widgets.LabeledPanel(varargin{:})
            
            %% create control
            this.createControl();
            this.LabeledControl_IsConstructed = true;
            
            %% set public properties
            this.setPublicProperties(varargin{:});
        end
    end
    
    methods (Abstract,Access=protected)
        createControl(this)
    end
    
    
end