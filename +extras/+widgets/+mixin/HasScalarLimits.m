classdef HasScalarLimits < extras.widgets.mixin.HasValueLimits
    %has scalar value, with scalar value limits
    
    properties(Dependent)
        Limits %alias to [Min,Max]
    end
    methods
        function l = get.Limits(this)
            l = [this.Min,this.Max];
        end
    end
    properties(Access=private)
        HasScalarLimits_IsConstructed = false;
    end
    methods
        function this = HasScalarLimits()
            this.Min = -Inf;
            this.Max = Inf;
            this.Value = NaN;
            this.HasScalarLimits_IsConstructed = true;
        end
    end
    methods (Access=protected)
        function value = validateValue(this,value)
        % override this method do add validation check to set.Value
        % returns the value that will be set to this.Value
            %value = value;
            if this.HasScalarLimits_IsConstructed
                if ischar(value)
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