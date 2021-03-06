% Test extras.ParticleTracking.radialcenter mex function

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

figure;
imagesc(I);
axis image;
colormap gray;
title('Radial Center Test');

%% Test Using Windows
WIND = [Xc,Yc,zeros(size(Xc)),zeros(size(Yc))] + [-WIDTH/(Nx+1)*0.4,-WIDTH/(Nx+1)*0.4,WIDTH/(Nx+1)*0.8,WIDTH/(Nx+1)*0.8];

hold on;
for n=1:numel(Xc)
    rectangle('Position',WIND(n,:));
end

[X,Y,varXY,d2] = extras.ParticleTracking.radialcenter(I,WIND,'GradientExponent',0,'DistanceExponent',0,'COMmethod','gradmag','RadiusCutoff',Inf);
plot(Xc,Yc,'*y','DisplayName','True Center');
plot(X,Y,'+r','DisplayName','From Windows');


%% test again using defaults args
[X,Y,varXY,d2] = extras.ParticleTracking.radialcenter(I,WIND);
%plot(oXYc(:,1),oXYc(:,2),'sr','DisplayName','Default Args: oXYc');
plot(X,Y,'^r','DisplayName','From Windows, defaults');

%% Test again using XYc estimates
XYc = [Xc,Yc];
XYc = XYc + randn(size(XYc));
plot(XYc(:,1),XYc(:,2),'sc','DisplayName','Guess Point');
[X,Y,varXY,d2] = extras.ParticleTracking.radialcenter(I,'XYc',XYc);
plot(X,Y,'xc','DisplayName','From Guess Point (uses whole image)');


%%
XYc = [Xc,Yc];
XYc = XYc + 10*randn(size(XYc));
[X,Y,varXY,d2] = extras.ParticleTracking.radialcenter(I,[],'GradientExponent',3,'XYc',XYc,'RadiusCutoff',20);
plot(XYc(:,1),XYc(:,2),'sm','DisplayName','Guess2');
plot(X,Y,'xg','DisplayName','From Guess2 with RadiusFilter and GradExp=3');


%% Test other types
typename = 'uint8';
Ityp = cast(double(intmax(typename))*mat2gray(I),typename);
[X,Y,varXY,d2] = extras.ParticleTracking.radialcenter(Ityp,[],'GradientExponent',3,'XYc',XYc,'RadiusCutoff',20);
plot(X,Y,'og','DisplayName','From Guess2 with RF, typecast and GradExp=3');
%plot(XYc(:,1),XYc(:,2),'om');

%% END
legend show
