% Test extras.ParticleTracking.radialcenter mex function

%extras.ParticleTracking.build_scripts.build_RadialcenterProcessor;

%% Generate Test Image
close all;
clear all;
clear mex;
clc;

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

figure;
imagesc(I);
axis image;
colormap gray;
title('Radial Center Test');

%% Test Using Windows
WIND = [Xc,Xc,Yc,Yc] + [-WIDTH/(Nx+1)*0.4,+WIDTH/(Nx+1)*0.4,-WIDTH/(Nx+1)*0.4,+WIDTH/(Nx+1)*0.4];

%% Create Processor
try
    delete(rcp)
catch
end

rcp = extras.ParticleTracking.RadialcenterProcessor();
CBQ = extras.CallbackQueue; %create callback queue to listen to results from processor

afterEach(CBQ,@CB) %assign callback to the callback queue

lst = addlistener(rcp,'ErrorOccured',@(~,err) disp(err)); %add listener for errors

rcp.registerQueue(CBQ); %register the callback queue
%% push classic
'push without persistent args'
rcp.pushTask(I,WIND);

% %% tmp
% 'delete'
% delete(rcp)
% return

%% pause
disp('press a key to continue');
pause

%% window 3 via persistent args
'setPersistent'
rcp.setPersistentArgs(WIND(1:3,:));
%pause
'push'
rcp.pushTask(I);

'setPersist again and push'
rcp.setPersistentArgs(WIND(4:6,:));
rcp.pushTask(I);
%pause

%% clear persisten args
disp('press a key to continue');
pause
'clear'
rcp.clearPersistentArgs();
rcp.pushTask(I,WIND(7,:));

delete(rcp);
%% define callback function
function CB(data)
persistent n;
if isempty(n)
    n=1;
else
    n=n+1;
end
fprintf('New Result:\n');
hold on;
plot(data{1},data{2},'+','Displayname',sprintf('Task %d',n));
legend show;
disp(data);
end
