% Test pixel artifacts

WIDTH = 50;
HEIGHT = 50;

[xx,yy] = meshgrid(1:WIDTH,1:HEIGHT);


%[Xc,Yc] = meshgrid(linspace(10,40,1000),linspace(10,40,1000));
XX = 24+2*rand(1,10000);
[Xc,Yc] = meshgrid(XX,25);

sz = 1.5;

BrightMax = 193;
DarkMin = 134;
AvgIntensity = 160;

noise = 7;
nRep = 100;

foundX = NaN(numel(Xc),nRep);
foundY = NaN(numel(Yc),nRep);

%% Loop over Xc and find centers
hWB = waitbar(0,'proccessing');
for n = 1:numel(Xc)
    %% Contruct image
    Img_Base = AvgIntensity*ones(size(xx));
    Img_Base = Img_Base + (BrightMax-AvgIntensity)*exp( -( (xx-(Xc(n)-sz)).^2 + (yy-(Yc(n)-sz)).^2)/(2*sz^2)) ...
        + (DarkMin-AvgIntensity)*exp( -( (xx-(Xc(n)+sz)).^2 + (yy-(Yc(n)+sz)).^2)/(2*sz^2));
    
    for m=1:nRep
        Img = Img_Base+noise*(2*rand(size(Img_Base))-1);
        Img = uint8(Img);

        %% perform fit
        [X,Y] = ParticleTrackers.BaryCenter(Img,[],5,0.25);

        foundX(n,m) = X;
        foundY(n,m) = Y;
        
    end
    
    if mod(n,50)==0
        figure(1);clf;
        imagesc(Img,[0,255])

        axis image;
        colormap gray;

        hold on;
        plot(X,Y,'+r');

        waitbar(n/numel(Xc),hWB,sprintf('Processing: %d/%d',n,numel(Xc)));
    end

end
delete(hWB);

%% Plot Results
hFig = figure('units','inches',...
    'position',[0,0,8,2],...
    'paperunits','inches',...
    'papersize',[8,2],...
    'paperposition',[0,0,8,2]);
movegui(hFig,'center');

subplot(1,4,1)
plot(repmat(reshape(Xc,[],1),1,nRep)',foundX','.k','markersize',0.5);
xlabel('Xc');
ylabel('foundX');
title('X vs Xc');
hold on;
plot([24,26],[24,26],'--r');
axis equal
axis tight


subplot(1,4,2)
histogram(foundX(:),'BinWidth',0.01,'EdgeColor','none','FaceColor','k');
xlabel('found X [px]');
ylabel('counts');
title('Found Locations')


subplot(1,4,3);
plot(repmat(reshape(Xc,[],1),1,nRep)',(foundX-reshape(Xc,[],1))','.k','markersize',0.5);
ylabel('Xerror : X-Xc');
xlabel('Xc');
hold on;
plot([24,26],[0,0],'--r');
title('Error');

subplot(1,4,4)
Err = (foundX-reshape(Xc,[],1));
histogram(Err(:),'EdgeColor','none','FaceColor','k');
xlabel('Error [px]');
ylabel('counts');
title('Error')

[fname,fdir] = uiputfile('*.pdf','Save Plot');
if fname~=0
    saveas(hFig,fullfile(fdir,fname));
end