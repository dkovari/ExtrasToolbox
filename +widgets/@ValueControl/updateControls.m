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
elseif strcmpi(this.ValueType,'integer')||strcmpi(this.ValueType,'float')
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

%% determine if we should include increment buttons
ShowIncButtons= false;

if strcmpi(this.ValueType,'integer')
    ShowIncButtons = true;
elseif strcmpi(this.ValueType,'float') && this.hasIncrement
    ShowIncButtons = true;
end

%% determine if we should show slider
ShowSlider = false;
if strcmpi(this.ValueType,'integer') || strcmpi(this.ValueType,'float')
    if this.hasAllowed && this.EnforceAllowedValues
        ShowSlider  = false;
    elseif this.hasMin && this.hasMax
        ShowSlider = true;
    end
end

        



