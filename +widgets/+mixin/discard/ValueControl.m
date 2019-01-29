classdef ValueControl < matlab.mixin.SetGet & extras.RequireWidgetsToolbox & ...
        uiw.abstract.WidgetContainer &...
        uiw.mixin.HasEditableText & ...
        uiw.mixin.HasValueEvents
    
    properties (AbortSet=true, SetObservable=true)
        SliderEnable = 'auto'; % auto(==on if valid) or off
        Min (1,1) double = -Inf;
        Max (1,1) double = Inf;
        AllowedValues double = [];
        Increment (1,1) double = NaN;
    end
    %% get/set
    methods
        function set.Min(this,val)
            this.Min = val;
            this.redraw();
        end
        function set.Max(this,val)
            this.Max = val;
            this.redraw();
        end
        function set.SliderEnable(this,val)
            if isnumeric(val)||islogical(val)
                val = logical(val);
                if val
                    val = 'auto';
                else
                    val = 'off';
                end
            end
            assert(ischar(val)&&any(strcmpi(val,{'auto','off'})),'SliderEnable must be ''auto'' or ''off''');
            this.SliderEnable = val;
            this.redraw();
        end
        function set.AllowedValues(this,val)
            val(isnan(val)) = [];
            val = sort(val);
            this.AllowedValues = val;
            this.redraw();
        end
        function set.Increment(this,val)
            if isnan(val)
                this.Increment = 0;
            elseif isinf(val)
                error('Increment cannot be inf');
            else
                this.Increment = val;
            end
            this.redraw();
        end
    end
    
    properties (Access=protected)
        OuterHBox
        Slider
    end
    
    properties (Access=protected)
        hEditBox = [] % Editable text box
        PendingValue
    end
    
    %% Constructor
    methods
        function obj = ValueControl(varargin)
            % Create the edit control
            
            obj.OuterHBox = uix.HBox('Parent',obj.hBasePanel);
            
            obj.Value = 0;
            
            obj.TextHorizontalAlignment = 'center';
            obj.hEditBox = uicontrol(...
                'Parent',obj.OuterHBox,...
                'Style','edit',...
                'HorizontalAlignment','center',...
                'Units','pixels',...
                'Callback', @(h,e)obj.onTextEdited() );
            obj.hTextFields = obj.hEditBox;
            
            % Set properties from P-V pairs
            set(obj,varargin{:});
            
            % No new value is pending
            obj.PendingValue = obj.Value;
            
            % Assign the construction flag
            obj.IsConstructed = true;
            
            % Redraw the widget
            obj.onResized();
            obj.onEnableChanged();
            obj.onStyleChanged();
            obj.redraw();
        end
    end
    
    %% Protected methods
    methods (Access=protected)
        
        function onValueChanged(obj,~)
            % Handle updates to value changes
            % No new value is pending
            obj.PendingValue = obj.Value;
            'onValueChanged'
            obj.redraw();    
        end %function
        
        function redraw(obj)
            % Handle state changes that may need UI redraw
            
            % Ensure the construction is complete
            if obj.IsConstructed
                
                % Update edit text value
                obj.hEditBox.String = obj.interpretValueAsString(obj.Value);
                
                % manage slider
                if strcmpi(obj.SliderEnable,'off') || ~isfinite(obj.Min) || ~isfinite(obj.Max) %no slider
                    try
                        delete(obj.Slider)
                    catch
                    end
                elseif isempty(obj.Slider)||~isvalid(obj.Slider)
                    obj.Slider = extras.widgets.SliderNoEdit(...
                        'Parent',obj.OuterHBox,...
                        'Min',obj.Min,...
                        'Max',obj.Max,...
                        'SnapToTicks',false,...
                        'Orientation','horizontal',...
                        'ShowTicks',true,...
                        'ShowLabels',true,...
                        'Value',obj.Value,...
                        'Callback',@(~,~) obj.onSliderChanged());
                end
                
                %update slider if needed
                if ~isempty(obj.Slider)&&isvalid(obj.Slider)
                    
                    obj.Slider.Min = obj.Min;
                    obj.Slider.Max = obj.Max;
                    obj.Slider.Value = obj.Value;
                    'redraw slider value'
                    disp(obj.Slider.Value)
                    
                    if obj.Increment~=0 && ~isnan(obj.Increment)
                        obj.Slider.MinTickStep = 0;%obj.Increment;
                        obj.Slider.SnapToTicks = false;
                        obj.Slider.ShowTicks = true;
                        obj.Slider.ShowLabels = true;
                    else
                        obj.Slider.MinTickStep = 0;
                        obj.Slider.SnapToTicks = false;
                        obj.Slider.ShowTicks = true;
                        obj.Slider.ShowLabels = true;
                    end
                end
                
            end %if obj.IsConstructed
            
        end %function
        
        function onEnableChanged(obj,~)
            % Handle updates to Enable state
            
            % Ensure the construction is complete
            if obj.IsConstructed
                
                % Call superclass methods
                onEnableChanged@uiw.mixin.HasEditableText(obj);
                
                if ~isempty(obj.Slider) && isvalid(obj.Slider)
                    obj.Slider.Enable = obj.Enable;
                end
                
            end %if obj.IsConstructed
            
        end %function
        
        function onStyleChanged(obj,~)
            % Handle updates to style and value validity changes
            
            % Ensure the construction is complete
            if obj.IsConstructed
                
                % Override edit text colors
