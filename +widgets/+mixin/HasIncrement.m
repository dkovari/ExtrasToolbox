classdef HasIncrement < extras.widgets.mixin.HasUnconstrainedValue
    properties(SetObservable=true,AbortSet=true)
        Increment (1,1) {mustBeNumeric,mustBeFinite,mustBeNonnegative} = 0;
        IncrementStart (1,1) {mustBeNumeric,mustBeFinite} = 0; %starting point of increment step (default=0)
    end
    properties (Dependent)
        hasIncrement
    end
    
    %% get/set
    methods
        function set.Increment(this,inc)
            this.Increment = inc;
            this.Value = this.UnconstrainedValue;
            this.onIncrementChanged();
        end
        function set.IncrementStart(this,inc)
            this.IncrementStart = inc;
            this.Value = this.UnconstrainedValue;
            this.onIncrementChanged();
        end
        
        function tf = get.hasIncrement(this)
            tf = ~(this.Increment==0);
        end
    end
    
    %% Overridable Callback methods
    methods (Access=protected)
        function onIncrementChanged(this)
        end
        
        function value = validateValue(this,value)
        % override this method do add validation check to set.Value
        % returns the value that will be set to this.Value
            %value = value;
            
            if ~isnan(this.Increment)&&this.Increment~=0
                value = round( (value - this.IncrementStart)/this.Increment)*this.Increment + this.IncrementStart;
            end
        end
    end
end