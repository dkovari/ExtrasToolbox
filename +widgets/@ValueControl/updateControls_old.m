function updateControls(this)
    if ~this.ValueControl_IsConstructed
        return;
    end
    switch this.ValueType
        case 'string'
            if ~isempty(this.AllowedValues) %using allowed values
                %% Delete aux controls
                try
                    delete(this.Slider)
                catch
                end
                try
                    delete(this.IncrementUpButton)
                catch
                end
                try
                    delete(this.IncrementDownButton)
                catch
                end                       
                %% check for enforcement
                if this.EnforceAllowedValues %need pop-up without edit
                    %% create/init control
                    if isempty(this.MainControl) || ~isvalid(this.MainControl) || ~isa(this.MainControl,'matlab.ui.control.UIControl')
                        try
                            delete(this.MainControl)
                        catch
                        end
                        this.MainControl = uicontrol(this.FieldButtonHBox,...
                            'Callback',@(~,~) this.FieldControlCallback(),...
                            'HandleVisibility','callback',...
                            'Interruptible','off');
                    end
                    this.MainControl.Style = 'popupmenu';

                    %% set value index
                    this.MainControl.Value = this.ValueIndex;

                    %% allowed values
                    AV = this.AllowedValues;
                    if isstring(this.AllowedValues)
                        AV = cellstr(AV);
                    elseif isnumeric(AV)
                        AV = cellstr(string(AV));
                    end
                    this.MainControl.String = AV;
                    %this.MainControl.TooltipString = 

                else %need pop-up with edit
                    if isempty(this.MainControl) || ~isvalid(this.MainControl) || ~isa(this.MainControl,'uiw.widget.EditablePopup')
                        try
                            delete(this.MainControl)
                        catch
                        end
                        this.MainControl = uiw.widget.EditablePopup('Parent',this.MainBox,...
                            'Callback',this.mainUICallback());
                    end

                    %% Allowed values
                    AV = this.AllowedValues;
                    if isstring(this.AllowedValues)
                        AV = cellstr(AV);
                    elseif isnumeric(AV)
                        AV = cellstr(string(AV));
                    end
                    if this.RememberValueHistory
                        this.MainControl.Items = unique(AV,this.EditHistory);
                    else
                        this.MainControl.Items = AV;
                    end
                    %% set value
                    this.MainControl.Value = this.Value;

                end
            elseif this.RememberValueHistory %need pop-up with edit
                if isempty(this.MainControl) || ~isvalid(this.MainControl) || ~isa(this.MainControl,'uiw.widget.EditablePopup')
                    try
                        delete(this.MainControl)
                    catch
                    end
                    this.MainControl = uiw.widget.EditablePopup('Parent',this.MainBox,...
                        'Callback',this.mainUICallback());
                end

                %% set allowed values
                AV = this.AllowedValues;
                if isstring(this.AllowedValues)
                    AV = cellstr(AV);
                elseif isnumeric(AV)
                    AV = cellstr(string(AV));
                end

                this.MainControl.Items = unique(AV,this.EditHistory);
                %% set value
                this.MainControl.Value = this.Value;

            else %just plain text field
                %% create/init control
                if isempty(this.MainControl) || ~isvalid(this.MainControl) || ~isa(this.MainControl,'matlab.ui.control.UIControl')
                    try
                        delete(this.MainControl)
                    catch
                    end
                    this.MainControl = uicontrol(this.MainBox,...
                        'Callback',@(~,~) this.mainUICallback(),...
                        'HandleVisibility','callback',...
                        'Interruptible','off');
                end
                this.MainControl.Style = 'edit';
                %% set value
                if ischar(this.Value)
                    this.MainControl.String = this.Value;
                else
                    try
                        this.MainControl.String = num2str(this.Value);
                    catch
                        this.MainControl.String = char(string(this.Value));
                    end
                end
            end

            this.CurrentStyle = 'Popup';
        case {'float','integer'}

            %% setup aux control
            if strcmpi(this.PreferedStyle,'Field')
                if this.RememberValueHistory %need popedit
                    %% Delete slider and increment buttons
                    try
                    delete(this.Slider)
                    catch
                    end
                    try
                        delete(this.this.IncrementUpButton)
                    catch
                    end
                    try
                        delete(this.this.IncrementDownButton)
                    catch
                    end                           
                    %% setup pop-edit
                    if isempty(this.MainControl) || ~isvalid(this.MainControl) || ~isa(this.MainControl,'uiw.widget.EditablePopup')
                        try
                            delete(this.MainControl)
                        catch
                        end
                        this.MainControl = uiw.widget.EditablePopup('Parent',this.MainBox,...
                            'Callback',this.mainUICallback());
                    end
                    %% set allowed values
                    AV = this.AllowedValues;
                    if isstring(this.AllowedValues)
                        AV = cellstr(AV);
                    elseif isnumeric(AV)
                        AV = cellstr(string(AV));
                    end

                    this.MainControl.Items = union(AV,this.EditHistory,'stable');
                    %% set value
                    this.MainControl.Value = this.Value;

                    %% set control style
                    this.CurrentStyle = 'Popup';
                else %just edit
                    %% setup main control
                    if isempty(this.MainControl) || ~isvalid(this.MainControl) || ~isa(this.MainControl,'matlab.ui.control.UIControl')
                        try
                            delete(this.MainControl)
                        catch
                        end
                        this.MainControl = uicontrol(this.MainBox,...
                            'Callback',@(~,~) this.mainUICallback(),...
                            'HandleVisibility','callback',...
                            'Interruptible','off');
                    end
                    this.MainControl.Style = 'edit';
                    %% set value
                    this.MainControl.String = num2str(this.Value);
                    %% set control style
                    this.CurrentStyle = 'Field';
                end
            elseif (strcmpi(this.PreferedStyle,'Slider') || strcmpi(this.PreferedStyle,'auto')) ...
                    && this.hasMax && this.hasMin ...
                    && ~this.hasAllowed
                'here'
                %% Delete and increment buttons
                    try
                        delete(this.this.IncrementUpButton)
                    catch
                    end
                    try
                        delete(this.this.IncrementDownButton)
                    catch
                    end

                %% setup edit control
                    if isempty(this.MainControl) || ~isvalid(this.MainControl) || ~isa(this.MainControl,'matlab.ui.control.UIControl')
                        try
                            delete(this.MainControl)
                        catch
                        end
                        this.MainControl = uicontrol(this.MainBox,...
                            'Callback',@(~,~) this.mainUICallback(),...
                            'HandleVisibility','callback',...
                            'Interruptible','off');
                    end
                    this.MainControl.Style = 'edit';
                %% setup slider
                if isempty(this.Slider) || ~isvalid(this.Slider)
                    this.Slider = extras.widgets.jSlider('Parent',this.AuxBox,...
                        'Callback',@(~,~) this.sliderCallback());
                end
                this.Slider.Min = this.Min;
                this.Slider.Max = this.Max;

                %% set value
                this.MainControl.String = num2str(this.Value);
                this.Slider.Value = this.Value;
                %% set control style
                this.CurrentStyle = 'Slider';
            elseif strcmpi(this.PreferedStyle,'Counter') && this.hasIncrement % COUNTER
                %% Delete slider
                    try
                    delete(this.Slider)
                    catch
                    end
                %% setup edit control
                if isempty(this.MainControl) || ~isvalid(this.MainControl) || ~isa(this.MainControl,'matlab.ui.control.UIControl')
                    try
                        delete(this.MainControl)
                    catch
                    end
                    this.MainControl = uicontrol(this.MainBox,...
                        'Callback',@(~,~) this.mainUICallback(),...
                        'HandleVisibility','callback',...
                        'Interruptible','off');
                end
                this.MainControl.Style = 'edit';
                %% up button
                if isempty(this.IncrementUpButton) || ~isvalid(this.IncrementUpButton)
                    this.IncrementUpButton = uicontrol(this.IncrementUpBox,...
                        'style','pushbutton',...
                        'string','+',...
                        'Callback',@(~,~) this.incrementValue(),...
                        'HandleVisibility','callback',...
                        'Interruptible','off');
                    this.IncrementUpBox.Widths = 10;
                end
                %% down button
                if isempty(this.IncrementDownButton) || ~isvalid(this.IncrementDownButton)
                    this.IncrementUpButton = uicontrol(this.IncrementDownBox,...
                        'style','pushbutton',...
                        'string','-',...
                        'Callback',@(~,~) this.decrementValue(),...
                        'HandleVisibility','callback',...
                        'Interruptible','off');
                    this.IncrementDownBox.Widths = 10;
                end

                %% set value
                this.MainControl.String = num2str(this.Value);
                %% set control style
                this.CurrentStyle = 'Counter';
            elseif this.hasAllowed && this.EnforceAllowedValues %popup without edit
                %% create/init control
                if isempty(this.MainControl) || ~isvalid(this.MainControl) || ~isa(this.MainControl,'matlab.ui.control.UIControl')
                    try
                        delete(this.MainControl)
                    catch
                    end
                    this.MainControl = uicontrol(this.MainBox,...
                        'Callback',@(~,~) this.mainUICallback(),...
                        'HandleVisibility','callback',...
                        'Interruptible','off');
                end
                this.MainControl.Style = 'popupmenu';

                %% set value index
                this.MainControl.Value = this.ValueIndex;

                %% allowed values
                AV = this.AllowedValues;
                if isstring(this.AllowedValues)
                    AV = cellstr(AV);
                elseif isnumeric(AV)
                    AV = cellstr(string(AV));
                end
                this.MainControl.String = AV;

                %% set control style
                this.CurrentStyle = 'Popup';

            else % fallback to just edit
                %% setup main control
                if isempty(this.MainControl) || ~isvalid(this.MainControl) || ~isa(this.MainControl,'matlab.ui.control.UIControl')
                    try
                        delete(this.MainControl)
                    catch
                    end
                    this.MainControl = uicontrol(this.MainBox,...
                        'Callback',@(~,~) this.mainUICallback(),...
                        'HandleVisibility','callback',...
                        'Interruptible','off');
                end
                this.MainControl.Style = 'edit';
                %% set value
                this.MainControl.String = num2str(this.Value);

                %% set control style
                    this.CurrentStyle = 'Field';
            end

    end
end