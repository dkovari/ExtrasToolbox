function loadUsingBFReader(this,filepath)

persistent lastDir;
if ~exist('filepath','var')
    [file,pth] = uigetfile(extras.bfmatlab.bfGetFileExtensions,'Select Bio-Formats Compatible Image',fullfile(lastDir,'*.nd2'));
    if file==0
        return;
    end
    lastDir = pth;
    filepath = fullfile(pth,file);
end

%% Change figure title
if this.CreatedParent
    [~,FN,EXT] = fileparts(filepath);
    try
        set(this.Parent,'Name',[FN,EXT]);
    catch
    end
end

%% create bfreader object
this.UsingBF  = true;
this.bfreader = extras.bfmatlab.bfGetReader(filepath);
this.bfmeta = this.bfreader.getMetadataStore();

nT = this.bfreader.getSizeT;
nC = this.bfreader.getSizeC;
nZ = this.bfreader.getSizeZ;
nSeries = this.bfreader.getSeriesCount;

this.NumStackDimensions = 4;
this.StackDimensions = [nC,nT,nZ,nSeries];

this.StackDimensionNames = {'Channel','Time','Z','Series'};
this.HasChannel = true;
this.ChannelDimension = 1;

this.ImageStack = cell(this.StackDimensions);

this.Internal_set_FrameIndicies = true;
this.FrameIndicies = ones(1,this.NumStackDimensions);
this.Internal_set_FrameIndicies = false;

this.ChannelIndex = 1;

%% Channel Specific

% channel names
this.ChannelNames = cell(1,nC);
for n=1:nC
    this.ChannelNames{n} = char(this.bfmeta.getChannelName(0,n-1));
end

% colormaps
this.ChannelColor = cell(1,nC);
this.UseAlphaMap = true(1,nC);
for n=1:nC
    %% alpha mask color
    color = this.bfmeta.getChannelColor(0,n-1);
    this.ChannelColor{n} = [color.getRed,color.getGreen,color.getBlue]/255;
    if all(this.ChannelColor{n}==1)
        this.UseAlphaMap(n) = false;
    end
    
    %% colorspline
    
    %use min/max of first image as default color range
    img = this.getImagePlane([n,1,1,1]);
    cl(1) = double(min(img(:)));
    cl(2) = double(max(img(:)));
    
    this.ChannelColorspline(n) = extras.colormapspline(cl,[0,0,0;this.ChannelColor{n}]);
end

%% rebuild controls
this.rebuildFrameControls();

%% updateImage
this.updateImage();
