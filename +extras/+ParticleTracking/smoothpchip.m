function [ph,phr] = smoothpchip(x,Y,varargin)
% Create a multi-dimensional (mapping 1D->nD) smoothing cubic hermite 
% spline from descrete data.
% The resulting spline is initially constructed using cubic smoothing
% splines, regularized and simplified into a piecewise smooth hermite
% spline.
% Simplification involves recursively reducing the number of knots within
% regions of the spline so that smoothing spline (which may have many
% pieces) is expresses with as few pieces as possible.
%
% Unlike direct calls to csaps(x(:),Y([nD],:),...) which require
% all elements of Y and x to be non-NaN values, smoothpchip ignores NaN
% values.
%
% Inputs:
%   x:  vector specifying the input values of the resulting function
%   Y:  Matrix specifying dependent values of the resulting function when
%       evaluated at x
%       NOTE: the long dimension of x must correspond with the same
%       dimension in y
%
% Outputs:
%   ph: the smoothed and simplified n-dimensional spline
%   phr: the smoothed and regularized spline (before simplification)
%
% Options
%   'RegularizationTolerance' (default=0.01):
%       Maximum average fractional error allowed when performing initial
%       conversion from independent smoothing splines to a single
%       n-dimensional cubic hermite spline
%           Error is calculated at each knot of the initial set of
%           smoothing splines as:
%               Err = |RegSpVal - KnotVal|/range(KnotVals)
%               where KnotVals corresponds to all the original knot values
%           Tolerance is compared to the maximum of Err for all points.
%
%   'SimplifyTolerance' (default=0.025):
%       Maximum average fractional error allowed when recursively simplifying
%       the final spline
%           Error is calculated at each knot of the regularized spline:
%               Err = |SimpleSpVal - KnotVal|/range(KnotVals)
%               where KnotVals corresponds to all the knot values in the
%               regularized spline
%               SimpleSpVal are the values for the proposed simplified
%               spline at the same points as KnotVals
%           Tolerance is compared to the maximum of Err for all points.
%
%   'SmoothingParameter' (default = 1/(1+mean(diff(unique(x)))^3) )
%       Smoothing parameter used when constructing the smoothing spline
%       see MATLAB's native CSAPS() function for details.
%
% Copyright 2018, Daniel T. Kovari, Emory University
% All rights reserved.

%% Parse Inputs
p = inputParser;
p.CaseSensitive = false;

addParameter(p,'SmoothingParameter',NaN,@(x) isnumeric(x)&&isscalar(x));
addParameter(p,'RegularizationTolerance',0.01,@(x) isnumeric(x)&&isscalar(x));
addParameter(p,'SimplifyTolerance',0.025,@(x) isnumeric(x)&&isscalar(x));

parse(p,varargin{:});

%% Figure out dipendent dimension
assert(isvector(x),'x must be a vector');
dep_dim = find(size(x)~=1);
assert(ismatrix(Y),'Y must be a matrix');
assert(size(Y,dep_dim)==numel(x),'Y must be a matrix with dependent dimension match length of x');

%reorient so x is horizontal and Y's columns correspond to elements in x
x = reshape(x,1,[]);
if dep_dim==1
    Y = Y';
end
% x = [x1,x2,x3...]
% Y = [Y1(x1),Y1(x2),...;
%      Y2(x1),Y2(x2),...
%           ...         ]

%% Remove NaNs from x
bad = find(isnan(x));
Y(:,bad) = [];
x(bad) = [];

%% sort x in increasing order
[x,ord] = sort(x);
Y = Y(:,ord);

%% Construct smoothing splines
smoothp = p.Results.SmoothingParameter;
if isnan(smoothp)
    smoothp = 1/(1+mean(diff(unique(x)))^3);
end

for n=size(Y,1):-1:1
    good = find(~isnan(Y(n,:)));
    pp(n) = csaps(x(good),Y(n,good),smoothp);
end

%% Regularize splines
ph = regularize_pchip_set(pp,p.Results.RegularizationTolerance);

%% Merge into a single spline
ph = pp2ndim(ph);
phr = ph;
%% Simplify spline to have a minimual number of breaks and still preserve the same shape
ph = simplify_pchip2(ph,p.Results.SimplifyTolerance);

end

