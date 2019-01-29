classdef HasAllowedValues < extras.widgets.mixin.HasUnconstrainedValue
    
    properties(SetObservable=true,AbortSet=true)
        AllowedValues
        EnforceAllowedValues (1,1) logical = true;
    end
    properties (SetAccess=protected,SetObservable=true,AbortSet=true)
        ValueIndex = [];
    end
    properties (Dependent)
        hasAllowed = false;
    end
    %% set/get
    methods
        function set.AllowedValues(this,av)
            [this.AllowedValues,tf] = this.validateAllowedValues(av);
            if ~tf
                warning('Value is not included in AllowedValues, suppressing constraint until next time Value is changed');
            else
                this.Value = this.UnconstrainedValue;
            end
            this.onAllowedValuesChanged();
        end
        function tf = get.hasAllowed(this)
            tf = ~isempty(this.AllowedValues);
        end
    end
    
    %% Overridable Callback methods
    methods (Access=protected)
        function value = validateValue(this,value)
            if ~isempty(this.AllowedValues) && this.EnforceAllowedValues
                [tf,value,index] = this.checkAllowed(value,this.AllowedValues);
                assert(tf,'Value is not included in AllowedValues');
                this.ValueIndex = index;
            else
                [tf,val2,index] = this.checkAllowed(value,this.AllowedValues);
                if tf
                    value = val2;
                    this.ValueIndex = index;
                else
                    this.ValueIndex = [];
                end
                
            end
        end
        
        function  [tf,value,IB] = checkAllowed(~,value,allowed)
            if isempty(allowed)
                tf = true;
                IB = [];
                return;
            end
            
            if iscell(value)
                value = {value};
            end
               
            [C,~,IB] = extras.redefined.intersect(value,allowed);
            tf = ~isempty(IB);
            if tf
                value = C;
            end
        end
        
        function [av,tf] = validateAllowedValues(this,av)
            [tf] = this.checkAllowed(this.UnconstrainedValue,av);
        end
        
        
        function onAllowedValuesChanged(this)
        end
    end
    
    
    
end