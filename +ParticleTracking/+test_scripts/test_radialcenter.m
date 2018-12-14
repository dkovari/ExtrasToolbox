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
WIND = [Xc,Xc,Yc,Yc] + [-WIDTH/(Nx+1)*0.4,+WIDTH/(Nx+1)*0.4,-WIDTH/(Nx+1)*0.4,+WIDTH/(Nx+1)*0.4];

hold on;
for n=1:numel(Xc)
    rectangle('Position',[WIND(n,1),WIND(n,3),WIND(n,2)-WIND(n,1),WIND(n,4)-WIND(n,3)]);
end

[X,Y,varXY,d2] = extras.ParticleTracking.radialcenter(I,WIND);

plot(X,Y,'+r','DisplayName','From Windows');

%% Test again using XYc estimates
XYc = [Xc,Yc];
XYc = XYc + randn(size(XYc));
plot(XYc(:,1),XYc(:,2),'sc','DisplayName','Guess Point');
[X,Y,varXY,d2] = extras.ParticleTracking.radialcenter(I,'XYc',XYc);
plot(X,Y,'xc','DisplayName','From Guess Point');


%% 
XYc = [Xc,Yc];
XYc = XYc + 10*randn(size(XYc));
[X,Y,varXY,d2] = extras.ParticleTracking.radialcenter(I,[],3,'XYc',XYc,'RadiusFilter',20);
plot(XYc(:,1),XYc(:,2),'+m','DisplayName','Guess2');
plot(X,Y,'xg','DisplayName','From Guess2 with RadiusFilter');


%% Test other types
typename = 'uint8';
Ityp = cast(double(intmax(typename))*mat2gray(I),typename);
[X,Y,varXY,d2] = extras.ParticleTracking.radialcenter(Ityp,[],3,'XYc',XYc,'RadiusFilter',20);
plot(X,Y,'og','DisplayName','From Guess2 with RF, typecast');
%plot(XYc(:,1),XYc(:,2),'om');

%% END
legend show