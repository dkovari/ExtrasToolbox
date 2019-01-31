function onTooltipChanged(this)

if ~isvalid(this)
    return;
end

TTS = char(this.Tooltip);

%% field tooltip
if strcmpi(this.FieldControlStyle,'edit') || strcmpi(this.FieldControlStyle,'popedit')
    TTS_FC = TTS;
    if this.hasIncrement
        TTS_FC = sprintf('%s\nIncrement: +/- %g',TTS_FC,this.Increment);
    end
    if this.hasMin
        TTS_FC = sprintf('%s\nMin: %g',TTS_FC,this.Min);
    end
    if this.hasMax
        TTS_FC = sprintf('%s\nMax: %g',TTS_FC,this.Max);
    end
    try
        set(this.FieldControl,'Tooltip',TTS_FC);
    catch
        try
            set(this.FieldControl,'TooltipString',TTS_FC);
        catch
        end
    end
else %boolean or popup
    try
        set(this.FieldControl,'Tooltip',TTS);
    catch
        try
            set(this.FieldControl,'TooltipString',TTS);
        catch
        end
    end
end

%% Slider Tooltip
if ~isempty(this.Slider)&&isvalid(this.Slider)
    try
        set(this.Slider,'Tooltip',TTS);
    catch
        try
            set(this.Slider,'TooltipString',TTS);
        catch
        end
    end
end