function expandAxes(hax)
% expand axes to fill figure

if nargin<1
    hax = gca;
end

assert(isgraphics(hax)&&strcmpi(hax.Type,'axes'),'expandAxes only works on axes type graphics handles');


axis(hax,'tight');
axis(hax,'image');
try
    set(hax,'LooseInset',get(hax,'TightInset')); %expand axes to fill figure
catch
end