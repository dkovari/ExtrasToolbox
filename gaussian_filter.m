function O = gaussian_filter(I, sig_2, SZ,varargin)
%% Gaussian Image Filter
% Applys a gaussian filter to I.  Images can be logical, numeric.
% Operation is performed only along the m-n plane, so RGB imagescan also be
% used.
%% Inputs
%   I:      Input image, logical, numeric, [m,n,c]
%	sig_2:  square of the "standard deviation" of the gaussian
%   SZ:     size of the filter, if single valued then assumed to be square
%           otherwise, two dimensions are required.
% Options: list options as a series of items
%  Input padding options
%   'symmetric' pad matrix by mirroring the border
%   'pad' or 'replicate' replicate the edges
%   'circular'  periodic boundaries
%   'PadValue',X    pad matrix with value specified in X, if no other
%                   options are specified, the default is 0
%  Output sizes
%   'same'  same size as the input image, default
%   'full'  fullsize, including added padding
%  Filter Methods
%   'corr' uses matrix correlation, default. (Optiomzed for Intel CPUs)
%   'conv' uses convolution 
% See imfilter for more details

%% Script Initialization & Input parsing
if nargin < 3
    error('not enough inputs specified');
end
if ~isnumeric(sig_2)
    error('sig_2 must be numeric');
end
if ~isnumeric(SZ)&&(numel(SZ)>2||numel(SZ)<1)
    error('SZ must be either a single value or 2 values');
end
if numel(SZ)==1
    SZ=[SZ,SZ];
end
%default options
PadOpt = 'PadValue';
PadVal = 0;
OutSz = 'same';
Fmeth='corr';
if nargin>3
    if sum(strlistcmpi({'PadValue','symmetric','replicate','circular','pad'},varargin))>1
        error('you can only specify one padding option');
    end
    if any(strcmpi('PadValue',varargin))
        xi = 1+find(strcmpi('PadValue',varargin));
        PadOpt = 'PadValue';
        PadVal = varargin{xi};
    end
    if any(strcmpi('symmetric',varargin))
        PadOpt = 'symmetric';
    end
    if any(strlistcmpi({'replicate','pad'},varargin))
        PadOpt = 'replicate';
    end
    if any(strcmpi('circular',varargin))
        PadOpt = 'circular';
    end
    
    if sum(strlistcmpi({'same','full'},varargin))>1
        error('you can only specify one output size option');
    end
    if any(strcmpi('same',varargin))
        OutSz = 'same';
    end
    if any(strcmpi('full',varargin))
        OutSz = 'same';
    end
    
    if sum(strlistcmpi({'corr','conv'},varargin))>1
        error('you can only specify one output size option');
    end
    if any(strcmpi('corr',varargin))
        Fmeth = 'corr';
    end
    if any(strcmpi('conv',varargin))
        Fmeth = 'conv';
    end
end
%% Compute Filter
filt = fspecial('gaussian',SZ,sqrt(sig_2));

if strcmpi(PadOpt,'PadValue')
    O = imfilter(I,filt,PadVal,OutSz,Fmeth);
else
    O = imfilter(I,filt,PadOpt,OutSz,Fmeth);
end

function res = strlistcmpi(listcell,strcell)
% Returns a logical array specifying if the elements of strcell match with
% any of the strings specified in listcell

res = zeros(size(strcell));

if ~iscell(listcell)
    if ~ischar(listcell)
        error('listcell should be either a cellstr or string');
    end
    res = strcmpi(listcell,strcell);
    return;
end

for idx = 1:numel(listcell)
    str = listcell{idx};
    res = res|strcmpi(str,strcell);
end
