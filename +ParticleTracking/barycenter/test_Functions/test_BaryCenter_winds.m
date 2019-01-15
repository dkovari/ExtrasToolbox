%% Generate some images
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
Img = Img+0.5*(rand(size(Img))-0.5);

Img = im2uint8(mat2gray(Img));


figure(1);clf;
imagesc(Img)
axis image;

Xc = reshape(Xc,[],1);
Yc = reshape(Yc,[],1);

width = 30;
WIND = [Xc-width/2,Yc-width/2,repmat(width,size(Xc)),repmat(width,size(Xc))];

hold on;
for n=1:numel(Xc)
    rectangle('position',WIND(n,:));
end
drawnow;
disp('press enter to proccess...');
pause();
%%

tic
[X,Y] = ParticleTrackers.BaryCenter(Img,WIND,5,.2);
%[X,Y,mxX,mxY,mnX,mnY] = ParticleTrackers.BaryCenter_sort(Img,WIND,10,.2);
dt = toc
f = 1/dt
figure(1);
plot(X,Y,'+r','markersize',16);
%hold on;
%plot(mxX,mxY,'sr','markersize',4);
%plot(mnX,mnY,'sc','markersize',4);