function addColorbarButton(hfig)
%% Add Button to toggle extras.colorbar to a figure
% usage
%   extras.addColorbarButton()
%   extras.addColorbarButton(hfig)

%% no args
if nargin<1
    hfig = gfc;
end

hfig = ancestor(hfig,'figure');

%% get axes

hcb = extras.colorbar.empty();

    function toggleColorbar()
        
        if isempty(hcb)||~isvalid(hcb)
            hax = hfig.CurrentAxes;
            hcb = extras.colorbar(hax);
        else
            delete(hcb);
        end
    end
%persistent ico;
persistent ico;
if isempty(ico)
    [pth,~,~] = fileparts(mfilename('fullpath'));
    ico = imread(fullfile(pth,'colormap_icon.png'));
end

%% Create button
tbh = findall(hfig,'Type','uitoolbar');

uipushtool(tbh,'CData',ico,...
                    'HandleVisibility','Callback',...
                    'TooltipString','Show/Hide Colorbar',...
                    'ClickedCallback',@(hb,~) toggleColorbar());


end
