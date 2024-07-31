classdef HasValue < handle
    % Define handle class that has a value-property and an associated
    % onValueChange property
    
    properties(SetObservable=true,AbortSet=true)
        Value %Value (could be numerical, string, etc) of the object
    end
    
    properties(Access=protected,SetObservable,AbortSet)
        HasValue_ValueHasBeenSet = false; %flaq used to set if object's value has been initialized at least once
    end
    %% Get/set
    methods
        function set.Value(this,value)
            %fprintf('set.Value: %g, before set\n',value);
            %'in set.Value of HasValue'
            %value
            %been_set = this.HasValue_ValueHasBeenSet
            
            this.Value = this.internal_setValue(value);
            this.HasValue_ValueHasBeenSet = true;
            %fprintf('set.Value: %g, after set\n',this.Value);
            this.onValueChanged();
        end
    end
    
    %% Overridable Callback methods
    methods (Access=protected)
        function value = internal_setValue(this,value)
        % returns value that will be set to this.Value
        % calls value = this.validateValue(value)
            value = this.validateValue(value);
        end
        function value = validateValue(~,value)
        % override this method do add validation check to set.Value
        % returns the value that will be set to this.Value
            %value = value;
        end
        function onValueChanged(~)
        % Called by set.Value after changing this.Value, but before
        % exiting set.Value() and executing listener callbacks 
        % default impelementation does nothing.
        % derived classes can use this to perform special operations before
        % set.Value exits.
        end
    end
end