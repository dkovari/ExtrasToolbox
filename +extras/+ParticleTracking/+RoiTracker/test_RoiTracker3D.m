% Test RoiTracker3D


%% Cleanup
try
    delete(rcp)
catch
end
close all;
clear all;
clear mex;

clc;


%% Generate LUT Data
nD = 25; %number of pixed dimensions
rRange = [0.5,20]; %radial coord for the besselj function

nu_min = 1; %min nu usws by besselj
nu_max = 10; %max nu used by besselj
Zdata = linspace(nu_min,nu_max,100)'; %
Rdata = 0:nD;

bessel_r = @(r) (rRange(1) + (rRange(2)-rRange(1))/nD*r);

ProfileFn = @(z,r)  besselj(z,bessel_r(r)).*(1./(1+exp(r-nD)));


ProfileData = zeros(numel(Zdata),nD+1);
for n=1:numel(Zdata)
    ProfileData(n,:) = ProfileFn(Zdata(n),Rdata);
end

imagesc(ProfileData)



%% Create Processor
rcp = extras.ParticleTracking.RoiTracker.RoiTracker3D();


%% Generate Test Image


Nx = 5;
Ny = 2;
WIDTH = 500;
HEIGHT = 250;


Xc = (1:Nx)*WIDTH/(Nx+1);
Yc = (1:Ny)*HEIGHT/(Ny+1);

[Xc,Yc] = meshgrid(Xc,Yc);

Xc = Xc + 15*(rand(size(Xc))-0.5);
Yc = Yc + 15*(rand(size(Yc))-0.5);
Xc = reshape(Xc,[],1);
Yc = reshape(Yc,[],1);

Zc = (nu_max-nu_min)*rand(size(Xc))+nu_min;

I = zeros(HEIGHT,WIDTH);

[xx,yy] = meshgrid(1:WIDTH,1:HEIGHT);

for n = 1:numel(Xc)
    rr = sqrt( (xx-Xc(n)).^2 + (yy-Yc(n)).^2);
    I = I + ProfileFn(Zc(n),rr);
end

hFig = figure;
imagesc(I);
hAx = gca;
axis image;
colormap gray;
title('Radial Center Test');
hold on;
plot(Xc,Yc,'*y');
hPlt = plot(NaN,NaN,'+r');
%% Create ROI Manager
RM = extras.roi.roiManager3D;
RL = extras.roi.roiListUI(RM);
RP = extras.roi.roiPlotUI(hAx,RM);


%% Setup callbacks


CBQ = extras.CallbackQueue; %create callback queue to listen to results from processor

afterEach(CBQ,@(d) CB(d,hPlt)) %assign callback to the callback queue

lst = addlistener(rcp,'ErrorOccured',@(~,err) disp(err)); %add listener for errors

rcp.registerQueue(CBQ); %register the callback queue

%



%% Add listener to roiValueChanged
addlistener(RM,'roiValueChanged',@(~,~) roiChanged(RM,rcp,I));


%% Add delete listeners
addlistener(hFig,'ObjectBeingDestroyed',@(~,~) delete_fn(rcp));
addlistener(hFig,'ObjectBeingDestroyed',@(~,~) delete(RM));

%% Add ROI
RM.AddROI();


%% barycenter

%rcp.setParameters('xyMethod','barycenter')

%% delete fn
function delete_fn(rcp)
delete(rcp);
clear mex;
end

%% define callback function
function CB(data,hPlt)

if ~iscell(data)
    res=data;
else
    res=data{1};
    figure(200);gcf;
    imagesc(data{2});
end

%x =[data.X]
%y = [data.Y]
if isempty(res)
    hPlt.XData = [];
    hPlt.YData = [];
else
    hPlt.XData = [res.X];
    hPlt.YData = [res.Y];
end
persistent n;
if isempty(n)
    n=1;
else
    n=n+1;
end

end

function roiChanged(hRM,hdt,I)
roiList = toStruct(hRM.roiList);

hdt.setParameters('roiList',roiList);

'before push image'
pause
hdt.pushTask(I);

end
