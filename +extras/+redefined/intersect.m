function varargout = intersect(A,B,varargin)
%   [C,IA,IB] = INTERSECT(A,B) also returns index vectors IA and IB such
%   that C = A(IA) and C = B(IB). If there are repeated common values in
%   A or B then the index of the first occurrence of each repeated value is
%   returned.


if nargout > 3
    error('output [C,IA,IB] = intersect_mixedtype(...)');
end

% Determine the number of outputs requested.
if nargout == 0
    nlhs = 1;
else
    nlhs = nargout;
end

try %try intersect first
    [varargout{1:nlhs}] = intersect(A,B,varargin{:});
catch % that didn't work try slow version
    if ~iscell(A) && ~iscell(B)
        error('DAN FIX ME! Neither A nor B is a cell, not sure why we''re here');
    end
    
    if ~iscell(A)
        A = {A};
    end
    
    if ~iscell(B)
        B = {B};
    end
    
    IA = [];
    IB = [];
    
    for n=1:numel(A)
        %A{n}
        for m=1:numel(B)
            %B{m}
            if isequal(A{n},B{m})
                
                IA = [IA,n];
                IB = [IB,m];
                break
            end
        end
        %don't output duplicates in a
        [IB,ib] = unique(IB);
        IA = IA(ib);
    end
    if ~isempty(IA)
        C = A(IA);
    else
        C = {};
    end
    
    if numel(C)==1
        C = C{1};
    end
    
    varargout{1} = C;
    if nlhs>1
        varargout{2}=IA;
    end
    if nlhs>2
        varargout{3}=IB;
    end
end

end