function [ph,breaks] = regularize_pchip_set(pp,TOL)
%Convert a set of pp splines into a set of piecewise cubic herimite splines
% with the same number of pieces

%% Paramameters

maxiter = max([pp.pieces]);
upper_breaks = max([pp.pieces])+1;

lower_breaks = min([pp.pieces])+1;


%% Find break start/end for all pp
break_start = Inf;
break_end = -Inf;
for n=1:numel(pp)
    break_start = min(break_start,pp(n).breaks(1));
    break_end = max(break_end,pp(n).breaks(end));
end

%% init
ppv = cell(numel(pp),1);
ppv_range = zeros(numel(pp),1);
for n=1:numel(pp)
    ppv{n} = ppval(pp(n),pp(n).breaks);
    ppv_range(n) = range(ppv{n});
end

    function [ph,mErr] = Calc_ph(breaks)
        ph(numel(pp)) = struct('form','pp','breaks',[],'coefs',[],'pieces',0,'order',4,'dim',1);
        mErr = zeros(numel(pp),1);
        for ind=1:numel(pp)
            %% calc new pchip
            ph(ind) = pchip(breaks,ppval(pp(ind),breaks));
            mErr(ind) = max(abs( (ppval(ph(ind),pp(ind).breaks) -ppv{ind})/ppv_range(ind) ));
        end
    end

%% Test with max num of breaks
breaks = linspace(break_start,break_end,upper_breaks);

[ph,mErr] = Calc_ph(breaks);
Err_upper = max(mErr);

if Err_upper>=TOL
    warning('Could not regularize breaks and maintain tolerance.');
    return;
end

%% Computer lower limit
breaks = linspace(break_start,break_end,lower_breaks);
[ph,mErr] = Calc_ph(breaks);
Err_lower = max(mErr);
if Err_lower<TOL
    return;
end

%% bisect breaks and test
itr = 0;
while Err_upper<TOL && itr<maxiter && upper_breaks>(lower_breaks+1)
    prev_lower = lower_breaks;
    lower_breaks = floor((upper_breaks+lower_breaks)/2);
    
    this_breaks = linspace(break_start,break_end,lower_breaks);
    
    %% Calc New Splines
    [this_ph,mErr] = Calc_ph(this_breaks);
    Err_lower = max(mErr);
    
    if Err_lower<TOL
        ph = this_ph;
        breaks=this_breaks;
        Err_upper = Err_lower;
        upper_breaks = lower_breaks;
        lower_breaks = prev_lower;
    end
    itr = itr+1;
end
end

function ph1 = pp2ndim(ph)

ph1 = ph(1);
ph1.dim = numel(ph);

COEF = NaN(ph1.pieces,ph1.order,numel(ph));
for n=1:numel(ph)
    COEF(:,:,n) = ph(n).coefs;
end
COEF = reshape(permute(COEF,[3,1,2]),[],ph1.order);

ph1.coefs = COEF;
end

function pp2 = simplify_pchip2(pp,err)
% Reduce the number of knots in a spline using piecewise cubic hermite splines
%
% Input:
%   pp: spline to simplify (it can be mulit-dimensional), should be in
%   standard pp-form
%
%   err: maximum allowable average percent error between simplified spline
%       and original spline
%       Calculated as:
%           r = sqrt(mean(mean( ((SimpVal-OrigVal)./OrigVal).^2)));
%
% Note: the function uses recursion, you may need to set the system 
% recursion limit higher if you have a large spline and want an accurate
% resulting curve.
%
% Copyright 2017, Daniel T. Kovari, Emory University
% All rights reserved.

BRK = pp.breaks; %original breaks
pp_vals = ppval(pp,BRK);
pp_range = max(pp_vals,[],2) - min(pp_vals,[],2);

XX= linspace(pp.breaks(1),pp.breaks(end),5);
pp2 = pchip(XX,ppval(pp,XX));


