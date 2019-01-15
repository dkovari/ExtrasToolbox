function WIND = FindParticleWindows(Image,WindowSz,ParticleSz,NoiseSz,WindowOverlap)
% Find particles in an image and return windows that contain the particle
%   Image: input image
%   WindowSz (default=75): Width & Height of resulting windows
%   ParticleSz (default 10): approximate particle size in pixels
%   NoiseSz (default 2): approximate noize size in pixels
%   WindowOverlap (default 0): allowed window overlap
%
% Particle location is determined by first value-shifting the image so that
% the average intensity equals zeros. Next the absolute value is taken.
% Finally, local maxima of abs(Image) are detected.
% Maxima closer than WindowSz/2-WindowOverlap are ignored.

%% Parse Inputs
if nargin<5
    WindowOverlap = 0;
end
if nargin<4
    NoiseSz = 2;
end
if nargin<3
    ParticleSz = 10;
end

if nargin<2
    WindowSz = 75;
end

%% INIT
import ParticleTracker.ImageTrackers.*

R_MAX = WindowSz/2-WindowOverlap; %min distance between particles, (WINDOW WIDTH)/2

Image = double(Image);

%% FIND PEAKS
Image = abs(Image-mean(Image(:)));

Image = bpass(Image,NoiseSz,ParticleSz);

XY = pkfnd(Image,ParticleSz/2);

%% REMOVE PARTICLES THAT ARE TOO CLOSE
% NPart = size(XY,1);

% CHECKED = false(NPart,1);
% while( any(~CHECKED))
%     n = find(~CHECKED,1);
%     too_close = sqrt( (XY(:,1)-XY(n,1)).^2 + (XY(:,2)-XY(n,2)).^2) < 1.7*R_MAX;
%     too_close(n) = false;
%     if any(too_close)
%         too_close(n) = true;
%         CHECKED(too_close) = [];
%         XY(too_close,:) = [];
%     end
% end

REM_XY = [];
for n=1:size(XY,1)
    d = sqrt( (XY(:,1)-XY(n,1)).^2 + (XY(:,2)-XY(n,2)).^2);
    d(n) = [];
    if any(d<1.7*R_MAX)
        REM_XY = [REM_XY,n];
    end
end

XY(REM_XY,:) = [];


%% CREATE WINDOWS
        % X                  , Y                    ,X2                            , Y2
WIND = [max(1,XY(:,1)-WindowSz/2) , max(1,XY(:,2)-WindowSz/2) , min(size(Image,2),XY(:,1)+WindowSz/2) , min(size(Image,1),XY(:,2)+WindowSz/2)];

%convert to [x,y,W,H]
WIND(:,3) = WIND(:,3)-WIND(:,1)+1;
WIND(:,4) = WIND(:,4)-WIND(:,2)+1;

%roiList = ParticleTracker.roiObject(WIND);
