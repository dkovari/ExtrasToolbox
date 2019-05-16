function changedChannelIndex(this,previous_channel,new_channel)

nChannels = this.StackDimensions(this.ChannelDimension);
%% change toggle states
for n=1:nChannels
    try
        if any(n==new_channel)
            this.ChannelControl.Value = 1;
        else
            this.ChannelControl.Value = 0;
        end
    catch
    end
end

%% colormap ui
if ~isempty(this.ColormapUI)&&isvalid(this.ColormapUI)
    this.ColormapUI.cmap = this.ChannelColorspline(new_channel(1));
    
    ChanName = this.ChannelNames{new_channel(1)};
    if isempty(ChanName)
        ChanName = sprintf('Channel %d',new_channel(1));
    end
    
    this.ColormapUI.Title = ChanName;
end

%% notify event
notify(this,'ChannelChanged',uiw.event.EventData('Interaction','ChannelChanged','PreviousChannel',previous_channel,'NewChannel',new_channel));

