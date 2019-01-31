function updateUIValue(this)
if ~this.ValueControl_IsConstructed
    return;
end

%% update field control
%'edit' 'popupmenu' 'popedit' 'checkbox'
switch this.FieldControlStyle
    case 'edit'
        this.FieldControl.String = this.valueToString();
    case 'popupmenu'
        %% set allowed values
        if isnumeric(this.AllowedValues)
            AV = cellstr(string(this.AllowedValues));
        else
            AV = this.AllowedValues;
        end
        AV = cellstr(AV);
        this.FieldControl.String = AV;
        %% value index
        try
        if isempty(this.ValueIndex)
            this.FieldControl.Value = 1;
            if iscell(this.AllowedValues)
                this.Value = this.AllowedValues{1};
            else
                this.Value = this.AllowedValues(1);
            end
        else
            this.FieldControl.Value = this.ValueIndex;
        end
        catch
        end
        
    case 'popedit'
        this.FieldControl.Value = this.Value;
    case 'checkbox'
        this.FieldControl.Value = this.Value;
end

%% Update slider
if ~isempty(this.Slider)&&isvalid(this.Slider)
    this.Slider.Value = this.Value;
end