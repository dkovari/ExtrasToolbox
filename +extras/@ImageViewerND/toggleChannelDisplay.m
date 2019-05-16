function toggleChannelDisplay(this,ChanIdx)

if ~this.HasChannel
    return;
end

%% set channel index via FrameIndicies
this.FrameIndicies(this.ChannelDimension) = ChanIdx;

