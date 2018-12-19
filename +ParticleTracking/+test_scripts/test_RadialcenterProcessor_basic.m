% Test extras.ParticleTracking.radialcenter mex function

%extras.ParticleTracking.build_scripts.build_RadialcenterProcessor;

%% Generate Test Image
close all;
clear all;
clear mex;
clc;

Nx = 1;
Ny = 1;
WIDTH = 250;
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

hold on;
rectangle('Position',[WIND(1),WIND(3),WIND(2)-WIND(1),WIND(4)-WIND(3)],'EdgeColor','r');


pause

%% get RadialcenterProcessorMex
p = extras.ParticleTracking.radialcenterAsync('new');

%% push data
extras.ParticleTracking.radialcenterAsync('pushTask',p,I,WIND);
extras.ParticleTracking.radialcenterAsync('pushTask',p,I,WIND);
pause(1);

remTasks = extras.ParticleTracking.radialcenterAsync('remainingTasks',p)

nRes = extras.ParticleTracking.radialcenterAsync('availableResults',p)

%% delete
'press key to delete'
pause
extras.ParticleTracking.radialcenterAsync('pause',p)
extras.ParticleTracking.radialcenterAsync('cancelRemainingTasks',p)
remTasks = extras.ParticleTracking.radialcenterAsync('remainingTasks',p)
extras.ParticleTracking.radialcenterAsync('delete',p)
