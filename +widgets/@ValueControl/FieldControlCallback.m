function FieldControlCallback(this)

%% update from field control
%'edit' 'popupmenu' 'popedit' 'checkbox'
switch this.FieldControlStyle
    case 'edit'
        this.Value = this.valueFromString(this.FieldControl.String);
    case 'popupmenu'
        idx = this.FieldControl.Value;
        if ~iscell(this.AllowedValues)
            this.Value = this.AllowedValues(idx);
        else
            this.Value = this.AllowedValues{idx};
        end
    case 'popedit'
        this.Value = this.valueFromString(this.FieldControl.Value);
    case 'checkbox'
        this.Value = this.FieldControl.Value;
end