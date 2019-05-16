function rebuildFrameControls(this)
% (re)builds the frame controls

%% Time Slider
if this.nT==1
    try
        delete(this.TimeSlider)
    catch
    end
    hasT = false;
else
    if isempty(this.TimeSlider)||~isvalid(this.TimeSlider)
        this.TimeSlider = uiw.widget.Slider(...
                            'Parent',this.ControlVBox,...
                            'NotifyOnMotion',true,...
                            'Min',1,...
                            'Max',this.nT,...
                            'Value',1,...
                            'Callback',@(h,~) set(this,'TimeIndex',h.Value),...
                            'Label','Time Index:',...
                            'LabelLocation','left',...
                            'LabelWidth',this.ControlLabelWidth);
    end
    set(this.TimeSlider,...
        'Max',this.nT,...
        'Value',this.TimeIndex);
    hasT = true;
end

%% Z Slider
if this.nZ==1
    try
        delete(this.ZSlider)
    catch
    end
    hasZ = false;
else
    if isempty(this.ZSlider)||~isvalid(this.ZSlider)
        this.ZSlider = uiw.widget.Slider(...
                            'Parent',this.ControlVBox,...
                            'NotifyOnMotion',true,...
                            'Min',1,...
                            'Max',this.nZ,...
                            'Value',1,...
                            'Callback',@(h,~) set(this,'ZIndex',h.Value),...
                            'Label','Z Index:',...
                            'LabelLocation','left',...
                            'LabelWidth',this.ControlLabelWidth);
    end
    set(this.ZSlider,...
        'Max',this.nZ,...
        'Value',this.ZIndex);
    hasZ =true;
end

%% Series Slider
if this.nSeries==1
    try
        delete(this.SeriesSlider)
    catch
    end
    hasS = false;
else
    if isempty(this.SeriesSlider)||~isvalid(this.SeriesSlider)
        this.SeriesSlider = uiw.widget.Slider(...
                            'Parent',this.ControlVBox,...
                            'NotifyOnMotion',true,...
                            'Min',1,...
                            'Max',this.nZ,...
                            'Value',1,...
                            'Callback',@(h,~) set(this,'SeriesIndex',h.Value),...
                            'Label','Series Index:',...
                            'LabelLocation','left',...
                            'LabelWidth',this.ControlLabelWidth);
    end
    set(this.SeriesSlider,...
        'Max',this.nSeries,...
        'Value',this.SeriesIndex);
    hasS = true;
end

%% Channel Buttons
if this.nC==1
    try
        delete(this.ChannelButtons)
    catch
    end
    try
        delete(this.ChannelButtonBox)
    catch
    end
    hasC = false;
else
    if isempty(this.ChannelButtonBox)||~isvalid(this.ChannelButtonBox)
        this.ChannelButtonBox = uix.HBox('Parent',this.ControlVBox,'Padding',5);
        uicontrol(this.ChannelButtonBox,'Style','text','String','Channel:');
        this.ChannelButtonBox.Widths(end) = 50;    
    end
    %% create merge button
    if isempty(this.OverlayChannelsButton)||~isvalid(this.OverlayChannelsButton)

        this.OverlayChannelsButton = uicontrol('Parent',this.ChannelButtonBox,...
            'style','radiobutton',...
            'String','All',...
            'TooltipString','Overlay all channels',...
            'Callback',@(~,~) this.displayChannelOverlay);
        
    end
    %% create channel buttons
    try
        delete(this.ChannelButtons);
        this.ChannelButtons = gobjects(0);
    catch
    end
    
    for n=1:this.nC
        ChanName = this.ChannelName{n};
        if isempty(ChanName)
            ChanName = sprintf('Chan. %d',n);
        end
        this.ChannelButtons(n) = uicontrol('Parent',this.ChannelButtonBox,...
            'style','radiobutton',...
            'String',ChanName,...
            'TooltipString',sprintf('Display %s only.',ChanName),...
            'Callback',@(~,~) this.toggleChannelDisplay(n));
            
    end
    
    hasC = true;
    
end

%% reorder controls
newOrder = gobjects(0);
if hasT
    newOrder = [newOrder,this.TimeSlider];
end
if hasZ
    newOrder = [newOrder,this.ZSlider];
end
if hasS
    newOrder = [newOrder,this.SeriesSlider];
end
if hasC
    newOrder = [newOrder,this.ChannelButtonBox];
end

this.ControlVBox.Contents = newOrder;

if this.ControlPanel.Docked
    this.ControlsHeight = 16 + numel(newOrder)*40;
    this.OuterVBox.Heights(end) = this.ControlsHeight;
end
    
    
