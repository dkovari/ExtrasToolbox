%% Generate and image
WIDTH = 2000;
HEIGHT = 1000;

[xx,yy] = meshgrid(1:WIDTH,1:HEIGHT);

[Xc,Yc] = meshgrid(30:200:WIDTH-30,30:200:HEIGHT-30);

Xc2 = Xc + 20*(rand(size(Xc))-0.5);
Yc2 = Yc + 20*(rand(size(Yc))-0.5);

sz = 1;

Img = zeros(size(xx));
for n=1:numel(Xc)
    Img = Img + -exp( -( (xx-(Xc2(n)-sz)).^2 + (yy-(Yc2(n)-sz)).^2)/(2*sz^2)) ...
        + exp( -( (xx-(Xc2(n)+sz)).^2 + (yy-(Yc2(n)+sz)).^2)/(2*sz^2));
end
Img = Img+0.1*(rand(size(Img))-0.5);

Img = im2uint8(mat2gray(Img));

%% Find Windows
WIND = ParticleTracker.ImageTrackers.FindParticleWindows(Img,50,10,2,0);

%% display
figure(1);clf;
imagesc(Img);
axis image;

hold on;
for n=1:size(WIND,1)
    rectangle('Position',WIND(n,:));
end