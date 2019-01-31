function updateControls(this)

%% skip if not valid
if ~isvalid(this)||~this.ValueControl_IsConstructed
    return;
end

%% determine best main control style
% 'edit' 'popupmenu' 'popedit' 'checkbox'

ControlType = 'edit'; %default to field
if strcmpi(this.ValueType,'boolean')
    ControlType = 'checkbox';
end

if ~this.HidePopupMenu
    if strcmpi(this.ValueType,'integer')||strcmpi(this.ValueType,'float')
        if this.hasAllowed && this.EnforceAllowedValues
            ControlType = 'popupmenu';
        elseif this.hasAllowed && ~this.EnforceAllowedValues
            ControlType = 'popedit';
        else
            ControlType = 'edit';
        end
    elseif strcmpi(this.ValueType,'string')
        if this.hasAllowed && this.EnforceAllowedValues
            ControlType = 'popupmenu';
        elseif this.hasAllowed && ~this.EnforceAllowedValues
            ControlType = 'popedit';
        elseif this.RememberValueHistory
            ControlType = 'popedit';
        else
            ControlType = 'edit';
        end
    end
end

%% determine if we should include increment buttons
ShowIncButtons= false;
if ~this.HideIncrementButtons&& ~(this.hasAllowed&&this.EnforceAllowedValues)
    if strcmpi(this.ValueType,'integer')
        ShowIncButtons = true;
    elseif strcmpi(this.ValueType,'float') && this.hasIncrement
        ShowIncButtons = true;
    end
end
%ShowIncButtons
%% determine if we should show slider
ShowSlider = false;
if ~this.HideSlider &&( strcmpi(this.ValueType,'integer') || strcmpi(this.ValueType,'float'))
    if this.hasAllowed && this.EnforceAllowedValues
        ShowSlider  = false;
    elseif this.hasMin && this.hasMax
        ShowSlider = true;
    end
end

%ShowSlider

%% Create/setup main control
switch ControlType
    case 'edit'
        %% create/init control
        if isempty(this.FieldControl) || ~isvalid(this.FieldControl) || ~isa(this.FieldControl,'matlab.ui.control.UIControl')
            try
                delete(this.FieldControl)
            catch
            end
            this.FieldControl = uicontrol(this.FieldButtonHBox,...
                'Callback',@(~,~) this.FieldControlCallback(),...
                'HandleVisibility','callback',...
                'Interruptible','off');
            
            this.addHasEnableObjects(this.FieldControl);
        end
        set(this.FieldControl,...
            'Style','edit',...
            'String',this.valueToString);
        this.FieldControlStyle = 'edit';      
    case 'popupmenu'
        %% create/init control
        if isempty(this.FieldControl) || ~isvalid(this.FieldControl) || ~isa(this.FieldControl,'matlab.ui.control.UIControl')
            try
                delete(this.FieldControl)
            catch
            end
            this.FieldControl = uicontrol(this.FieldButtonHBox,...
                'Callback',@(~,~) this.FieldControlCallback(),...
                'HandleVisibility','callback',...
                'Interruptible','off');
            this.addHasEnableObjects(this.FieldControl);
        end        
        %% set control
        if isnumeric(this.AllowedValues)
            AV = cellstr(string(this.AllowedValues));
        else
            AV = this.AllowedValues;
        end
        AV = cellstr(AV);
        set(this.FieldControl,...
            'Style','popupmenu',...
            'String',AV,...
            'Value',1);
        %% style flag
        this.FieldControlStyle = 'popupmenu';
        %% force update
        this.updateUIValue();
    case 'popedit'
        %% create control
        if isempty(this.FieldControl) || ~isvalid(this.FieldControl) || ~isa(this.FieldControl,'uiw.widget.EditablePopup')
            try
                delete(this.FieldControl)
            catch
            end
            this.FieldControl = uiw.widget.EditablePopup('Parent',this.FieldButtonHBox,...
                'Callback',this.FieldControlCallback());
            this.addHasEnableObjects(this.FieldControl);
        end
        %% Allowed values
        AV = this.AllowedValues;
        if isstring(this.AllowedValues)
            AV = cellstr(AV);
        elseif isnumeric(AV)
            AV = cellstr(string(AV));
        end
        AV = cellstr(AV);
        if this.RememberValueHistory
            AV = union(AV,this.EditHistory,'stable');
        end
        %% set control
        set(this.FieldControl,...
            'Items',AV,...
            'Value',this.valueToString);
        %% style flag
        this.FieldControlStyle = 'popedit';
    case 'checkbox'
        %% create/init control
        if isempty(this.FieldControl) || ~isvalid(this.FieldControl) || ~isa(this.FieldControl,'matlab.ui.control.UIControl')
            try
                delete(this.FieldControl)
            catch
            end
            this.FieldControl = uicontrol(this.FieldButtonHBox,...
                'Callback',@(~,~) this.FieldControlCallback(),...
                'HandleVisibility','callback',...
                'Interruptible','off');
            this.addHasEnableObjects(this.FieldControl);
        end        
        %% set control
        set(this.FieldControl,...
            'Style','checkbox',...
            'String','',...
            'Value',this.Value);
        %% style flag
        this.FieldControlStyle = 'checkbox';
    otherwise
        error('shouldnt be here!!!');
end

%% Setup Increment Buttons
if ShowIncButtons
    %% create container if needed
    if isempty(this.ButtonVBox) || ~isvalid(this.ButtonVBox)
        this.ButtonVBox = uix.VBox('Parent',this.FieldButtonHBox);
        set(this.FieldButtonHBox,...
            'Contents',[this.ButtonVBox,this.FieldControl],...
            'Widths',[15,-1])
    end
    %% create up button if needed
    if isempty(this.IncrementButton) || ~isvalid(this.IncrementButton)
        this.IncrementButton = uicontrol('Parent',this.ButtonVBox,...
            'style','pushbutton',...
            'string','+',...
            'HandleVisibility','callback',...
            'Interruptible','off',...
            'Callback',@(~,~) this.incrementValue);
        this.addHasEnableObjects(this.IncrementButton);
    end
    this.IncrementButton.TooltipString = sprintf('Increment +%g',this.Increment);
    %% create down button if needed
    %% create up button if needed
    if isempty(this.DecrementButton) || ~isvalid(this.DecrementButton)
        this.DecrementButton = uicontrol('Parent',this.ButtonVBox,...
            'style','pushbutton',...
            'string','-',...
            'HandleVisibility','callback',...
            'Interruptible','off',...
            'Callback',@(~,~) this.decrementValue);
        this.addHasEnableObjects(this.DecrementButton);
    end
    this.DecrementButton.TooltipString = sprintf('Decrement -%g',this.Increment);
    %% set button order
    this.ButtonVBox.Contents = [this.IncrementButton,this.DecrementButton];
    
else
    try
        delete(this.ButtonVBox);
    catch
    end
end

%% Setup Slider
if ShowSlider&&~this.HideSlider
    %% create slider if needed
    if isempty(this.Slider)||~isvalid(this.Slider)
        this.Slider = extras.widgets.jSlider('Parent',this.ControlGrid,...
            'Callback',@(~,~) this.sliderCallback(),...
            'OnMotionCallback',@(~,~) this.sliderMotionCallback());
        this.ControlGrid.Contents = [this.FieldButtonHBox,this.Slider];
        this.addHasEnableObjects(this.Slider);
    end
    this.Slider.Min = this.Min;
    this.Slider.Max = this.Max;
    this.Slider.Value = this.Value;
else
    try
        delete(this.Slider);
    catch
    end
end

%% update tooltips
this.onTooltipChanged();
    
    

