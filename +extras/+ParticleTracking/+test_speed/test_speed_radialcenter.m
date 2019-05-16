%% Generate Test Images
Nx = 10;
Ny = 4;
WIDTH = 1000;
HEIGHT = 500;
N_IMAGES = 10;

[xx,yy] = meshgrid(1:WIDTH,1:HEIGHT);


Rfn = @(r) (0.5+r-r.^3).*sinc(r/5).*(1./(1+exp(r-(WIDTH/(Nx+1)*0.4))));

Xc = (1:Nx)*WIDTH/(Nx+1);
Yc = (1:Ny)*HEIGHT/(Ny+1);

[Xc,Yc] = meshgrid(Xc,Yc);

I = cell(N_IMAGES,1);

hWB = [];
t0 = tic;
for n=1:N_IMAGES
    
    Xc1 = Xc + 15*(rand(size(Xc))-0.5);
    Yc1 = Yc + 15*(rand(size(Yc))-0.5);
    Xc1 = reshape(Xc1,[],1);
    Yc1 = reshape(Yc1,[],1);

    I{n} = zeros(HEIGHT,WIDTH);

    for jj = 1:numel(Xc1)
        rr = sqrt( (xx-Xc1(jj)).^2 + (yy-Yc1(jj)).^2);
        I{n} = I{n} + Rfn(rr);
    end
    
    if toc(t0)>0.85
        if isempty(hWB)
            hWB = waitbar(n/N_IMAGES,'Generating Images');
        else
            waitbar(n/N_IMAGES,hWB);
        end
        t0=tic;
    end
    
end
try
    delete(hWB);
catch
end

%% Run calculation
WIND = [Xc(:),Yc(:),zeros(numel(Xc),1),zeros(numel(Yc),1)] + [-WIDTH/(Nx+1)*0.4,-WIDTH/(Nx+1)*0.4,WIDTH/(Nx+1)*0.8,WIDTH/(Nx+1)*0.8];

dt = zeros(N_IMAGES,1);
t1=tic;
for n=repmat(1:N_IMAGES,1,10)
    tt=now();
    [X,Y,varXY,d2] = extras.ParticleTracking.radialcenter(I{n},WIND,'RadiusCutoff',50,'DistanceExponent',0);
    dt(n) = (now-tt)*24*3600;
end
dT = toc(t1);

fprintf('Avg. duration for radialcenter: %g sec, f=%g Hz\n',dT/N_IMAGES/10,10*N_IMAGES/dT);
histogram(dt);
xlabel('Duration [sec.]');