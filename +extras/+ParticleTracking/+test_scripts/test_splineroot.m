% Test splineroot
close all
clear all
%% Construct Data for generating spline

nD = 10;
rRange = [0.5,20];

nu_min = 1;
nu_max = 5;
X = linspace(nu_min,nu_max,100);
Y = zeros(nD,numel(X));
for n=1:numel(X)
    Y(:,n) = besselj(X(n),linspace(rRange(1),rRange(2),nD));
end

%construct noisy data
data_noise = 0.005;
Xn=[X,X,X];
Yn = [Y + data_noise*randn(size(Y)),Y + data_noise*randn(size(Y)),Y + data_noise*randn(size(Y))];

%% Construct smoothing spline
ph = smoothpchip(Xn,Yn);

%% Plot Spline Results
figure(1);clf;
xx = linspace(nu_min,nu_max,1000);

hL = plot(repmat(xx,nD,1)',ppval(ph,xx)','-');
ylabel('J_\nu[r]');
ylabel('\nu');

hold on;
%plot raw data
hLn = plot(repmat(Xn,nD,1)',Yn','x','MarkerSize',6);
for n=1:numel(hLn)
    hLn(n).Color = hL(n).Color;
end

%% Generate Test Data and Plot
sig_noise = 0.01;
xc =  nu_min+(nu_max-nu_min)*rand();

YYn = ppval(ph,xc)+sig_noise*randn(nD,1);
hAx = gca;
YLIM = hAx.YLim;

hold on;
plot([xc,xc],YLIM,'--k','LineWidth',0.5);

XLIM = hAx.XLim;
hLn = plot(repmat(XLIM,nD,1)',[YYn,YYn]',':');
for n=1:numel(hLn)
    hLn(n).Color = hL(n).Color;
end
plot(repmat(xc,nD,1),YYn,'.k','MarkerSize',10);

%% Test splineroot
[z,varz] = splineroot(YYn,ph);

plot([z,z],YLIM,'-r');
plot([z,z]-2*sqrt(varz),YLIM,'-.r');
plot([z,z]+2*sqrt(varz),YLIM,'-.r');

%% Test Splineroot speed
fprintf('Running speed test\n');
tic;
for n=1:size(Yn,2)
    [z,varz] = splineroot(Yn(:,n),ph);
end
t = toc;
fprintf('\tAverage Time: %f (# runs: %d)\n ',t/size(Yn,2),size(Yn,2));
