function expandAxes(hax,axis_opts)
% expand axes to fill figure
%
% hAx: axes handle to expand
% axis_opts: (optional) char array specifying extra axis option to apply
%   e.g. axis_opts = 'image' to force square pixel scale format
% you can specify multiple axis options by passing a cellstr array
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.

if nargin<1
    hax = gca;
end

assert(isgraphics(hax)&&strcmpi(hax.Type,'axes'),'expandAxes only works on axes type graphics handles');


axis(hax,'tight');
if nargin>1
    asert(ischar(axis_opts)||iscellstr(axis_opts),'axis_opts must be char or cellstr');
    if iscellstr(axis_opts)
        for n=1:numel(axis_opts)
            axis(hax,axis_opts{n});
        end
    else
        axis(hax,axis_opts);
    end
end
%axis(hax,'image');
try
    set(hax,'LooseInset',get(hax,'TightInset')); %expand axes to fill figure
catch
end