% Test DiffractionTracker
try
    delete(rcp)
catch
end
close all;
clear all;
clear mex;
clc;
%% Create Processor

'press a key to create roitracker'
pause
rcp = extras.ParticleTracking.RoiTracker.RoiTracker();
'press a key to continue'
pause

%% Generate Test Image


Nx = 5;
Ny = 2;
WIDTH = 500;
HEIGHT = 250;

Rfn = @(r) (0.5+r-r.^3).*sinc(r/5).*(1./(1+exp(r-(WIDTH/(Nx+1)*0.4))));

Xc = (1:Nx)*WIDTH/(Nx+1);
Yc = (1:Ny)*HEIGHT/(Ny+1);

[Xc,Yc] = meshgrid(Xc,Yc);

Xc = Xc + 15*(rand(size(Xc))-0.5);
Yc = Yc + 15*(rand(size(Yc))-0.5);
Xc = reshape(Xc,[],1);
Yc = reshape(Yc,[],1);

I = zeros(HEIGHT,WIDTH);

[xx,yy] = meshgrid(1:WIDTH,1:HEIGHT);

for n = 1:numel(Xc)
    rr = sqrt( (xx-Xc(n)).^2 + (yy-Yc(n)).^2);
    I = I + Rfn(rr);
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
RM = extras.roi.roiManager;
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

rcp.setParameters('xyMethod','barycenter')

%% delete fn
function delete_fn(rcp)
delete(rcp);
clear mex;
end

%% define callback function
function CB(data,hPlt)
v =[data.X]
hPlt.XData = [data.X];
hPlt.YData = [data.Y];
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
hdt.pushTask(I);

end
