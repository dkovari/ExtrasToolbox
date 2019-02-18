function trackResults = track_fluorescent()
% Track Fluorescent Particles 


%% Load File
FilePath = ''; %Specify file here, or leave blank to choose file

persistent last_dir;

if ~isempty(FilePath)
    if ~exist(p.Results.FilePath,'file')
        error('Specified File: %s does not exits',p.Results.FilePath);
    end
    
    [Dir,File,ext] = fileparts(FilePath);
    File = [File,ext];
    
else
    %select file
    [File,Dir] = uigetfile(extras.bfmatlab.bfGetFileExtensions,'Select Bio-Formats Compatible Image',fullfile(last_dir,'*.nd2'));
    if File==0
        return
    end
    if ~isempty(Dir)
        last_dir = Dir;
    end
    FilePath = fullfile(Dir,File);
end

%% Specify Series and Channel Options
Series = []; %specify series, empty prompts using gui
Channel = []; %specify channel, empty prompts using gui
Z = 1; 

% Get File Info
bfreader = extras.bfmatlab.bfGetReader(FilePath);
bfmeta = bfreader.getMetadataStore();

% check for multiple series of channel ----> NEED TO ADD CODE FOR Z
numSeries = bfreader.getSeriesCount();
numChan = bfreader.getSizeC();


% Use image viewer to select channels
if numSeries==1
    Series=1;
end
if numChan==1
    Channel=1;
end

if isempty(Series) || isempty(Channel)
    %% Use ImageViewer
    im_viewer = extras.ImageViewerND(FilePath);

    if numSeries>1 && numChan>1
        res = extras.inputdlg(...
            {sprintf('Series (1:%d):',numSeries),...
                sprintf('Channel (1:%d)',numChan)},...
            'Image Planes',1,...
            {num2str(Series),...
                num2str(Channel)},'WindowStyle','normal');
        if isempty(res)
            error('canceled');
        end
        Series = floor(str2double(res{1}));
        Channel = floor(str2double(res{2}));
        assert(isfinite(Series)&&Series<=numSeries,'Invalid Series specified');
        assert(isfinite(Channel)&&Channel<=numChan,'Invalid Channel specified');

    elseif numSeries>1
        res = extras.inputdlg(...
            {sprintf('Series (1:%d):',numSeries)},...
            'Image Planes',1,...
            {num2str(Series)},'WindowStyle','normal');
        if isempty(res)
            error('canceled');
        end
        Series = floor(str2double(res{1}));
        assert(isfinite(Series)&&Series<=numSeries,'Invalid Series specified');
    elseif numChan>1
        res = extras.inputdlg(...
            {sprintf('Channel (1:%d)',numChan)},...
            'Image Planes',1,...
            {num2str(Channel)},'WindowStyle','normal');
        if isempty(res)
            error('canceled');
        end
        Channel = floor(str2double(res{1}));
        assert(isfinite(Channel)&&Channel<=numChan,'Invalid Channel specified');
    end

    delete(im_viewer);

end

%% Load Images
bfreader.setSeries(Series-1);
WIDTH = bfreader.getSizeX();
HEIGHT = bfreader.getSizeY();
numT = bfreader.getSizeT();
meta = bfreader.getMetadataStore();


TimeSec = NaN(numT,1);

usingWB = false;
hWB = [];
ImageStack = cell(numT,1);
t = tic;
for f=1:numT
    if usingWB && ~isvalid(hWB)
        error('Image Read canceled by user.');
    end
    idx = bfreader.getIndex(0,Channel-1,f-1)+1;
    img = extras.bfmatlab.bfGetPlane(bfreader,idx);
    img = reshape(img,HEIGHT,WIDTH);
    ImageStack{f} = img;
    
    
    %get timestamp
    dT = meta.getPlaneDeltaT(Series-1,idx-1);
    TimeSec(f) = dT.value(ome.units.UNITS.S).doubleValue();
    
    if ~usingWB && toc(t)>0.85 %taking a while, show waitbar
        hWB = waitbar(f/numT,sprintf('Retrieving ImageStack (%d/%d)',f,numT));
        usingWB = true;
        t = tic;
    elseif usingWB && toc(t)>0.8%update waitbar
        waitbar(f/numT,hWB,sprintf('Retrieving ImageStack (%d/%d)',f,numT));
    end
end
%delete waitbar
try
    delete(hWB)
catch
end

%% Particle Tracking Options
Prefilter = 'none'; %prefilter option (none is best for fluorescene)
UseRidgeFilter = false; %not implemented yet
bpass_lnoise = 1; %noise filter size (1 is best)
bpass_sz = 7; %size of particles in pixels
pkfnd_sz = NaN; %slightly larger than particle size NaN corresponds to bpass_sz+2
pkfnd_th = []; %min threshold for finding peaks, value (relative to bpass results), number of percentile ('##%') or # std dev ('##s'), Empty prompts using gui

CentroidMethod = 'cntrd'; %options: cntrd() radialcenter() barycenter() [NOTE: only cntrd implemented]
CentroidWindowSize = NaN; %NaN->auto determine
CentroidArgs={}; %optional args for centroid detection method, NOT IMPLEMENTEY
MaxDisplacement=10; %maximum displacement of particles between frames
ExpandingSearch=true; %true if search window should expand with each successive frame for particles that have dissapeared
Memory = 3; %number of frames a particle is allowed to go missing for

%% Track Particles
trackResults = extras.matPTV.ptv2D(ImageStack,...
    'Prefilter',Prefilter,...
    'UseRidgeFilter',UseRidgeFilter,...
    'bpass_lnoise',bpass_lnoise,...
    'bpass_sz',bpass_sz,...
    'pkfnd_sz',pkfnd_sz,...
    'pkfnd_th',pkfnd_th,...
    'CentroidMethod',CentroidMethod,...
    'CentroidWindowSize',CentroidWindowSize,...
    'CentroidArgs',CentroidArgs,...
    'MaxDisplacement',MaxDisplacement,...
    'ExpandingSearch',ExpandingSearch,...
    'Memory',Memory);

trackResults.TimeSec = TimeSec;

%% compute drift
DriftTimeScale = 1; %drift moving average duration (in seconds);

[dX,dY] = extras.matPTV.drift([trackResults.track.X],[trackResults.track.Y],...
    'Time',trackResults.TimeSec,...
    'AverageWindow',DriftTimeScale);
trackResults.driftX = dX;
trackResults.driftY = dY;

for n=1:numel(trackResults.track)
    trackResults.track(n).Xcorrected = trackResults.track(n).X - dX;
    trackResults.track(n).Ycorrected = trackResults.track(n).Y - dY;
end

%% view results
%extras.matPTV.trackviewer(trackResults);


%% set output
if nargout<1
    extras.uiputvar(trackResults)
    clear trackResults;
end
