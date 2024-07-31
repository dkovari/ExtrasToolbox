function TF = inrange(val,range)
%Return TF if val is in range
% Inputs:
%   val: numeric array of any size
%   range: [1x2] or [numel(val) x 2] or [2 x numel(val)] numeric array 
%          of cell array of the form {[1x2],[1x2],...}
% Output:
%   TF: array same size as val
%       if element in val or range are NaN, TF(n)=NaN
%       otherwise TF(n) is 0/1 indicating if val(n) is respecitve range
%
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.

%% validate inputs

assert(isnumeric(val),'val must be numeric array');
TF = NaN(size(val));

if isnumeric(range)
    range = squeeze(range);
    assert(ndims(range)<=2,'range must be matrix or cell array of [1x2]');
    if numel(range)==2
        range = reshape(range,1,[]);
    else
        if size(range,2)>size(range,1) %put long dimension vertically
            range = range';
        end

    end
    assert(size(range,2)==2,'range must be matrix with size [nx2]');
    
    
    if size(range,1)>1
        assert(size(range,1)==numel(val),'long dimension of range must match numel(val)');
    end

    if size(range,1)==1
        thisRange = range;
        thisRange = sort(thisRange);
    end
    for n=1:numel(val)
        if isnan(val(n))
            TF(n) = NaN;
        else
            if size(range,1)>1
                thisRange = range(n,:);
                thisRange = sort(thisRange);
            end
            if any(isnan(thisRange))
                TF(n) = NaN;
            else
                TF(n) = val(n)>=thisRange(1)&&val(n)<=thisRange(2);
            end
        end
    end
else
    assert(iscell(range)&&all(size(range)==size(val)),'Range must be numeric matrix or cell array with same size as numel');
    for n=1:numel(val)
        thisRange = range{n};
        assert(isnumeric(thisRange)&&numel(thisRange)==2,'elemens of range{} must be numeric with numel==2');
        thisRange = sort(thisRange);
        
        if isnan(val(n))
            TF(n) = NaN;
        elseif any(isnan(thisRange))
            TF(n) = NaN;
        else
            TF(n) = val(n)>=thisRange(1)&&val(n)<=thisRange(2);
        end   
    end 
end

