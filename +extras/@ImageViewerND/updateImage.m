function updateImage(this)

%% single channel display
if numel(this.ChannelIndex)==1
    this.Image.CData = this.getImagePlane(this.FrameIndicies);
    this.Axes.Colormap = this.ChannelColorspline(this.ChannelIndex).colormap();
else
    error('Overlay NOT implemented yet');
end

