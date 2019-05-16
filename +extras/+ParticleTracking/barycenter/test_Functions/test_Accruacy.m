%function test_Accruacy()

%% Params
WIDTH = 50;
HEIGHT = 50;

[xx,yy] = meshgrid(1:WIDTH,1:HEIGHT);


sz = 1.5;

BrightMax = 193;
DarkMin = 134;
AvgIntensity = 160;

noise = 7;
nRep = 100000;

sigPosition = 1;

foundX = NaN(nRep,1);
foundY = NaN(nRep,1);

foundX_3x3 = NaN(nRep,1);
foundY_3x3 = NaN(nRep,1);

foundX_5x5 = NaN(nRep,1);
foundY_5x5 = NaN(nRep,1);

foundX_p5x5 = NaN(nRep,1);
foundY_p5x5 = NaN(nRep,1);

foundX_classic = NaN(nRep,1);
foundY_classic = NaN(nRep,1);

%% Loop and find centers
hWB = waitbar(0,'proccessing');
for n = 1:nRep
    %% Contruct image
    Xc = sigPosition*randn(1) + WIDTH/2;
    Yc = sigPosition*randn(1) + HEIGHT/2;
    
    Img = AvgIntensity*ones(size(xx));
    Img = Img + (BrightMax-AvgIntensity)*exp( -( (xx-(Xc-sz)).^2 + (yy-(Yc-sz)).^2)/(2*sz^2)) ...
        + (DarkMin-AvgIntensity)*exp( -( (xx-(Xc+sz)).^2 + (yy-(Yc+sz)).^2)/(2*sz^2));
    
    Img = Img+noise*(2*rand(size(Img))-1);
    Img = uint8(Img);
    
    %% perform fit
    
    [foundX(n),foundY(n)] = ParticleTrackers.BaryCenter(Img,[],5,0.25);
    
    [foundX_3x3(n),foundY_3x3(n)] = ParticleTrackers.BaryCenter_m3x3(Img,[],5,0.25);
    
    [foundX_5x5(n),foundY_5x5(n)] = ParticleTrackers.BaryCenter_m5x5(Img,[],5,0.25);
    
    [foundX_p5x5(n),foundY_p5x5(n)] = ParticleTrackers.BaryCenter_p5x5(Img,[],5,0.25);
    
    [foundX_classic(n),foundY_classic(n)] = ParticleTrackers.BaryCenter_classic(Img,[],5,0.25);
    
    if mod(n,100)==0
%         figure(1);clf;
%         imagesc(Img,[0,255])
% 
%         axis image;
%         colormap gray;
% 
%         hold on;
%         plot(foundX(n),foundY(n),'+r');

        waitbar(n/nRep,hWB,sprintf('Processing: %d/%d',n,nRep));
    end

end
delete(hWB);

%% Create Plot mean3x3
hFig = figure('name','no mean','units','inches',...
    'position',[0,0,9,3],...
    'paperunits','inches',...
    'papersize',[9,3],...
    'paperposition',[0,0,9,3]);
movegui(hFig,'center');

%image
subplot(1,3,1);
imagesc(Img,[0,255]);
axis image;
colormap gray;
title('Example Image');
hold on;
plot(foundX(n),foundY(n),'+','LineWidth',1,'markersize',8);

%X histogram
subplot(1,3,2);
histogram(foundX(:),'BinWidth',0.01,'EdgeColor','none','FaceColor','k','normalization','pdf');
ylabel('pdf');
xlabel('Found X Pos [px]');
hold on;
fxx=linspace(min(foundX),max(foundX),100);
hL=plot(fxx,normpdf(fxx,WIDTH/2,sigPosition),'-r','linewidth',1.5,'displayname','Target');
legend(hL,'location','northeast')
title('No Filter');
%theta histogram
subplot(1,3,3);
theta = atan2(foundY-mean(foundY),foundX-mean(foundX));
polarhistogram(theta,360,'displaystyle','stairs');
[~, ~, SymRatio] = CalcSymStats(foundX,foundY);
title(sprintf('Symmetry: %g',SymRatio));

%% Create Plot mean3x3
hFig = figure('name','mean3x3','units','inches',...
    'position',[0,0,9,3],...
    'paperunits','inches',...
    'papersize',[9,3],...
    'paperposition',[0,0,9,3]);
movegui(hFig,'center');

%image
subplot(1,3,1);
imagesc(Img,[0,255]);
axis image;
colormap gray;
title('Example Image');
hold on;
plot(foundX_3x3(n),foundY_3x3(n),'+','LineWidth',1,'markersize',8);

