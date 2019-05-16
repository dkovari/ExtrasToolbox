function showColormapUI(this)

if isempty(this.ColormapUI)||~isvalid(this.ColormapUI)
    cmap = this.ChannelColorspline(this.ChannelIndex(1));
    this.ColormapUI = extras.colormapUI('Image',this.Image,'cmap',cmap);
else %bring figure to front
    figure(ancestor(this.ColormapUI.Parent,'figure'));
end