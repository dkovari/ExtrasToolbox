classdef stackviewer < extras.RequireGuiLayoutToolbox & extras.GraphicsChild
%% Interactive viewer for displaying image stacks or video clips
%
% Usage:
% stackviewer([HxWxN])
% stackviewer([HxWx3xN])
% stackviewer({[HxW],[HxW],...})
% stackviewer('path/to/images/*.tif')
% stackviewer('path/to/images')
% stackviewer({path/img1.tif','path2/img2.tif',...})
% stackviewer(Structwithnames) <-- use with dir(): e.g stackviewer(dir(mytiffs_*.tif)) 
%
% stackviewer(fig_handle,__)
% stackviewer('Parent',fig_handle,'StackData',__)
%
% Properties
%------------
% StackData (observable):
%   [HxW,N] or [HxWx3xN] numeric array specifying images
%   {[H1xW1],[H2xW2],...} cell array specifying images
%
% CurrentFrame (observable):
%   Scalar specifying current frame to display
%
% UserData (observable):
%   storage space for users variables
%
% ChangeFrameCallback (observable)
%   callback function executed after each time the frame is changed.
%       ==> fn(obj,imagedata)
%
% NumberOfFrames (observable):
%   scalar specifying number of frames in the stack
%
% UseImageFiles:
%   T/F specifying if images on the disk are being used
%
% StackFileList:
%   struct specifying image file location
%
% CurrentImageData:
%   dependent property, numeric array [WxH] or [WxHx3] containing current
%   pixel data

    %% Public properties
    properties (SetObservable=true,AbortSet=true)
        StackData
        CurrentFrame = 1;
        
        UserData
        ChangeFrameCallback
        
        XData;
        YData;
    end
    methods
        function set.XData(this,val)
            assert(isnumeric(val)&&numel(val)==2,'XData must be numeric with 2 values');
            set(this.Image,'XData',val);
            this.XData = this.Image.XData;
        end
        function set.YData(this,val)
            assert(isnumeric(val)&&numel(val)==2,'YData must be numeric with 2 values');
            set(this.Image,'YData',val);
            this.YData = this.Image.YData;
        end
    end

    properties (SetAccess=protected,SetObservable=true,AbortSet=true)
        NumberOfFrames = 0;
        UseImageFiles = false
        StackFileList = struct('name',{},'folder',{},'date',{},'bytes',{},'isdir',{},'datenum',{});
    end
    
    %% dependent
    properties (Dependent)  
        CurrentImageData
        %XData
        %YData
    end
    methods        
        function val = get.CurrentImageData(this)
            
            if this.UseImageFiles
                if isempty(this.StackData{this.CurrentFrame})
                    if isfield(this.StackFileList,'folder')
                        pth = fullfile(this.StackFileList(this.CurrentFrame).folder,this.StackFileList(this.CurrentFrame).name);
                    else
                        pth = this.StackFileList(this.CurrentFrame).name;
                    end
                    this.StackData{this.CurrentFrame} = imread(pth);
                end
                val = this.StackData{this.CurrentFrame};
            elseif isempty(this.StackData)
                val = [];
            elseif iscell(this.StackData)
                val = this.StackData{this.CurrentFrame};
            elseif isnumeric(this.StackData) && ndims(this.StackData)==3
                val = this.StackData(:,:,this.CurrentFrame);
            elseif isnumeric(this.StackData) && ndims(this.StackData)==4
                val = this.StackData(:,:,:,this.CurrentFrame);
            else
                warning('StackData is not properly initialized, Current Frame cannot be returned');
                val = [];
            end
        end
    end
    
    %% Internal Set
    properties (SetAccess=protected)
        Image
        ImageAxes
        Slider
        OuterVBox
        
        LevelsUI
    end
    
    %% Create figure method
    methods(Hidden,Static)
        function hFig = createFig()
            hFig = figure('Name','Image Stack Viewer',...
                    'NumberTitle','off',...
                    'MenuBar','none',...
                    'ToolBar','figure');
           
            %% disable some of the toolbar controls
            %New Figure
            delete(findall(hFig,'ToolTipString','New Figure'));
            %Open File
            delete(findall(hFig,'ToolTipString','Open File'));
            %Print Figure
            delete(findall(hFig,'ToolTipString','Print Figure'));
            %Edit Plot
            delete(findall(hFig,'ToolTipString','Edit Plot'));
            %Rotate 3D
            delete(findall(hFig,'ToolTipString','Rotate 3D'));
            %Data Cursor
            delete(findall(hFig,'ToolTipString','Data Cursor'));
            %Brush/Select Data
            delete(findall(hFig,'ToolTipString','Brush/Select Data'));
            %Link Plot
            delete(findall(hFig,'ToolTipString','Link Plot'));
            %Insert Legend
            delete(findall(hFig,'ToolTipString','Insert Legend'));
            %Hide Plot Tools
            delete(findall(hFig,'ToolTipString','Hide Plot Tools'));
            %Show Plot Tools and Dock Figure
            delete(findall(hFig,'ToolTipString','Show Plot Tools and Dock Figure'));
            
        end
    end
    
    %% Create/Delete
    methods
        function this = stackviewer(varargin)
            %% Setup Parent
            %initiate graphics parent related variables
            this@extras.GraphicsChild(@() extras.stackviewer.createFig());
            
            %look for parent specified in arguments
            varargin = this.CheckParentInput(varargin{:});
            
            %% Create Custom Toolbar button for histogram levels
            tbh = findall(this.ParentFigure,'Type','uitoolbar');
            uipushtool(tbh,'CData',extras.ImageLevelsUI.ToolbarIcon,...
                'HandleVisibility','Callback',...
                'TooltipString','Image Levels Tool',...
                'ClickedCallback',@(~,~) this.showLevelsUI());
            
            %% Create Graphical elements
            this.OuterVBox = uix.VBox('Parent',this.Parent);
            
            %create slider
            this.Slider = extras.LabeledSlider('Parent',this.OuterVBox,'Callback',@(~,~) this.UIchangeframe(),'Min',0,'Max',1,'Value',1,'SliderStep',[1,1]);
            
            %imagebox = uix.Panel('Parent',this.OuterVBox,'BorderType','none','BorderWidth',0);
            
            %Create image
            this.ImageAxes = axes('Parent',this.OuterVBox,'NextPlot','replacechildren');
            
            this.ParentFigure.CurrentAxes = this.ImageAxes;
            
            this.Image = imagesc('Parent',this.ImageAxes,'HandleVisibility','off');
            
            %% Modify axes to look nice
            axis(this.ImageAxes,'tight');
            axis(this.ImageAxes,'image');
            try
                set(this.ImageAxes,'LooseInset',get(this.ImageAxes,'TightInset')); %expand axes to fill figure
            catch
            end
            %Invert Y-axis for normal image display
            this.ImageAxes.YDir = 'reverse';

            
            %Set heights
            this.OuterVBox.Heights = [this.Slider.Horizontal_Height,-1];
            
            %% look for StackData
            found_data = false;
            if ~isempty(varargin)
                
                if iscellstr(varargin{1}) %cell str of file names
                    this.UseImageFiles = true;
                    this.StackFileList = struct('name',varargin{1});
                    this.StackData = cell(numel(this.StackFileList),1);
                    varargin(1) = [];
                    found_data = true;
                elseif isstruct(varargin{1})&&isfield(varargin{1},'name') %struct returned from dir
                    this.UseImagesFiles = true;
                    this.StackFileList = varargin{1};
                    this.StackData = cell(numel(this.StackFileList),1);
                    varargin(1) = [];
                    found_data = true;
                elseif isnumeric(varargin{1}) %numeric array
                    this.StackData = varargin{1};
                    this.varargin(1) = [];
                    found_data = true;
                elseif iscell(varargin{1}) % cell array of images
                    this.StackData = varargin{1};
                    varargin(1) = [];
                    found_data = true;
                elseif ischar(varargin{1}) %string
                    props = extras.getproperties(this,'SetAccess','public');
                    if ~ismember(lower(varargin{1}),lower(props)) %make sure user didn't specifiy a property name
                        [pth,nm,ext] = fileparts(varargin{1});
                        imf = imformats;
                        exts = [imf.ext];

                        if isempty(ext) %only dir specified
                            pth = fullfile(pth,nm);
                            for n = 1:numel(exts)
                                this.StackFileList = [this.StackFileList; dir(fullfile(pth,['*.',exts{n}]))];
                            end
                        else %dir1\dir2\file...ext
                            assert(ismember(ext(2:end),exts),'Image extension must be valid type see imformats()');
                            this.StackFileList = dir(varargin{1});
                        end
                        this.UseImageFiles=true;
                        this.StackData = cell(numel(this.StackFileList),1);

                        varargin(1) = [];
                        found_data = true;
                    end
                end
            end
            %% parse remaining inputs using nave-value pairs
            if numel(varargin)>1
                n=1;
                while n<numel(varargin)-1
                    if ischar(varargin{n}) && strcmpi(varargin{n},'StackData')
                        if found_data
                            error('Stack data was specified more than once');
                        end
                        
                        if iscellstr(varargin{n+1}) %cell str of file names
                            this.UseImageFiles = true;
                            this.StackFileList = struct('name',varargin{n+1});
                        elseif isstruct(varargin{n+1})&&isfield(varargin{n+1},'name') %struct returned from dir
                            this.UseImagesFiles = true;
                            this.StackFileList = varargin{n+1};
                        elseif isnumeric(varargin{n+1}) %numeric array
                            this.StackData = varargin{n+1};
                        elseif iscell(varargin{n+1}) % cell array of images
                            this.StackData = varargin{n+1};
                        elseif ischar(varargin{n+1}) %string
                            [pth,nm,ext] = fileparts(varargin{n+1});
                            imf = imformats;
                            exts = [imf.ext];

                            if isempty(ext) %only dir specified
                                pth = fullfile(pth,nm);
                                for m = 1:numel(exts)
                                    this.StackFileList = [this.StackFileList; dir(fullfile(pth,['*.',exts{m}]))];
                                end
                            else %dir1\dir2\file...ext
                                assert(ismember(ext(2:end),exts),'Image extension must be valid type see imformats()');
                                this.StackFileList = dir(varargin{n+1});
                            end
                            this.StackData = cell(numel(this.StackFileList),1);
                            this.UseImageFiles=true;
                        end
                        found_data = true;
                        
                        varargin(n:n+1) = []; %remove from varargs
                    else %not found, advance
                        n=n+1;
                    end
                end
            end
            persistent LastDir;
            if ~found_data %did not find data, ask for folder?
                custdir = uigetdir(LastDir,'Select Directory containing images');
                if custdir==0
                    return;
                end
                LastDir = custdir;
                imf = imformats;
                exts = [imf.ext];
                for m = 1:numel(exts)
                    this.StackFileList = [this.StackFileList; dir(fullfile(custdir,['*.',exts{m}]))];
                end
                this.StackData = cell(numel(this.StackFileList),1);
                this.UseImageFiles=true;
            end
            
            %% Set other options
            set(this,varargin{:});
        end
        
        function delete(this)
            delete(this.Image);
            delete(this.LevelsUI);
            delete(this.ImageAxes);
            delete(this.OuterVBox);
        end
    end
    
    %% Internal Use
    methods(Access=protected)
        function UpdateImage(this)
            this.Image.CData = this.CurrentImageData;
        end
    end
    
    %% Callbacks
    methods (Hidden)
        function showLevelsUI(this)
            if isempty(this.LevelsUI) || ~isvalid(this.LevelsUI)
                this.LevelsUI = extras.ImageLevelsUI(this.Image);
            end
            lf = ancestor(this.LevelsUI.Parent,'figure');
            figure(lf);
        end
        
        function UIchangeframe(this)
            this.CurrentFrame = this.Slider.Value;
        end
    end
    
    %% Set Methods
    methods
        function set.StackData(this,val)
            assert(iscell(val)||...
                (isnumeric(val)&&(ndims(val)==2 || ndims(val)==3 || ndims(val)==4)),...
                'StackData must be a cell array of images, 3-d numeric array [Y,X,FRAMES], or 4-d numeric array [Y,X,RGB,FRAMES]');
            
            if isnumeric(val)&&ndims(val)==2
                sz = size(val);
                val = reshape(val,[sz,1]);
            end
            
            %% NumOfFrames
            if isempty(val)
                this.NumberOfFrames = 0;
            elseif iscell(val)
                this.NumberOfFrames = numel(val);
            elseif isnumeric(val) && ndims(val)==3
                this.NumberOfFrames = size(val,3);
            elseif isnumeric(val) && ndims(val)==4
                this.NumberOfFrames = size(val,4);
            else
                warning('StackData is not properly initialized, NumberOfFrames cannot be determined');
                this.NumberOfFrames = NaN;
            end
            
            %% Set Data
            this.StackData = val;
            
            %% Current Frame => calls ChangeFrame Functions, updates display
            this.CurrentFrame = max(1,min(this.CurrentFrame,this.NumberOfFrames));
            
            %% Update Display again in case CurentFrame did't actually change
            this.UpdateImage();
            
        end
        
        function set.NumberOfFrames(this,val)
            this.NumberOfFrames = val;
            
            set(this.Slider,'Min',1,'Max',this.NumberOfFrames,...
                'SliderStep',[1/this.NumberOfFrames,max(0.1,10/this.NumberOfFrames)]);
        end
        
        function set.CurrentFrame(this,val)
            old_frame = this.CurrentFrame;
            this.CurrentFrame = max(1,min(round(val),this.NumberOfFrames));
            
            this.Slider.Value = this.CurrentFrame;
            
            this.UpdateImage();
            
            if old_frame ~= this.CurrentFrame
                hgfeval(this.ChangeFrameCallback,this,this.CurrentImageData);
            end
        end
    end
end