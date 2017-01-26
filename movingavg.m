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