classdef HasValueLimits < extras.widgets.mixin.HasUnconstrainedValue
% Class for adding limits to a HasValue-class

    properties (SetObservable=true,AbortSet=true)
        Min = []; %min allowed value
        Max = []; %max allowed value
        EnforceLimits (1,1) logical = true; %logical indicating if limits should be enforced
    end
    
    properties(Dependent)
        hasMin; %logical indicating if Min has been set
        hasMax; %logical indicating if Max has been set
    end
    
    %% get/set
    methods
        function set.EnforceLimits(this,enfl)
            this.EnforceLimits = enfl;
            if enfl
                this.Value = this.validateValue(this.Value);
            else
                this.Value = this.UnconstrainedValue;
            end
        end
        function set.Min(this,newMin)
            [this.Min,enfl] = this.validateMin(newMin);
            if this.EnforceLimits&&enfl
                this.Value = this.UnconstrainedValue;
            end
            this.onLimitsChanged();
        end
        function set.Max(this,newMax)
            [this.Max,enfl] = this.validateMax(newMax);
            if this.EnforceLimits&&enfl
                this.Value = this.UnconstrainedValue;
            end
            this.onLimitsChanged();
        end
        
        function tf = get.hasMin(this)
            tf = ~( isempty(this.Min) || isscalar(this.Min)&&(isnan(this.Min)||isinf(this.Min)));
        end
        function tf = get.hasMax(this)
            tf = ~( isempty(this.Max) || isscalar(this.Max)&&(isnan(this.Max)||isinf(this.Max)));
        end
    end
    
    %% overrideable
    methods (Access=protected)
        function value = validateValue(this,value)
        % override this method do add validation check to set.Value
        % returns the value that will be set to this.Value
            %value = value;
            
            if ~this.EnforceLimits
                return;
            end
            
            assert(isnumeric(value),'Value must be numeric type for HasValueLimits type class');
            
            %% apply min
            if ~isempty(this.Min)
                if ~isscalar(this.Min) %min is not scalar, expect Value to be same dim
                    assert(all(size(value)==size(this.Min)),'Min is non-scalar, value must be same size');
                    value = max(this.Min,value);
                elseif ~isnan(this.Min)&&~isinf(this.Min) % min is scalar
                    value = max(value,this.Min);
                end
            end
            
            %% apply max
            if ~isempty(this.Max)
                if ~isscalar(this.Max) %max is not scalar, expect Value to be same dim
                    assert(all(size(value)==size(this.Max)),'Max is non-scalar, value must be same size');
                    value = min(this.Max,value);
                elseif ~isnan(this.Max)&&~isinf(this.Max) % max is scalar
                    value = min(value,this.Max);
                end
            end
        end
        
        function [newMin,enfl] = validateMin(this,newMin)
            enfl = true;
            assert(isnumeric(newMin),'Min must be numeric');
            if ~isscalar(newMin) && any(size(newMin)~=size(this.Value))
                warning('Setting Min to array with different dim than Value. Limits will not be enforced until next Value change');
                enfl = false;
                return;
            end
            
        end
        
        function [newMax,enfl] = validateMax(this,newMax)
            enfl = true;
            assert(isnumeric(newMax),'Min must be numeric');
            if ~isscalar(newMax) && any(size(newMax)~=size(this.Value))
                warning('Setting Max to array with different dim than Value. Limits will not be enforced until next Value change');
                enfl = false;
                return;
            end
            
        end
        
        function onLimitsChanged(~)
        end
    end
    
end