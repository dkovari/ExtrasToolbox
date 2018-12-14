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
[Ravg,Loc,Cnt] = imradialavg(I,Xc,Yc,WIDTH/2,25,0.3);

hold on;
plot(Loc,Ravg,'.');

% %% Type Specific
% typename = 'uint8';
% Ityp = cast(double(intmax(typename))*mat2gray(I),typename);
% [Ravg,Loc,Cnt] = imradialavg(Ityp,Xc,Yc,WIDTH/2,0,0.3);
% plot(Loc,Ravg/double(intmax(typename))*range(I(:))+min(I(:)),'x');