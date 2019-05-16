function [dX,dY] = drift(XX,YY,varargin)
% Calculate drift in set of XY data point
% Input:
%   XX: matrix of time-ordered X coorinates
%       [ TimeStep x CoordinateID]
%  
%   YY: matrix of time-ordered Y coorinates
%       [ TimeStep x CoordinateID]
%
% Parameters:
%   'Time',tval: specify timestamp per step
%           if tval is a scalar then it is interpreted as the dT per step
%           if it is a vector then it is the timestamp of each step
%
%   'AverageWindow',val (default=0)
%       specify averaging window to apply to data before computing drift
%       if Time is specified then window is in same units as Time
%       otherwise it is number of steps in XX & YY
%
%   'IncludeData',INC
%       if specified, then it holds flags indicating if a coordinate should
%       be used when computing the drift
%
%   'UseCoordinate',TRUE_FASLE
%       if specified, then it holds flags indicating if columns in XX and
%       YY should be used or ignored
%       must be have same number of elements as size(XX,2)

%% validate XX and YY
assert(all(size(XX)==size(YY)),'XX and YY must be the same size');

%% Input Parameters
p=inputParser;

addParameter(p,'Time',[],@(x) isnumeric(x));
addParameter(p,'AverageWindow',0,@(x) isnumeric(x)&&isscalar(x));
addParameter(p,'IncludeData',true(size(XX)),@(x) isnumeric(x)||islogical(x));
addParameter(p,'UseCoordinate',true(1,size(XX,2)),@(x) isnumeric(x)||islogical(x));

parse(p,varargin{:});

%% Validate Time
Time = p.Results.Time;
if isempty(Time)
    Time = (1:size(XX,1))';
    dT = 1;
elseif isscalar(Time)
    dT = Time;
    Time = (0:size(XX,1)-1)'*dT;
else %Vector time
    assert(numel(Time)==size(XX,1),'Number of Time steps must be same as size(XX,1)');
    dT = nanmean(diff(Time));
end

%% validate include data
assert(all(size(p.Results.IncludeData)==size(XX)),'IncludeData must be same dimensions as XX and YY');

nInc = ~p.Results.IncludeData;
XX(nInc) = NaN;
YY(nInc) = NaN;

%% validate use coord
OutSteps = size(XX,1);

assert(numel(p.Results.UseCoordinate)==size(XX,2),'UseCoordinate must have same number of elements as size(XX,2)');
nUse = ~p.Results.UseCoordinate;
XX(:,nUse) = [];
YY(:,nUse) = [];

if isempty(XX)
    dX = zeros(OutSteps,1);
    dY = zeros(OutSteps,1);
    return;
end


%% Compute Moving Averages and calculate drift
wind = p.Results.AverageWindow;

if wind~=0 %napply moving avg;
    wind = min(1,round(wind/dT));
    XX = movingavg(XX,wind,1);
    YY = movingavg(YY,wind,1);
end

%subtract first to find frame-by-frame shift
%delCols = []; %columns that are all nan
for n=1:size(XX,2)
    first_ind = find(~isnan(XX(:,n))&~isnan(YY(:,n)),1,'first');
    if isempty(first_ind) %all nan in column
        %delCols = [delCols,n];
        continue;
    end
    XX(:,n) = XX(:,n) - XX(first_ind,n);
    YY(:,n) = YY(:,n) - YY(first_ind,n);
end
%XX(:,delCols) = []; %delete all NaN columns
%YY(:,delCols) = [];
 
%average all the drift beads
XX = nanmean(XX,2);
YY = nanmean(YY,2);

%if we still have some nans in the data, set them to zero so that we don't
%apply drift correction during those frames.
XX(isnan(XX)) = 0;
YY(isnan(YY)) = 0;

dX = XX;
dY = YY;


function filtX = movingavg(X,wind,dim)
%% Moving Average with NaN support
% Applies a moving average of length wind to data.
% For odd wind the window is centered around the output index. For even
% wind, wind/2 points to the left and wind/2-1 to the right of each index
% are used.
% NaNs in the data are ignored. Each average is calculated using nanmean()
% 
% X: input matrix
%   if ndim(X)>1 then the average is applied to the first dim (is along
%   columns)
% wind: length of the moving average window;
% dim: (optional, default=1) dim to apply average to
%
%% Daniel T Kovari, 2016

p = inputParser();
p.CaseSensitive = false;

if wind<=1
    filtX = X;
    return;
end

if nargin<3
    if isvector(X)
        if isrow(X)
            dim = 2;
        else
            dim = 1;
        end
    else
        dim = 1;
    end
end
%shift dim into first position
perm = [dim 1:dim-1 dim+1:ndims(X)];
X = permute(X,perm);
szX = size(X);
%reshape to a matrix to make it easier to process n-dim data
X = reshape(X,size(X,1),[]);

%apply moving avg along each column
filtX = NaN(size(X));
for n=1:size(X,1)
    range = n-floor(wind/2):n+ceil(wind/2-1);
    range(range<1) = [];
    range(range>size(X,1)) = [];
    filtX(n,:) = nanmean(X(range,:),1);
end

%undo reshape and permute
filtX = reshape(filtX,szX);
filtX = ipermute(filtX,perm);


    