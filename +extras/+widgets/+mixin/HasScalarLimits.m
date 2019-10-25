classdef HasScalarLimits < extras.widgets.mixin.HasValueLimits
    %has scalar value, with scalar value limits
    
    properties(Dependent)
        Limits %alias to [Min,Max]
    end
    methods
        function l = get.Limits(this)
            l = [this.Min,this.Max];
        end
        function set.Limits(this,val)
            assert(numel(val)==2);
            val = sort(val);
            this.Min = val(1);
            this.Max = val(2);
        end
    end
    properties(Access=private)
        HasScalarLimits_IsConstructed = false;
    end
    methods
        function this = HasScalarLimits()
            if ~isempty(this.Min) || ~isscalar(this.Min)
                this.Min = -Inf;
            end
            if ~isempty(this.Max) || ~isscalar(this.Max)
                this.Max = Inf;
            end
            was_value_Set = this.HasValue_ValueHasBeenSet;
            if isnumeric(this.Value)&&isempty(this.Value)
                %'in HasScalarLimits set value=NaN'
                this.Value = NaN;
            end
            this.HasValue_ValueHasBeenSet = was_value_Set;
            
            this.HasScalarLimits_IsConstructed = true;
        end
    end
    methods (Access=protected)
        function value = validateValue(this,value)
        % override this method do add validation check to set.Value
        % returns the value that will be set to this.Value
            %value = value;
            if this.HasScalarLimits_IsConstructed && this.EnforceLimits
                if ischar(value)
                    %'in hasscalarlimits validateValue->ischar'
                    value = str2num(value);
                end
                assert(isscalar(value)&&isnumeric(value),'Value must be numeric scalar');
                %fprintf('HasScalar.validateValue: %g before\n',value);
                value = validateValue@extras.widgets.mixin.HasValueLimits(this,value);
                %fprintf('HasScalar.validateValue: %g after\n',value);
            end
        end
        
        function [value,enfl] = validateMin(this,value)
            enfl = false;
            if this.HasScalarLimits_IsConstructed
                assert(isscalar(value)&&isnumeric(value),'Min must be numeric scalar');
                [value,enfl] = validateMin@extras.widgets.mixin.HasValueLimits(this,value);
            end
        end
        
        function [value,enfl] = validateMax(this,value)
            enfl = false;
            if this.HasScalarLimits_IsConstructed
                assert(isscalar(value)&&isnumeric(value),'Max must be numeric scalar');
                [value,enfl] = validateMax@extras.widgets.mixin.HasValueLimits(this,value);
            end
        end
    end
end