%                 obj.TextForegroundColor = obj.ForegroundColor;
%                 obj.TextBackgroundColor = obj.BackgroundColor;
                
                % Call superclass methods
                onStyleChanged@uiw.mixin.HasEditableText(obj);
                
            end %if obj.IsConstructed
            
        end %function
        
        function StatusOk = checkValue(~, value)
            % Return true if the value is valid
            
            StatusOk = isnumeric(value) && isscalar(value);
            
        end %function
        
        function value = interpretStringAsValue(obj,str)
            % Convert entered text to stored data type
            
            value = str2double(str);
            
            if isnan(value)%revert to old value
                return;
            end
            
            if ~isempty(obj.AllowedValues)
                if value==Inf 
                    if any(obj.AllowedValues==Inf)
                        return;
                    else
                        value=max(obj.AllowedValues);
                    end
                elseif value==-Inf 
                    if any(obj.AllowedValues==-Inf)
                        return;
                    else
                        value = min(obj.AllowedValues);
                    end
                else
                    [~,ind] = min(abs(obj.AllowedValues-value));
                    value = obj.AllowedValues(ind);
                end
            end
            
            mn = obj.Min;
            if isnan(mn)
                mn = -Inf;
            end
            mx = obj.Max;
            if isnan(mx)
                mx = Inf;
            end
            value = max(mn,min(value,mx));
                        
        end %function
        
        function str = interpretValueAsString(~,value)
            % Convert stored data to displayed text
            
            str = num2str(value);
            
        end %function
        
        function onTextEdited(obj)
            % Handle interaction with edit box
            
            newValue = obj.interpretStringAsValue(obj.hEditBox.String);
           
            if isnan(newValue)
                obj.hEditBox.String = obj.interpretValueAsString(obj.Value);
                return;
            end
            
            evt = struct('Source', obj, ...
                'Interaction', 'Edit Changed', ...
                'OldValue', obj.Value, ...
                'NewValue', newValue);
                
            'onTextEdited'
            obj.Value = newValue;
            obj.hEditBox.String = obj.interpretValueAsString(obj.Value);
            
            if evt.OldValue ~= evt.NewValue
                obj.callCallback(evt);
            end
        end %function
        
        function onSliderChanged(obj)
            if ~isempty(obj.Slider)&&isvalid(obj.Slider)
                newValue = obj.Slider.Value;
                
                if ~isempty(obj.AllowedValues)
                    if newValue==Inf && any(obj.AllowedValues==Inf)
                        newValue=Inf;
                    elseif newValue==-Inf && any(obj.AllowedValues==-Inf)
                        newValue = -Inf;
                    else
                        [~,ind] = min(abs(obj.AllowedValues-newValue));
                        newValue = obj.AllowedValues(ind);
                    end
                end

                evt = struct('Source', obj, ...
                    'Interaction', 'Slider Changed', ...
                    'OldValue', obj.Value, ...
                    'NewValue', newValue);
                obj.Value = newValue;
                obj.redraw();
                obj.callCallback(evt);
            end
        end
                
    end % Protected methods
 
end % classdef