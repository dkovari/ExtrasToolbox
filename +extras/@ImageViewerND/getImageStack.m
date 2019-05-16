function ImStack = getImageStack(this,varargin)
% Return stack of images
% Usage:
%   ImStack = obj.getImageStack()
%       Returns all images
%
%   ImStack = obj.getImageStack(R1,R2,R3,...)
%       Returns images over the range
%           ImStack = obj.ImageStack(R1,R2,R3,...);
%       If one of the Rn arrays is set to [] it is interpreted as  
%       the color (:) operator.
%         e.g. with R2=[]
%               ImStack = obj.ImageStack(R1,:,R3);
%       unspecified indicies are also assumed to correspond to : operator
%
% Hints:
%   If ImageViewer is using Bio-Formats to read the data, the Stack
%   dimensions are:
%    Stk = obj.getImageStack(ChannelIndex,TimeIndex,ZIndex,SeriesIndex)

%% prep args
args = cell(1,this.NumStackDimensions);

for n=1:this.NumStackDimensions
    if numel(varargin)<n || isempty(varargin{n})
        args{n} = 1:this.StackDimensions(n);
    else
        assert(isnumeric(varargin{n}),'Specified dimension must be numeric')
        assert(all(isfinite(varargin{n})),'Specified dimensions must be finite and non-nan');
        assert(all(varargin{n}==floor(varargin{n})),'Specified dimensions must be integer');
        assert(all(varargin{n}<=this.StackDimensions(n)),'Specified dimensions must be in range of stack');
        args{n} = varargin{n};
    end
end

%% prep ImStack
sz = zeros(1,numel(args));
for n=1:numel(args)
    sz(n) = numel(args{n});
end
ImStack = cell(sz);

%% create list of indicies
ind = reshape(args{1},[],1);
for n=2:numel(args)
    i2 = reshape(args{n},[],1);
    ind2 = [];
    for j=1:numel(i2)
        ind2 = [ind2;ind,repmat(i2(j),size(ind,1),1)];
    end
    ind = ind2;
end

%% set images
t = tic;
usingWB = false;
hWB = [];
for n=1:size(ind,1)
    if usingWB && ~isvalid(hWB)
        error('getImageStack() canceled by user');
    end
    ImStack{n} = this.getImagePlane(ind(n,:));
    if ~usingWB && toc(t)>0.85 %taking a while, show waitbar
        hWB = waitbar(n/size(ind,1),sprintf('Retrieving ImageStack (%d/%d)',n,size(ind,1)));
        usingWB = true;
        t = tic;
    elseif usingWB && toc(t)>0.8%update waitbar
        waitbar(n/size(ind,1),hWB,sprintf('Retrieving ImageStack (%d/%d)',n,size(ind,1)));
    end
end
%delete waitbar
try
    delete(hWB)
catch
end