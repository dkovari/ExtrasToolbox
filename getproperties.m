function props = getproperties(obj,varargin)
%returns a class's properties with a given access state
% Options:
%   'SetAccess',___: Set access restrictions e.g. 'public',
%       {'public','protected'}
%
%   'GetAccess',___: get access restrictions

p = inputParser;
addParameter(p,'SetAccess',[],@(x) isempty(x)||(ischar(x)||(iscellstr(x))&&all(ismember(lower(x),{'public','protected','private'}))));
addParameter(p,'GetAccess',[],@(x) isempty(x)||(ischar(x)||(iscellstr(x))&&all(ismember(lower(x),{'public','protected','private'}))));

parse(p,varargin{:});


md = metaclass(obj);
pl = md.PropertyList;
%% remove complicated properties
badInd = [];
pl_A = {pl.SetAccess}';
for n=1:numel(pl_A)
    if ~ischar(pl_A{n})
        badInd = [badInd,n];
    end
end
pl(badInd) = [];
badInd = [];
pl_A = {pl.GetAccess}';
for n=1:numel(pl_A)
    if ~ischar(pl_A{n})
        badInd = [badInd,n];
    end
end
pl(badInd) = [];

%% SetAccess
sa = true(size(pl));
if ~isempty(p.Results.SetAccess)
    res = lower(p.Results.SetAccess);
    if iscellstr(res)
        res = reshape(res,1,[]);
    end
    
    %% get properties that match sa
    sa = any( ismember({pl.SetAccess}',res),2);
end

%% Get Access
ga = true(size(pl));
if ~isempty(p.Results.GetAccess)
    res = lower(p.Results.GetAccess);
    if iscellstr(res)
        res = reshape(res,1,[]);
    end    
    
    %% get props that match GA
    ga = any( ismember({pl.GetAccess}',res),2);
end

props = {pl(sa&ga).Name}';