function sliderMotionCallback(this)
%callback executed when slider moves, but not the final change
if ~isvalid(this)
    return;
end
if ~isvalid(this.Slider)
    return;
end


if this.hasAllowed

    [value,idx] = this.closestAllowedValue(this.Slider.PendingValue);
    
else
    value = this.Slider.PendingValue;
end

%% update field control
%'edit' 'popupmenu' 'popedit' 'checkbox'
switch this.FieldControlStyle
    case 'edit'
        this.FieldControl.String = num2str(value);
    case 'popupmenu'
        if this.hasAllowed
            %% value index
            this.FieldControl.Value = idx;
        end
        
    case 'popedit'
        this.FieldControl.Value = num2str(value);
    case 'checkbox'
        this.FieldControl.Value = value;
end
drawnow();