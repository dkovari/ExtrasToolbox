% Test imradialavg

%% Setup
WIDTH = 150;
HEIGHT = 100;

Rfn = @(r) (0.5+r-r.^3).*sinc(r/5).*(1./(1+exp(r-(WIDTH*0.4))));


[xx,yy] = meshgrid(1:WIDTH,1:HEIGHT);

Xc = WIDTH/2 + 10*randn(1);
Yc = HEIGHT/2+ 10*randn(1);

rr = sqrt( (xx-Xc).^2 + (yy-Yc).^2);

I = Rfn(rr);

figure(1);clf;
subplot(3,1,1:2)
imagesc(I);
colormap gray;
axis image;
hold on; plot(Xc,Yc,'+r');
colorbar;

subplot(3,1,3);
x=linspace(0,WIDTH/2);
plot(x,Rfn(x),'-');
xlabel('Radius [px]');
ylabel('Intensity');

%% Compute Radial Avg
[Ravg,Loc,Cnt] = extras.ParticleTracking.imradialavg(I,Xc,Yc,WIDTH/2,0,0.3);

hold on;
plot(Loc,Ravg,'.');

%% Test Speed
disp('Running Speed Test');
nRep = 5000;
I = cell(nRep,1);
XXc = WIDTH/2 + 10*randn(nRep,1);
YYc = HEIGHT/2 + 10*randn(nRep,1);
for n=1:nRep
    rr = sqrt( (xx-XXc(n)).^2 + (yy-YYc(n)).^2);
    I{n} = Rfn(rr);
end

tic
for n=1:nRep
    [Ravg,Loc,Cnt] = extras.ParticleTracking.imradialavg(I{n},XXc(n),YYc(n),WIDTH/2,0,1);
end
t = toc;
fprintf('\tAvg time: %f\n',t/nRep);

%% Test Multiple Locations
disp('test multiple locations')
[Ra,Lc,Ct] = extras.ParticleTracking.imradialavg(I{n},[20:20:100],[20:20:100],WIDTH/2,0,1);

figure(2);clf;
for n=1:numel(Ra)
    plot(Lc{n},Ra{n},'-');
    hold on;
end
