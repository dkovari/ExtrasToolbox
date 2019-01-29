classdef ValueControl < extras.widgets.LabeledPanel & ...
        extras.widgets.mixin.abstract.HasEnable & ...
        extras.widgets.mixin.HasCallback & ...
        extras.widgets.mixin.HasScalarLimits & ...
        extras.widgets.mixin.HasAllowedValues & ...
        extras.widgets.mixin.HasIncrement
    
    properties (SetObservable=true,AbortSet=true)
        PreferedStyle = 'auto'; % prefered style for the uicontrol
        ValueType = 'string'; % type of values to accept
        RememberValueHistory (1,1) logical = false;
        %SliderOrientation = 'horizontal';
    end
    %% get/set
    methods
        function set.PreferedStyle(this,style)
            this.PreferedStyle = validatestring(style,{'auto','Slider','Field','Popup','Counter'});
        end
        function set.ValueType(this,vtype)
            this.ValueType = validatestring(vtype,{'string','float','integer'});
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
    end
    
    %% internal changable
    properties(SetAccess=protected,AbortSet=true,SetObservable=true)
        CurrentStyle = 'Field';
        EditHistory = {};
    end
    
    %% internal only
    properties(Access=private)
        ControlGrid
        ValueControl_IsConstructed = false;
        MainControl
        MainBox
        AuxBox
        Slider
        IncrementVBox
        IncrementUpBox
        IncrementDownBox
        IncrementUpButton
        IncrementDownButton
    end
    
    %% Constructor
    methods
        function this = ValueControl(varargin)
            % responsible for initial control creation
            this@extras.widgets.LabeledPanel(varargin{:})
            
            %% Create HBox to hold controls
            this.ControlGrid = uix.HBox('Parent',this.MainPanel);
            this.MainBox = uix.HBox('Parent',this.ControlGrid);
            this.AuxBox = uix.HBox('Parent',this.ControlGrid);
            this.IncrementVBox = uix.VBox('Parent',this.AuxBox);
            this.IncrementUpBox = uix.HBox('Parent',this.IncrementVBox);
            this.IncrementDownBox = uix.HBox('Parent',this.IncrementVBox);
            
            %% Create main uicontrol element
            this.MainControl = uicontrol(this.MainBox,...
                'Style','edit',...
                'Callback',@(~,~) this.mainUICallback());
            
            %% construction flag
            this.ValueControl_IsConstructed = true;
            
            %% set public
            this.setPublicProperties(varargin{:})
            
            this.updateControls();
            
        end
    end
    
    
    %% Overridable Callback methods
    methods (Access=protected)
        
        updateControls(this) %implemented in file
        
        function incrementValue(this)
            if ~isvalid(this)
                return;
            end
            if this.hasIncrement
                try
                this.Value = this.Value + this.Increment;
                catch
                end
            end
        end
        
        function decrementValue(this)
            if ~isvalid(this)
                return;
            end
            if this.hasIncrement
                try
                this.Value = this.Value - this.Increment;
                catch
                end
            end
        end
        
        function updateUIValue(this)
            if ~this.ValueControl_IsConstructed
                return;
            end
            if isa(this.MainControl,'matlab.ui.control.UIControl')
                %ctrl_val = [];
                switch this.MainControl.Style
                    case 'edit'
                        this.MainControl.String = num2str(this.Value);
                    case 'popupmenu'
                        this.MainControl.Value = this.ValueIndex;
                    otherwise
                        error('invalid MainControl Type');
                end
            elseif isa(this.MainControl,'uiw.widget.EditablePopup')
                this.MainControl.Value = this.Value;
            end
            
            if isvalid(this.Slider)
                try
                    this.Slider.Value = this.Value;
                catch
                end
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
            end
            
        end
        
        function onValueChanged(this)
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
        
        function mainUICallback(this)
            if ~isvalid(this)||isempty(this.MainControl)||~isvalid(this.MainControl)
                return;
            end
            
            if isa(this.MainControl,'matlab.ui.control.UIControl')
                %ctrl_val = [];
                switch this.MainControl.Style
                    case 'edit'
                        ctrl_val = this.MainControl.String;
                    case 'popupmenu'
                        ctrl_val = this.MainControl.String{this.MainControl.Value};
                    otherwise
                        error('invalid MainControl Type');
                end
                if strcmpi(this.ValueType,'string')
                    this.Value = ctrl_val;
                else
                    [val,tf]= str2num(ctrl_val);
                    if tf
                       this.Value = val;
                    else
                        this.updateUIValue();
                    end
                end
                if strcmpi(this.MainControl.Style,'edit')
                    this.addHistory(ctrl_val);
                end
            elseif isa(this.MainControl,'uiw.widget.EditablePopup')
                if strcmpi(this.ValueType,'string')
                    this.Value = this.MainControl.Value;
                else
                    [val,tf]= str2num(this.MainControl.Value);
                    if tf
                       this.Value = val;
                    else
                        this.updateUIValue();
                    end
                end
                if isempty(this.MainControl.SelectedIndex)
                    this.addHistory(this.MainControl.Value);
                end
            end
            
        end
        
        function sliderCallback(this)
            if ~isvalid(this)||isempty(this.Slider)||~isvalid(this.Slider)
                return;
            end
            this.Value = this.Slider.Value;
        end
    end
end