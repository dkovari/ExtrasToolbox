function varargout = intersect_mixedtype(A,B,varargin)

if nargout > 3
    error('output [C,IA,IB] = intersect_mixedtype(...)');
end

try %try intersect first
    [varargout{1:end}] = intersect(A,B,varargin{:});
catch
end

end