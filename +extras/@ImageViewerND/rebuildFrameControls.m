function rebuildFrameControls(this)
% (re)builds the frame controls



%% delete old controls
try
    delete(this.StackIndexControls);
catch
end
%% delete channel controls box
try
    delete(this.ChannelButtonBox)
    delete(this.ChannelControl);
catch
end

%% construct controls
this.StackIndexControls = repmat(matlab.graphics.GraphicsPlaceholder,1,this.NumStackDimensions); %pre-allocate stackcontrols array
for idx=1:this.NumStackDimensions
    if this.HasChannel && this.ChannelDimension==idx % handle channel differently
        continue;
    elseif this.StackDimensions(idx)<=1  % only one index in this dimension, just leave as placeholder
        continue;
    else % build the control normally
        
        %% control name
        if isempty(this.StackDimensionNames{idx})
            CtrlName= sprintf('StackIndex%d:',idx);
        else
            CtrlName = [num2str(this.StackDimensionNames{idx}),':'];
        end
        
        %% create control
        this.StackIndexControls(idx) = ...
            uiw.widget.Slider(...
                            'Parent',this.ControlVBox,...
                            'NotifyOnMotion',true,...
                            'Min',1,...
                            'Max',this.StackDimensions(idx),...
                            'Value',1,...
                            'Callback',@(h,~) this.changeStackIndex(idx,h.Value),...
                            'Label',CtrlName,...
                            'LabelLocation','left',...
                            'LabelWidth',this.ControlLabelWidth);
    end     
end

%% Channel Buttons

if this.HasChannel && this.StackDimensions(this.ChannelDimension)>1
    
    idx = this.ChannelDimension;
    
    %% control name
    if isempty(this.StackDimensionNames{idx})
        CtrlName= 'Channel:';
    else
        CtrlName = [num2str(this.StackDimensionNames{idx}),':'];
    end
    
    %% create containing box
    this.ChannelButtonBox = uix.HBox('Parent',this.ControlVBox,'Padding',5);
        uicontrol(this.ChannelButtonBox,'Style','text','String',CtrlName);
        this.ChannelButtonBox.Widths(end) = this.ControlLabelWidth;
        
    %% create channel buttons
    for n=1:this.StackDimensions(idx)
        ChanName = this.ChannelNames{n};
        if isempty(ChanName)
            ChanName = sprintf('Chan. %d',n);
        end
        this.ChannelControl(n) = uicontrol('Parent',this.ChannelButtonBox,...
            'style','togglebutton',...
            'String',ChanName,...
            'TooltipString',sprintf('Display %s only.',ChanName),...
            'Callback',@(~,~) this.toggleChannelDisplay(n));
    end
end

%% set controls height
if this.ControlPanel.Docked
    this.ControlsHeight = 16 + numel(this.ControlVBox.Contents)*this.ControlsDefaultRowHeight;
    this.MainOuterVBox.Heights(end) = this.ControlsHeight;
end
    
    
