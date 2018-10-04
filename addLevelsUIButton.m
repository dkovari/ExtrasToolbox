function addLevelsUIButton(varargin)
%% Add ImageLevelsUI Button to a figure containing an image
% Syntax:
%   extras.addLevelsUIButton()
%       add button to current figure, using first image in the figure as
%       the data source
%   extras.addLevelsUIButton(himg): add button to toolbar of figure containing himg
%   extras.addLevelsUIButton(hfig): add button to figure
%   extras.addLevelsUIButton(__,'FigureHandle',hFig): place tool button on
%       specific figure, even if that figure doesn't contain the image
%   extras.addLevelsUIButton(__,'ImageHandle',himg): use specified image
%       for levels

p = inputParser();
addOptional(p,'graphicsobj',[],@(x) isscalar(x)&&ishghandle(x));
addParameter(p,'FigureHandle',[],@(x) isscalar(x)&&isgraphics(x,'figure'));
addParameter(p,'ImageHandle',[],@(x) isscalar(x)&&isgraphics(x,'image'));

parse(p);

%% look for inputs
hImg = [];
hFig = [];
if isempty(p.Results.graphicsobj)&&isempty(p.Results.FigureHandle)&&isempty(p.Results.ImageHandle)
    %nothing specified
    hFig = gcf;
    
    imgs = findall(hFig,'Type','image');
    
    if isempty(imgs)
        error('No argument was specified: adding buttton to current figure; however, no image was found. Specify image or add image to figure before calling function.');
    end
    hImg = imgs(1);
elseif ~isempty(p.Results.graphicsobj) && strcmp(p.Results.graphicsobj.Type,'image')
    %image specified
    if ~isempty(p.Results.ImageHandle)
        error('image handle specified in first argument and with Name,Value parameters. Only specify image handle once.');
    end
    hImg = p.Results.graohicsobj;
    if isempty(p.Results.FigureHandle)
        hFig = ancestor(hImg,'figure');
    else
        hFig = p.Results.FigureHandle;
    end
elseif ~isempty(p.Results.graphicsobj) && strcmp(p.Results.graphicsobj.Type,'figure')
    %fig specified first
    if ~isempty(p.Results.FigureHandle)
        error('figure handle specified in first argument and with Name,Value parameters. Only specify handle once.');
    end
    hFig = p.Results.graohicsobj;
    if isempty(p.Results.ImageHandle)
        imgs = findall(hFig,'Type','image');
        if isempty(imgs)
            error('No image handle was specified: adding buttton to specified figure; however, no image was found. Specify image or add image to figure before calling function.');
        end
        hImg = imgs(1);
    else
        hImg = p.Results.ImageHandle;
    end
else
    hFig = p.Results.FigureHandle;
    hImg = p.Results.ImageHandle;
    
    if isempty(hFig)&&isempty(hImg)
        hFig = gcf;
    end
    
    if isempty(hImg)
        imgs = findall(hFig,'Type','image');
        if isempty(imgs)
            error('No image handle was specified: adding buttton to specified figure; however, no image was found. Specify image or add image to figure before calling function.');
        end
        hImg = imgs(1);
    end
    
    if isempty(hFig)
        hFig = ancestor(hImg,'figure');
    end
end

%% Setup icon
persistent ico;
if isempty(ico)
    [pth,~,~] = fileparts(mfilename('fullpath'));
    ico = imread(fullfile(pth,'LevelsUI_Button.png'));
end



%% Create button
tbh = findall(hFig,'Type','uitoolbar');

uipushtool(tbh,'CData',ico,...
                    'HandleVisibility','Callback',...
                    'TooltipString','Image Levels Tool',...
                    'ClickedCallback',@(hb,~) buttonPressed(hb,hImg));

end

%% Button down function
function buttonPressed(hBtn,hImg)
    if isempty(hImg)||~isvalid(hImg)
        hFig = ancestor(hBtn,'figure');
        imgs = findall(hFig,'Type','image');
        if isempty(imgs)
            error('No image handle found, cannot create LevelsUI');
        end
        hImg = imgs(1);
    end
    if isempty(hBtn.UserData)||~isfield(hBtn.UserData,'LevelsUI')||~isvalid(hBtn.UserData.LevelsUI)
        %create levelsui
        hBtn.UserData.LevelsUI = extras.ImageLevelsUI(hImg);
    end
    %bring levelsui to front
    figure(ancestor(hBtn.UserData.LevelsUI.Parent,'figure'));
end