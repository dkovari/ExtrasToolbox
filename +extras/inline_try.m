function inline_try(fn,varargin)
% inline way to add try/catch to function

try
    fn(varargin{:});
catch ME
    disp(ME.getReport);
end