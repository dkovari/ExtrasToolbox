%test csaps
close all;
clear all;

noise = 0.01;

nD = 10;
rRange = [0.5,20];

nu_max = 5;
X = linspace(1,nu_max,500);

Y = zeros(nD,numel(X));
for n=1:numel(X)
    Y(:,n) = besselj(X(n),linspace(rRange(1),rRange(2),nD));
end

%Yn = Y + noise*randn(size(Y));

Xn=[X,X,X];
Yn = [Y + noise*randn(size(Y)),Y + noise*randn(size(Y)),Y + noise*randn(size(Y))];

figure(1);clf;
hL = plot(repmat(X',1,nD),Y','--','LineWidth',2);
hold on;
for n=1:nD
    plot(Xn',Yn(n,:)','x','Color',hL(n).Color);
end
xlabel('\nu');
ylabel('J_\nu(r)');

%% Construct csaps directly
xx = linspace(1,nu_max,1000);

% dX = mean(diff(X));
% pp = csaps(Xn,Yn,1/(1+(dX^3)/1));
% 
% Ys = ppval(pp,xx);
% for n=1:nD
%     plot(xx,Ys(n,:)','-','Color',hL(n).Color);
% end

%% Test smoothpchip

%set random elements of Yn to NAN
nBad = 100;
bad_ind = 1+ fix((numel(Yn)-1)*rand(nBad,1));

XXn = repmat(Xn,nD,1);
plot(XXn(bad_ind),Yn(bad_ind),'ok');

Yn(bad_ind) = NaN;


[ph,phr] = extras.ParticleTracking.splineroot_helpers.smoothpchip(Xn,Yn);
Yph = ppval(ph,xx);
hYph = gobjects(0);
for n=1:nD
    hYph(n) = plot(xx,Yph(n,:)',':','Color',hL(n).Color);
end

Yphr = ppval(phr,xx);
hYphr = gobjects(0);
for n=1:nD
   hYphr(n) =  plot(xx,Yphr(n,:)','-.','Color',hL(n).Color);
end

%% Legend
legend([hL(1),hYph(1),hYphr(1)],{'Original Function','Regularized & Simplified','Regularized Only'});
