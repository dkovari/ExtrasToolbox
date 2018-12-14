%% Setup
WIDTH = 150;
HEIGHT = 100;

Rfn = @(r) (0.5+r-r.^3).*sinc(r/5).*(1./(1+exp(r-(WIDTH*0.4))));


[xx,yy] = meshgrid(1:WIDTH,1:HEIGHT);

Xc = WIDTH/2 ;%+ 10*randn(1);
Yc = HEIGHT/2;%+ 10*randn(1);

rr = sqrt( (xx-Xc).^2 + (yy-Yc).^2);

I = Rfn(rr);

figure;
imagesc(I);
axis image;
hold on;
plot((20:20:100),(20:20:100),'+r');

%% run
disp('test multiple locations')
[Ra,Lc,Ct] = imradialavg(I,(20:20:100),(20:20:100),WIDTH/2,0,1);

disp('plot');
figure();
for n=1:numel(Ra)
    plot(Lc{n},Ra{n},'-');
    hold on;
end