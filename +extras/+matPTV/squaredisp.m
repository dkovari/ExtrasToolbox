function [sqdisp,tau,msd_fit,scF] = squaredisp(Position,Time, varargin)
% Compute Square Displacement of a trajectory as a function of Time
%
% Input:
%   Positions: Time-ordered position coordinates
%       If position is multi-dimensional (e.g. X,Y,Z) then time steps
%       should correspond to rows in position:
%           [X1, Y1, Z1;
%            X2, Y2, Z2;
%                ...   ]
%       Note: If multiple dimension are included in Position then
%       displacement is computed as:
%           d^2 = sum((Pos(t+tau,:)-Pos(t+tau,:).^2)
%          in other words for X,Y,Z that would be
%              d^2 = (X-Xo)^2 + (Y-Yo)^2 + (Z-Zo)^2
%
%  Time: (optional) time point of each step
%       If Time is not specified, a default time step dT=1 is assumed.
%       If Time is a scalar then it is interpreted as dT=Time, this
%       duration between successive steps.
%       Otherwise Time should be a vector with the same number of elements
%       as rows in Position.
%
% Output:
%   sqdisp: list of all square displacement pairs
%       Example:
%          = [ (P(2)-P(1))^2;
%              (P(3)-P(2))^2;
%               ...
%              (P(3)-P(1))^2;
%              (P(4)-P(2))^2;
%                   ...      ]
%
%   tau: list of time-step difference for results in sqdisp
%
%   MSD_fit: (optional) Compute the best fitting MSD curve
%           sqdisp(t) = a*t^b
%       output is MATLAB fitobject (see cfit for details)
%
% Optional Parameters:
%   'ShowWaitbar',true/false (default=true)
%
%   'AverageResults',true/false (default=false)
%           if true, then duplicate time-steps in are averated together
%           in which case sqdisp will be a matrix of the form
%               [ sqdisp(:), StdDev(:), nRep(:)]

%% Make row->column
if isrow(Position)
    Position = Position';
end
    
%% Handle time
if ~exist('Time','var')
    Time = (1:size(Position,1))';
elseif isscalar(Time)
    Time = (Time*0:size(Position,1)-1)';
else
    assert(numel(Time)==size(Position,1),'Time must be scalar or same numel as number of positions');
end

%% Parse optional args
prs=inputParser;
addParameter(prs,'ShowWaitbar',true,@isscalar);
addParameter(prs,'AverageResults',false,@isscalar);
parse(prs,varargin{:});

%% prep results
nPos = size(Position,1);

res(nPos-1) = struct('dT',NaN,'sD',NaN);

%% compute square displacements for all intervals
tic1 = tic;
hWB = [];
for n=1:nPos-1
    res(n).dT = (Time(n+1:end)-Time(n))';
    res(n).sD = (sum((Position(n+1:end,:) - Position(n,:)).^2,2))';
    if prs.Results.ShowWaitbar&&toc(tic1)>0.8
        if isempty(hWB)
            hWB = waitbar(n/(nPos-1),sprintf('Computing Sq. Disp %d/%d',n,nPos-1));
        else
            try
                waitbar(n/(nPos-1),hWB,sprintf('Computing Sq. Disp %d/%d',n,nPos-1));
            catch
                error('Canceled by user');
            end
        end
    end
end
try
    delete(hWB);
catch
end

%% Sort results
tau = [res.dT];
sqdisp = [res.sD];

%remove nans
badInd = find(isnan(sqdisp));
tau(badInd) = [];
sqdisp(badInd) = [];

[tau,ind] = sort(tau);
tau = tau';
sqdisp = sqdisp(ind)';

%% Compute fit
if nargout>2
    %scale factor
    scF = 1;%10/max(sqdisp)';
    
    %guess starting point using log-log lls
    pp = polyfit(log(tau),log(sqdisp+eps),1)
    
    try
    fop = fitoptions('power1',...
        'Weights',tau.^-4,...
        'Lower',[0,0],...
        'Upper',[Inf,10*pp(1)],...
        'StartPoint',[max(0,pp(2)),pp(1)]);
    msd_fit = fit(tau,sqdisp,'power1',fop);
    catch ME
        figure(1000);
        loglog(tau,sqdisp+eps,'.k','Markersize',3);
        hold on;
        loglog(tau,exp(polyval(pp,log(tau))),'--r');
        throw(ME)
    end
end

%% Handle Replicated
if prs.Results.AverageResults
    [tau,~,ic] = unique(tau);
    sd2 = NaN(numel(tau),3);
    for n=1:numel(tau)
        id = find(ic==n);
        sd2(n,1) = mean(sqdisp(id));
        sd2(n,2) = std(sqdisp(id));
        sd2(n,3) = numel(id);
    end
    sqdisp = sd2;
end
