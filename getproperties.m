function props = getproperties(obj,varargin)
%returns a class's properties with a given access state

p = inputParser;
addParameter(p,'SetAccess',[],@(x) isempty(x)||(ischar(x)||(iscellstr(x))&&all(ismember(lower(x),{'public','protected','private'}))));
addParameter(p,'GetAccess',[],@(x) isempty(x)||(ischar(x)||(iscellstr(x))&&all(ismember(lower(x),{'public','protected','private'}))));

parse(p,varargin{:});


md = metaclass(obj);
pl = md.PropertyList;

sa = true(size(pl));
if ~isempty(p.Results.SetAccess)
    res = lower(p.Results.SetAccess);
    if iscellstr(res)
        res = reshape(res,1,[]);
    end
    
    sa = any( ismember({pl.SetAccess}',res),2);
end

ga = true(size(pl));
if ~isempty(p.Results.GetAccess)
    res = lower(p.Results.GetAccess);
    if iscellstr(res)
        res = reshape(res,1,[]);
    end
    
    ga = any( ismember({pl.GetAccess}',res),2);
end

props = {pl(sa&ga).Name}';