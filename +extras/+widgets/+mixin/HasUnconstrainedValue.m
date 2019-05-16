classdef HasUnconstrainedValue < extras.widgets.mixin.HasValue
    properties(SetAccess=protected,SetObservable=true,AbortSet=true)
        UnconstrainedValue
    end
    %% Overridable Callback methods
    methods (Access=protected)
        function value = internal_setValue(this,value)
        % returns value that will be set to this.Value
        % calls value = this.validateValue(value)
            this.UnconstrainedValue = value;
            value = this.validateValue(value);
        end
    end
    
    %% public methods - should be overriden
    methods
        function varargout = closestAllowedValue(~,value)
            varargout = cell(1,nargout);
            varargout{1} = value;
        end
    end
end