rc(3,1,5);

    function mErr = CalcErr(pp2,brks)
        mErr = max(max( abs(bsxfun(@rdivide,(ppval(pp2,brks) - ppval(pp,brks)),pp_range)) ));
    end

    function [x1,x2,xm] = run_pch(x1,x2,xm)
        
        x1v = pp2.breaks(x1);
        x2v = pp2.breaks(x2);
        xmv = pp2.breaks(xm);
        
        b = pp2.breaks(x1:x2);
        db_2 = diff(b)/2;
        b = [reshape([b(1:end-1);b(1:end-1)+db_2],1,[]),b(end)];

        bstart = max(1,x1-2);
        brk = pp2.breaks(bstart:x1-1);

        brk = [brk,b];

        bend = min(numel(pp2.breaks),x2+2);
        brk=[brk, pp2.breaks(x2+1:bend)];
        
        
        spp = pchip(brk,ppval(pp,brk));
        
        %% combine with last pp2
        if x1-2==bstart
            spp_start = 2;
        else
            spp_start = 1;
        end

        if bend==x2+2
            spp_end = numel(spp.breaks)-1;
        else
            spp_end = numel(spp.breaks);
        end

        pp_start = x1-2;
        pp_end = x2+2;
      
        breaks = [pp2.breaks(1:pp_start),...
            spp.breaks(spp_start:spp_end),...
            pp2.breaks(pp_end:end)];
        
        %% reshape coefs so we can concatenate them
        sppc = reshape(spp.coefs.',spp.order,spp.dim,spp.pieces);
        pp2c = reshape(pp2.coefs.',pp2.order,pp2.dim,pp2.pieces);
        
        
        %% concatenate coefs
        coefs = cat(3,pp2c(:,:,1:pp_start),...
                      sppc(:,:,spp_start:spp_end-1),...
                      pp2c(:,:,pp_end-1:end));
                  
        pieces = size(coefs,3); %new number of pieces
                  
        coefs = (reshape(coefs,pp2.order,[])).'; %reshape back to matrix form
        
        %% assign back to pp2
        pp2.breaks = breaks;
        pp2.coefs = coefs;
        pp2.pieces = pieces;
        
        %% update index locations, we are using find cause i'm dumb
        x1 = findInAscending(x1v,pp2.breaks);
        x2 = findInAscending(x2v,pp2.breaks);
        xm = fix(findInAscending(xmv,pp2.breaks));
        
    end

    function rc(xm_brk,x1_brk,x2_brk)
        
        x1 = pp2.breaks(x1_brk);
        xm = pp2.breaks(xm_brk);
        x2 = pp2.breaks(x2_brk);
        
        %% calc err on left and right
        LBR = BRK(BRK>=x1&BRK<=xm);
        RBR = BRK(BRK>=xm&BRK<=x2);
        
        lr = CalcErr(pp2,LBR);
        rr = CalcErr(pp2,RBR);
        
        %lr2 = mean( sum((ppval(pp2,LBR)-ppval(pp,LBR)).^2,1));
        %rr2 = mean( sum((ppval(pp2,RBR)-ppval(pp,RBR)).^2,1));
        
        %% Subdivide if error too high
        if lr>err %lr2>r2 %process left
            
            x1L = x1_brk;
            x2L = xm_brk;
            xmL = fix(mean([x1L,x2L]));
            
            [x1L,x2L,xmL] = run_pch(x1L,x2L,xmL);            
            
            rc(xmL,x1L,x2L);
            
            xm_brk = fix(findInAscending(xm,pp2.breaks));
            x2_brk = fix(findInAscending(x2,pp2.breaks));
        end
        if rr>err %rr2>r2 %process right
            x1R = xm_brk;
            x2R = x2_brk;
            xmR = fix(mean([x1R,x2R]));
            [x1R,x2R,xmR] = run_pch(x1R,x2R,xmR);
            rc(xmR,x1R,x2R);
        end
    end       
        
end

function k = findInAscending(v,X)
%find index of v in ascending vector X
% if v is not in x, return 1/2 interval indicating where v would lie

low = 1;
up = numel(X);
while true
    
    if v<X(low)||v>X(up)
        k=NaN;
        return;
    end
    
    i = fix( (low+up)/2);
    
    if v==X(low)
        k=low;
        return;
    elseif v==X(up)
        k=up;
        return;
    elseif v==X(i)
        k=i;
        return;
    elseif v<X(i)
        if i-1<=low
            k=low+0.5;
            return;
        end
        up = i;
    elseif v>X(i)
        if i+1>=up
            k=up-0.5;
            return;
        end
        low = i;
    end   
end

end
