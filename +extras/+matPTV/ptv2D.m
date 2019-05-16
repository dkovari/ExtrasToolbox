function trackStruct = ptv2D(ImageStack,varargin)
% 2D Particle Tracking
%

%% Parse Args

p = inputParser;
p.CaseSensitive = false;

addParameter(p,'Prefilter','none',@(x) validstring(x,{'none','abs-mean','mean-shift'}));
addParameter(p,'bpass_lnoise',1,@(x) isscalar(x)&&isnumeric(x)&&x>0&&isfinite(x));
addParameter(p,'bpass_sz',NaN,@(x) isscalar(x)&&isnumeric(x));
addParameter(p,'pkfnd_sz',NaN,@(x) isscalar(x)&&isnumeric(x));
addParameter(p,'pkfnd_th','50%',@valid_pkfnd_th); %number of percentile ('##%') or # std dev ('##s'), empty uses gui to select value
addParameter(p,'UseRidgeFilter',false,@(x) isscalar(x));
addParameter(p,'CentroidMethod','cntrd'); %options: cntrd() radialcenter() barycenter()
addParameter(p,'CentroidWindowSize',NaN); %NaN->auto determine

addParameter(p,'CentroidArgs',{},@iscell);
addParameter(p,'MaxDisplacement',10);
addParameter(p,'ExpandingSearch',true);
addParameter(p,'Memory',10);

addParameter(p,'SkipTracking',false,@(x) isscalar(x));


parse(p,varargin{:});

assert(ischar(p.Results.CentroidMethod)||isa(p.Results.CentroidMethod,'function_handle'),'CentroidMethod must be string specifying algorithm name, or function handle');


TrackingParameters = p.Results;

%% Stack Info
assert(iscell(ImageStack)||isnumeric(ImageStack)&&ndims(ImageStack)<=3,'ImageStack must be cell array of images, or numeric array of images dims: [HxWxnFrames]');

StackIsCell = iscell(ImageStack);

if StackIsCell
    nF = numel(ImageStack);
else
    nF = size(ImageStack,3);
end

%% WindowSz
CntWindSz = p.Results.CentroidWindowSize;
if ~isfinite(CntWindSz)%&&strcmpi(p.Results.CentroidWindowSize,'cntrd')
    CntWindSz = p.Results.bpass_sz+2;
end
TrackingParameters.CentroidWindowSize = CntWindSz;

%% Keep FiltImg?
KeepFiltImg = strcmpi(p.Results.CentroidMethod,'cntrd') || ~isfinite(CntWindSz);
if KeepFiltImg
    FiltImg = cell(nF,1);
end

%% Find PKS

Use_stacktheshold = false;
if isempty(p.Results.pkfnd_th)
    Use_stacktheshold = true;
    KeepFiltImg = true;
end

PKS = cell(nF,1);
for f=1:nF
    if StackIsCell
        img = ImageStack{f};
    else
        img = ImageStack(:,:,f);
    end
    img = double(img);
    
    %% prefilter
    switch validatestring(lower(p.Results.Prefilter),{'none','abs-mean','mean-shift'})
        case 'abs-mean'
            img = abs(img-mean(img(:)));
        case 'mean-shift'
            img = img-mean(img(:));
    end
    
    %% apply bpass (if needed)
    if isfinite(p.Results.bpass_sz)
        img = extras.ImageProcessing.bpass(img,p.Results.bpass_lnoise,p.Results.bpass_sz);
    end
    
    %% find peak threshold
   if ~Use_stacktheshold
       if isnumeric(p.Results.pkfnd_th)
            pkfnd_th = p.Results.pkfnd_th;
        elseif pkfnd_th_isPercentile(p.Results.pkfnd_th)
            pct = sscanf(p.Results.pkfnd_th,'%g');
            pkfnd_th = prctile(img(:),pct);
        elseif pkfnd_th_isStdDev(p.Results.pkfnd_th)
            nsig = sscanf(p.Results.pkfnd_th,'%g');
            [m,s] = normfit(img(:));
            pkfnd_th = m+nsig*s;
        end
   end

    %% pkfnd
    if ~Use_stacktheshold
        if isfinite(p.Results.pkfnd_sz)
            PKS{f} = extras.ImageProcessing.pkfnd(img,pkfnd_th,p.Results.pkfnd_sz);
        else
            PKS{f} = extras.ImageProcessing.pkfnd(img,pkfnd_th);
        end
    end
    
    if KeepFiltImg
        FiltImg{f} = img;
    end
