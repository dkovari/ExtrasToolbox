function [x0,y0,sigx,sigy,theta,pci] = fit_ellipsoid(im)
% fit image to gaussian ellipsoid

[xx,yy] = meshgrid(1:size(im,2),1:size(im,1));
XY=[reshape(xx,[],1),reshape(yy,[],1)];

logim = reshape(log(im-min(im(:))+eps),[],1);


[x,resnorm,residual,exitflag,output,lambda,jacobian] = lsqcurvefit(...
    @logEllipsoid,[max(logim),size(im,1)/2,size(im,2)/2,2,2,0],...
    XY,logim);

function F = logEllipsoid(coef,XY)
% coef: [logA,mux,muy,sigx,sigy,theta]
% XY = [X,Y]

F = coef(1) + -(XY(:,1)*cos(coef(6))+XY(:,2)*sin(coef(6))-coef(2)).^2/(2*coef(4)^2)...
    -(-XY(:,1)*sin(coef(6))+XY(:,2)*cos(coef(6))-coef(3)).^2/(2*coef(5)^2);