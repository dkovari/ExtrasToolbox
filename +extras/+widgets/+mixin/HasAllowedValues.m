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
    
    %% public methods - should be overriden
    methods
        function varargout = closestAllowedValue(this,value)
        % [value,index] = closestAllowedValue(this,value)
        % returns closest value in AllowedValues
        % and index of that value in the list
            varargout = cell(1,nargout);
        
            if ~this.hasAllowed
                varargout{1} = value;
                varargout{2} = [];
                for n = 3:nargout
                    varargout{n} = [];
                end
                return
            end

            if isnumeric(this.AllowedValues)
                assert(isscalar(value)&&isnumeric(value),'AllowedValues is numeric array, value must be scalar numeric');
                AV = double(this.AllowedValues);
                [~,idx] = min(abs(AV-value));
                value = this.AllowedValues(idx);
            elseif iscell(this.AllowedValues)&&~iscellstr(this.AllowedValues)
                %check all cells for numeric
                for n=1:numel(this.AllowedValues)
                    if ~isnumeric(this.AllowedValues{n})
                        error('closestAllowedValue for cell-type AllowedValues requires all cells contain numeric data');
                    end
                end
                sqdif = zeros(numel(this.AllowedValues),1);
                for n=1:numel(this.AllowedValues)
                    d2 = (this.AllowedValues{n} - value).^2;
                    sqdif(n) = sum(d2(:));
                end
                [~,idx] = min(sqdif);
                value = this.AllowedValues{idx};
            else %string/cellstr
                try
                    value = validatestring(value,this.AllowedValues);
                    [~,~,idx] = extras.redefined.intersect(value,this.AllowedValues);
                catch
                    error('Could not find close match for %s',value);
                end
            end
            
            varargout{1} = value;
            varargout{2} = idx;
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
        
        
        function onAllowedValuesChanged(~)
        end
    end
    
    
    
end