%X histogram
subplot(1,3,2);
histogram(foundX_3x3(:),'BinWidth',0.01,'EdgeColor','none','FaceColor','k','normalization','pdf');
ylabel('pdf');
xlabel('Found X Pos [px]');
hold on;
fxx=linspace(min(foundX_3x3),max(foundX_3x3),100);
hL=plot(fxx,normpdf(fxx,WIDTH/2,sigPosition),'-r','linewidth',1.5,'displayname','Target');
legend(hL,'location','northeast')
title('Mean 3x3 Filter');
%theta histogram
subplot(1,3,3);
theta = atan2(foundY_3x3-mean(foundY_3x3),foundX_3x3-mean(foundX_3x3));
polarhistogram(theta,360,'displaystyle','stairs');
[~, ~, SymRatio] = CalcSymStats(foundX_3x3,foundY_3x3);
title(sprintf('Symmetry: %g',SymRatio));

%% Create Plot mean5x5
hFig_m5x5 = figure('name','mean5x5','units','inches',...
    'position',[0,0,9,3],...
    'paperunits','inches',...
    'papersize',[9,3],...
    'paperposition',[0,0,9,3]);
movegui(hFig_m5x5,'center');

%image
subplot(1,3,1);
imagesc(Img,[0,255]);
axis image;
colormap gray;
title('Example Image');
hold on;
plot(foundX_5x5(n),foundY_5x5(n),'+','LineWidth',1,'markersize',8);

%X histogram
subplot(1,3,2);
histogram(foundX_5x5(:),'BinWidth',0.01,'EdgeColor','none','FaceColor','k','normalization','pdf');
ylabel('pdf');
xlabel('Found X Pos [px]');
hold on;
fxx=linspace(min(foundX_5x5),max(foundX_5x5),100);
hL=plot(fxx,normpdf(fxx,WIDTH/2,sigPosition),'-r','linewidth',1.5,'displayname','Target');
legend(hL,'location','northeast')
title('Mean 5x5 Filter');
%theta histogram
subplot(1,3,3);
theta = atan2(foundY_5x5-mean(foundY_5x5),foundX_5x5-mean(foundX_5x5));
polarhistogram(theta,360,'displaystyle','stairs');
[~, ~, SymRatio] = CalcSymStats(foundX_5x5,foundY_5x5);
title(sprintf('Symmetry: %g',SymRatio));


%% Create Plot parab5x5
hFig_p5x5 = figure('name','parab5x5','units','inches',...
    'position',[0,0,9,3],...
    'paperunits','inches',...
    'papersize',[9,3],...
    'paperposition',[0,0,9,3]);
movegui(hFig_p5x5,'center');

%image
subplot(1,3,1);
imagesc(Img,[0,255]);
axis image;
colormap gray;
title('Example Image');
hold on;
plot(foundX_p5x5(n),foundY_p5x5(n),'+','LineWidth',1,'markersize',8);

%X histogram
subplot(1,3,2);
histogram(foundX_p5x5(:),'BinWidth',0.01,'EdgeColor','none','FaceColor','k','normalization','pdf');
ylabel('pdf');
xlabel('Found X Pos [px]');
hold on;
fxx=linspace(min(foundX_p5x5),max(foundX_p5x5),100);
hL = plot(fxx,normpdf(fxx,WIDTH/2,sigPosition),'-r','linewidth',1.5,'displayname','Target');
legend(hL,'location','northeast')
title('Parabola 5x5 Filter');
%theta histogram
subplot(1,3,3);
theta = atan2(foundY_p5x5-mean(foundY_p5x5),foundX_5x5-mean(foundX_p5x5));
polarhistogram(theta,360,'displaystyle','stairs');
[~, ~, SymRatio] = CalcSymStats(foundX_p5x5,foundY_p5x5);
title(sprintf('Symmetry: %g',SymRatio));

%% Create Plot classic
hFig_classic = figure('name','classic','units','inches',...
    'position',[0,0,9,3],...
    'paperunits','inches',...
    'papersize',[9,3],...
    'paperposition',[0,0,9,3]);
movegui(hFig_classic,'center');

%image
subplot(1,3,1);
imagesc(Img,[0,255]);
axis image;
colormap gray;
title('Example Image');
hold on;
plot(foundX_classic(n),foundY_classic(n),'+','LineWidth',1,'markersize',8);

%X histogram
subplot(1,3,2);
histogram(foundX_classic(:),'BinWidth',0.01,'EdgeColor','none','FaceColor','k','normalization','pdf');
ylabel('pdf');
xlabel('Found X Pos [px]');
hold on;
fxx=linspace(min(foundX_classic),max(foundX_classic),100);
hL = plot(fxx,normpdf(fxx,WIDTH/2,sigPosition),'-r','linewidth',1.5,'displayname','Target');
legend(hL,'location','northeast')
title('BaryCenter Classic');
%theta histogram
subplot(1,3,3);
theta = atan2(foundY_classic-mean(foundY_classic),foundX_classic-mean(foundX_classic));
polarhistogram(theta,360,'displaystyle','stairs');
[~, ~, SymRatio] = CalcSymStats(foundX_classic,foundY_classic);
title(sprintf('Symmetry: %g',SymRatio));
