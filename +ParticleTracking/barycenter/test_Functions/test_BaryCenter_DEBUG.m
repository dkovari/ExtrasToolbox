%test_BaryCenter_DEBUG
clc;
clear all;
close all;
%% Parameters
WIDTH = 200;
HEIGHT = 200;

[xx,yy] = meshgrid(1:WIDTH,1:HEIGHT);

Xc = 97.5;
Yc = 105.75;

sz = 1.5;

BrightMax = 193;
DarkMin = 134;
AvgIntensity = 160;

noise = 7;

%% Contruct image
Img_Base = AvgIntensity*ones(size(xx));
Img_Base = Img_Base + (BrightMax-AvgIntensity)*exp( -( (xx-(Xc-sz)).^2 + (yy-(Yc-sz)).^2)/(2*sz^2)) ...
    + (DarkMin-AvgIntensity)*exp( -( (xx-(Xc+sz)).^2 + (yy-(Yc+sz)).^2)/(2*sz^2));
    
Img = Img_Base+noise*(2*rand(size(Img_Base))-1);
        Img = uint8(Img);
%% Plot Image

hOrig = figure;
imagesc(Img,[0,255]);
axis image;
colormap gray;
title("Original Image");

hold on;
plot(Xc,Yc,'xr','markersize',16);

wind = [25,25,150,150];
hold on;
rectangle('Position',wind);


%% call BaryCenter

%tic
[X,Y,mxX,mxY,mnX,mnY] = ParticleTrackers.BaryCenter_DEBUG(Img,wind,5,0.2);
%toc

%% Plot
figure(hOrig);
plot(X,Y,'+m','markersize',16);
hold on;
plot(mxX,mxY,'sr','markersize',4);
plot(mnX,mnY,'sc','markersize',4);