end

if Use_stacktheshold
    [m,s] = normfit(FiltImg{1}(:));
    pkfnd_th = m+3*s;
    pkfnd_th = extras.matPTV.stackfig_threshold(FiltImg,pkfnd_th);
    
    TrackingParameters.pkfnd_th = pkfnd_th;
    
    for f=1:nF
        if isfinite(p.Results.pkfnd_sz)
            PKS{f} = extras.ImageProcessing.pkfnd(FiltImg{f},pkfnd_th,p.Results.pkfnd_sz);
        else
            PKS{f} = extras.ImageProcessing.pkfnd(FiltImg{f},pkfnd_th);
        end
    end
end

%% Determine window size if needed
if ~isfinite(CntWindSz)
    error('Not Yet implemented');
end

%% Find Centroids
for f=1:nF
    
    if isa(p.Results.CentroidMethod,'function_handle') %function
    else %algo. name
        switch validatestring(...
                p.Results.CentroidMethod,...
                {'cntrd','radialcenter','barycenter'})
            case 'cntrd'
                cnt{f} = extras.ImageProcessing.cntrd(double(ImageStack{f}),PKS{f},CntWindSz);
            otherwise
                error('NOT IMPLEMENTED YET');
        end
    end
end

%% Track Particles
if ~p.Results.SkipTracking
    particle_tracks = extras.matPTV.find_tracks(cnt,'MaxDisp',p.Results.MaxDisplacement,'Memory',p.Results.Memory,'ExpandingSearch',p.Results.ExpandingSearch);
end

%% Create output structure
trackStruct.ImageStack = ImageStack;
trackStruct.nFrames = nF;
trackStruct.Centroids = cnt;
trackStruct.TrackingParameters = TrackingParameters;
if ~p.Results.SkipTracking
    trackStruct.nParticles = numel(particle_tracks);
    % particle structure
    for n=1:trackStruct.nParticles
        trackStruct.track(n).X = particle_tracks{n}(:,1);
        trackStruct.track(n).Y = particle_tracks{n}(:,2);
        %% other output parameters
        if isa(p.Results.CentroidMethod,'function_handle') %function
            trackStruct.track(n).CentroidInfo = particle_tracks{n}(:,3:end);
        else %algo. name
            switch validatestring(...
                    p.Results.CentroidMethod,...
                    {'cntrd','radialcenter','barycenter'})
                case 'cntrd'
                    trackStruct.track(n).Brightness = particle_tracks{n}(:,3);
                    trackStruct.track(n).RadGyration = particle_tracks{n}(:,4);
                    trackStruct.track(n).Eccentricity = particle_tracks{n}(:,5);
                    trackStruct.track(n).Angle = particle_tracks{n}(:,6);
                otherwise
                    error('NOT IMPLEMENTED YET');
            end
        end
    end  
end

%% functions
function tf = valid_pkfnd_th(x)

if isempty(x)
    tf = true;
    return;
end

tf = false;
if isnumeric(x) && isscalar(x)
    tf = true;
    return;
end

tf = pkfnd_th_isPercentile(x) || pkfnd_th_isStdDev(x);

function tf = pkfnd_th_isPercentile(x)
tf = false;
if ischar(x) 
    rex = regexp(x,'(\d*\.?\d*)%');
    if ~isempty(rex) &&rex==1
        tf=true;
        return
    end
end

function tf = pkfnd_th_isStdDev(x)
tf = false;
if ischar(x) 
    rex = regexp(x,'(\d*\.?\d*)s');
    if ~isempty(rex) &&rex==1
        tf=true;
        return
    end
end
       
function tf = validstring(str,validStrings)
tf = true;
try
    o = validatestring(str,validStrings);
catch
    tf = false;
end