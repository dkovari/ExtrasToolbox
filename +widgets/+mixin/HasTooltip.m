classdef HasTooltip < handle
    properties(SetObservable=true,AbortSet=true)
        Tooltip
        TooltipString
    end
    %% get/set
    methods
        function set.Tooltip(this,value)
            if this.Tooltip_internalset
                this.Tooltip = value;
                return;
            end
            this.Tooltip = num2str(value);
            
            this.Tooltip_internalset = true;
            this.TooltipString = this.Tooltip;
            this.Tooltip_internalset = false;
            
            this.onTooltipChanged()
        end
        function set.TooltipString(this,value)
            if this.Tooltip_internalset
                this.TooltipString = value;
                return;
            end
            this.TooltipString = num2str(value);
            
            this.Tooltip_internalset = true;
            this.Tooltip = this.TooltipString;
            this.Tooltip_internalset = false;
            
            this.onTooltipChanged()
        end
    end
    
    properties(Access=private)
        Tooltip_internalset = false;
    end
    
    
    
    %% Override
    methods (Access=protected)
        function onTooltipChanged(~)
        end
    end
end