function set_FrameIndicies(this,value)

% change value
this.Internal_set_FrameIndicies = true;
this.FrameIndicies = value;
this.Internal_set_FrameIndicies = false;

if this.HasChannel
    this.ChannelIndex = this.FrameIndicies(this.ChannelDimension);
else
    this.ChannelIndex = 1;
end

%% update display
this.updateImage