function FieldControlCallback(this)

%% update from field control
%'edit' 'popupmenu' 'popedit' 'checkbox'
Evt = uiw.event.EventData('OldValue',this.Value,'NewValue',[]);
switch this.FieldControlStyle
    case 'edit'
        this.Value = this.valueFromString(this.FieldControl.String);
        Evt.Interaction = 'Edit Field';
    case 'popupmenu'
        idx = this.FieldControl.Value;
        if ~iscell(this.AllowedValues)
            this.Value = this.AllowedValues(idx);
        else
            this.Value = this.AllowedValues{idx};
        end
        Evt.Interaction = 'Change Popupmenu';
    case 'popedit'
        this.Value = this.valueFromString(this.FieldControl.Value);
        Evt.Interaction = 'Change Popedit';
    case 'checkbox'
        this.Value = this.FieldControl.Value;
        Evt.Interaction = 'Change Checkbox';
    case 'button'
        Evt.Interaction = 'Press Button';
end

Evt.NewValue = this.Value;

%% Fire Callback
this.callCallback(Evt);
notify(this,'UserInteraction',Evt);
