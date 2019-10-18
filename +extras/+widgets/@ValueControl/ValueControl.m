classdef ValueControl < extras.RequireGuiLayoutToolbox & ...
        extras.RequireWidgetsToolbox & ...
        extras.widgets.LabelPanel & ...
        extras.widgets.mixin.abstract.HasEnable & ...
        extras.widgets.mixin.HasCallback & ...
        extras.widgets.mixin.HasScalarLimits & ...
        extras.widgets.mixin.HasAllowedValues & ...
        extras.widgets.mixin.HasIncrement & ...
        extras.widgets.mixin.HasTooltip & ...
        extras.widgets.mixin.AssignNV
% UI widget for controlling a value
% Automatically picks best display for numeric and string data
% implements:
%       Min/Max
%       Increment
%       AllowedValues

    %% Public Controllable properties
    properties (SetObservable=true,AbortSet=true)
        HideSlider (1,1) logical = false;
        HidePopupMenu (1,1) logical = false;
        HideIncrementButtons (1,1) logical = false;
        
        ValueType = 'float'; % type of values to accept possible values 'float' 'integer' 'string' 'boolean'
        RememberValueHistory (1,1) logical = false;
        RememberAllValues (1,1) logical = true; %if true, history includes values not set by uiedit
        %SliderOrientation = 'horizontal';
    end
    %% get/set
    methods
        function set.ValueType(this,vtype)
            if ~ischar(vtype)||isstring(vtype)
                vtype = char(vtype);
            end
            this.ValueType = validatestring(vtype,{'string','float','integer','boolean','command'});
            this.updateControls();
        end
        %function set.SliderOrientation(this,sorient)
        %    this.SliderOrientation = validatestring(sorient,{'horizontal','vertical'});
        %end
        
        function set.RememberValueHistory(this,val)
            this.RememberValueHistory = val;
            if ~val
                this.EditHistory = {};
            end
            this.updateControls();
        end
        
        function set.HideSlider(this,value)
            this.HideSlider = value;
            this.updateControls();
        end
        function set.HidePopupMenu(this,value)
            this.HidePopupMenu = value;
            this.updateControls();
        end
        function set.HideIncrementButtons(this,value)
            this.HideIncrementButtons = value;
            this.updateControls();
        end
    end
    
    %% Events
    events
        UserInteraction %event called when user interacts with the controls
    end
    
    %% public visible, internal changable
    properties(SetAccess=protected,AbortSet=true,SetObservable=true)
        EditHistory = {};
    end
    
    %% Graphics elements - internal only
    properties(Access=private)
        ValueControl_IsConstructed = false; %flag if object has been constructed
        
        ControlGrid %outer grid that holds all elements
        FieldButtonHBox %holds increment buttons and edit/popup field
        ButtonVBox = uix.VBox.empty();%holds inrement buttons
        
        FieldControl %handle to main value control
        IncrementButton %handle to increment button
        DecrementButton %handle to decrement button
        Slider %handle to slider
        
        FieldControlStyle ='edit';%char array specifying FieldControl type
    end
    
    %% Constructor
    methods
        function this = ValueControl(varargin)
            % responsible for initial control creation
            this@extras.widgets.LabelPanel(varargin{:})
            
            %% Create Boxes to hold controls
            this.ControlGrid = uix.Grid('Parent',this);
            this.FieldButtonHBox = uix.HBox('Parent',this.ControlGrid);
            
            %% Create main uicontrol element
            this.FieldControl = uicontrol(this.FieldButtonHBox,...
                'Style','edit',...
                'HandleVisibility','callback',...
                'Interruptible','off',...
                'Callback',@(~,~) this.FieldControlCallback());
            
            %% construction flag
            this.ValueControl_IsConstructed = true;
            
            %% set public
            varargin = this.setParentFromArgs(varargin{:});
            this.setPublicProperties(varargin{:});
            
            this.updateControls();
            
        end
        
        function delete(this)
            try
                delete(this.FieldControl)
            catch
            end
            try
                delete(this.Slider)
            catch
            end
            try
                delete(this.IncrementButton)
            catch
            end
            try
                delete(this.DecrementButton)
            catch
            end
            try
                delete(this.ControlGrid)
            catch
            end
        end
    end
    
    
    %% Implemented in seperate files
    methods (Access=protected)
        updateControls(this) %(re)builds controls
        updateUIValue(this) %updates control value display
        FieldControlCallback(this) %executed when field/popup is changed
        sliderMotionCallback(this)
        onTooltipChanged(this)
    end
    
    %% public - must be overriden
    methods
        function varargout = closestAllowedValue(this,value)
            value = closestAllowedValue@extras.widgets.mixin.HasIncrement(this,value);
            [varargout{1:nargout}] = closestAllowedValue@extras.widgets.mixin.HasAllowedValues(this,value);
        end
    end
    
    %% Overridable Callback methods
    methods (Access=protected)
        
        function str = valueToString(this,str)
            if nargin<2
                str = num2str(this.Value);
            else
                str = num2str(str);
            end
        end
        
        function value = valueFromString(this,str)
            switch this.ValueType
                case 'string'
                    value = this.valueToString(str);
                case 'integer'
                    str=char(str);
                    [value,tf] = str2num(str);
                    assert(tf,sprintf('Could not convert: %s to number',str));
                    value = round(value);
                case 'float'
                    str=char(str);
                    [value,tf] = str2num(str);
                    assert(tf,sprintf('Could not convert: %s to number',str));
                case 'boolean'
                    str=char(str);
                    if strcmpi(str,'true')
                        value = true;
                    elseif strcmpi(str,'false')
                        value = false;
                    elseif strcmpi(str,'on')
                        value = true;
                    elseif strcmpi(str,'off')
                        value = false;
                    else
                        [value,tf]=str2num(str);
                        assert(tf,sprintf('Could not convert: %s to number',str));
                        value = logical(value);
                    end
                    
            end
        end
        
        function incrementValue(this)
            if ~isvalid(this)
                return;
            end
            if this.hasIncrement
                Evt = uiw.event.EventData('Interaction','Increment','OldValue',this.Value,'NewValue',[]);
                try
                this.Value = this.Value + this.Increment;
                catch
                end
                Evt.NewValue = this.Value;
                
                %% fire callback
                this.callCallback(Evt);
                notify(this,'UserInteraction',Evt);
            elseif strcmpi(this.ValueType,'integer')
                this.Value = this.Value + 1;
            end
        end
        
        function decrementValue(this)
            if ~isvalid(this)
                return;
            end
            if this.hasIncrement
                Evt = uiw.event.EventData('Interaction','Decrement','OldValue',this.Value,'NewValue',[]);
                try
                this.Value = this.Value - this.Increment;
                catch
                end
                Evt.NewValue = this.Value;
                
                %% fire callback
                this.callCallback(Evt);
                notify(this,'UserInteraction',Evt);
            elseif strcmpi(this.ValueType,'integer')
                this.Value = this.Value - 1;
            end
        end

        function addHistory(this,str)
            if this.RememberValueHistory
                this.EditHistory = union(this.EditHistory,str,'stable');
            end
        end
        
        function value = validateValue(this,value)
            if ~this.ValueControl_IsConstructed
                return;
            end
            
            switch this.ValueType
                case 'string'
                    if isnumeric(value)
                        value = num2str(value);
                    elseif isstring(value)
                        assert(isscalar(value),'String type values can be char array or scalar string');
                        value = char(value);
                    end
                        
                    assert(ischar(value),'Value must be convertable to char array when ValueType==''string''');
                    
                    value = validateValue@extras.widgets.mixin.HasAllowedValues(this,value);
                case 'float'
                    if ~isnumeric(value)
                        [value,tf] = str2num(value);
                        assert(tf,'Float type expected: Could not covert to numeric');
                    end 
                    
                    value = validateValue@extras.widgets.mixin.HasIncrement(this,value);
                    value = validateValue@extras.widgets.mixin.HasAllowedValues(this,value);
                    value = validateValue@extras.widgets.mixin.HasValueLimits(this,value);
                case 'integer'
                    if ~isnumeric(value)
                        [value,tf] = str2num(value);
                        assert(tf,'Integer type expected: Could not covert to numeric');
                    end
                    
                    value = round(value);
                    
                    value = validateValue@extras.widgets.mixin.HasIncrement(this,value);
                    value = validateValue@extras.widgets.mixin.HasAllowedValues(this,value);
                    value = validateValue@extras.widgets.mixin.HasValueLimits(this,value);
                case 'command'
                    value = NaN;
            end
            
        end
        
        function onValueChanged(this)
            if this.RememberValueHistory && this.RememberAllValues
                try
                    this.addHistory(this.Value)
                catch ME
                    disp(ME.getReport)
                end
                this.updateControls();
            end
            this.updateUIValue();
            
        end
        
        function onAllowedValuesChanged(this)
            this.updateControls();
        end
        
        function onLimitsChanged(this)
            this.updateControls();
        end
        
        function onIncrementChanged(this)
            this.updateControls();
        end

        function sliderCallback(this)
            
            if ~isvalid(this)||isempty(this.Slider)||~isvalid(this.Slider)
                return;
            end
            
            Evt = uiw.event.EventData('Interaction','Slider Changed','OldValue',this.Value,'NewValue',[]);
            this.Value = this.Slider.Value;
            Evt.NewValue = this.Value;
            %% fire callback
            this.callCallback(Evt);
            notify(this,'UserInteraction',Evt);
        end
        
    end
end