classdef ImageViewerND < extras.RequireGuiLayoutToolbox & extras.GraphicsChild
    
    
    %% FrameDimension
    properties(SetObservable,AbortSet)
        FrameIndicies = []; %indicies in stack for the current frame
    end
    properties(SetAccess=private,SetObservable,AbortSet)
        NumStackDimension = 0; %number of dimensions in the image stack (e.g. stack with TZC: NumDim->3)
        StackDimensions = [0,0]; %dimsinons of the image stack (e.g. stack with TZC: [numT,numZ,numC]);
        StackDimensionNames = {}; %cell array with human-readable dimension names (e.g. {'Time','Z','Channel'});
        HasChannel (1,1) logical = false; %flag specifying if one of the dimensions corresponds to an image channel
        ChannelDimension = []; %dimension in stack that corresponds to channel
        
        ImageStack = {};
    end
    properties(Access=private)
        Internal_set_FrameIndicies = false; %flag specifying set_FrameIndicies() is executing
    end
    methods %get/set methods
        function set.FrameIndicies(this,value)
            if this.Internal_setFrameIndicies
                this.FrameIndicies = value;
                return;
            end
            set_FrameIndicies(this,value);
        end
        
    end
    methods (Access=private) %defined externally
        set_FrameIndicies(this,value); %internal function for setting FrameIndicies
        
    end
    
    %% UI controls
    properties(Access=private)
        StackIndexControls = gobjects(0); %array of uicontrols which change the indicies. Same size a StackDimensions. Dimensions which have only 1 element will have a matlab.graphics.GraphicsPlaceholder
        ChannelControl = matlab.graphics.GraphicsPlaceholder; %uicontrol which changes channel index (if there is a channel index)
        
    end
    
    
    
    %% Public Frame Controls
    properties
        TimeIndex (1,1) double = 1;
        ZIndex (1,1) double = 1;
        SeriesIndex (1,1) double= 1;
        ChannelIndex double = 1;
    end
    methods
        function set.TimeIndex(this,value)
            value = round(value);
            value = max(1,min(value,this.nT));
            this.TimeIndex = value;
            
            try
                this.TimeSlider.Value = value;
            catch
            end
            
            this.updateImage();
        end
        function set.ZIndex(this,value)
            value = round(value);
            value = max(1,min(value,this.nZ));
            this.ZIndex = value;
            
            try
                this.ZSlider.Value = value;
            catch
            end
            
            this.updateImage();
        end
        function set.SeriesIndex(this,value)
            value = round(value);
            value = max(1,min(value,this.nSeries));
            this.SeriesIndex = value;
            
            try
                this.SeriesSlider.Value = value;
            catch
            end
            
            this.updateImage();
        end
        function set.ChannelIndex(this,value)
            if isempty(value)
                value = 1;
            end
            value = reshape(value,1,[]);
            value = round(value);
            if any(~ismember(value,1:this.nC))
                error('invalid channels');
            end
            value = unique(value);
            
            this.ChannelIndex = value;
            
            this.updateChannels()
        end
    end
    
    %% Control Panel Containers
    properties(Access=private)
        ControlPanel
        ControlPanelFig = gobjects(0);
        OuterVBox
        ControlVBox
        AxesPanel
        Axes
        Image
        
        ControlsHeight = 50;
        
        ControlLabelWidth = 75;
      
    end
    
    %% Internal Stack Related
    properties(Access=private)
        nT = 1; %number of time points
        nC = 1; %number of chanels
        nZ = 1; %number of z-steps
        nSeries = 1; %number of image series
        
        ChannelName = {}; %char specifying name of channe;
        ChannelColor = {}; %rgb color for each channel, used for alphamaps
        ChannelColorspline = extras.colormapspline.empty(); %colormap spline for each channel
        ChannelCLim = {}; %clim for each channel
        UseAlphaMap = []; %flag for each channel specifying if it should be used as an alpha map when viewing merged image
        
        imageCell = cell(0,0,0,0); %T,Z,S,C
        
        UsingBF = false;
        bfreader
        bfmeta
        
        
    end
    
    %% Create figure method
    methods(Access=protected,Static)
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
            %Insert Colorbar
            delete(findall(hFig,'ToolTipString','Insert Colorbar'));
            
            %% custom colorbar
            %extras.addColorbarButton(hFig);
                        
        end
    end
    
    %% constructor/destructor
    methods
        function this = ImageViewerND(varargin)
        % Accepts standard Graphics object parent specifiers
        %   ImageViewerND(parent,...)
        %   ImageViewerND(...,'Parent',parent,...)
        
            %% setup parent
            %initiate graphics parent related variables
            this@extras.GraphicsChild(@() extras.ImageViewerND.createFig());
            
            %look for parent specified in arguments
            varargin = this.CheckParentInput(varargin{:});
            
            %% Create outer container
            this.OuterVBox = uix.VBox('Parent',this.Parent);
            
            %% Image
            
            this.AxesPanel = uix.HBox('Parent',this.OuterVBox);
            this.Axes = axes(this.AxesPanel,'NextPlot','ReplaceChildren','HandleVisibility','Callback');
            set(this.ParentFigure,'CurrentAxes',this.Axes);
            
            
            this.Image = imagesc('parent',this.Axes,'CData',[0,0;0,0],...
                'HandleVisibility','Callback',...
                'SelectionHighlight','off',...
                'PickableParts','none');
            axis(this.Axes,'image');
            
            extras.expandAxes(this.Axes);
            
            %% controls
            this.ControlPanel = uix.BoxPanel('Title','Frame Controls',...
                'Parent',this.OuterVBox,...
                'DockFcn',@(~,~) this.ControlsDockCallback);
            this.ControlVBox = uix.VBox('Parent',this.ControlPanel);
            
            
            this.OuterVBox.Heights(end) = this.ControlsHeight;
            
            %%
            this.loadUsingBFReader();
        end
        
        function delete(this)
            try
                delete(this.ControlPanelFig)
            catch
            end
        end
    end
 
    %% internal
    methods(Access=private)
        function ControlsDockCallback(this)
            if this.ControlPanel.Docked %was docked, need to undock
                pos = getpixelposition(this.ControlPanel);
                this.ControlPanelFig = figure(...
                    'Name',this.ControlPanel.Title,...
                    'NumberTitle','off',...
                    'MenuBar','none',...
                    'Toolbar','none',...
                    'CloseRequestFcn',@(~,~) this.ControlsDockCallback);
                this.ControlPanelFig.Position = [this.ParentFigure.Position(1:2),pos(3:4)];
                
                set(this.ControlPanel,...
                    'Docked',false,...
                    'Parent',this.ControlPanelFig,...
                    'Units','Normalized',...
                    'Position',[0,0,1,1]);
            else %put back
                set(this.ControlPanel,...
                    'Docked',true,...
                    'Parent',this.OuterVBox);
                this.OuterVBox.Heights(end) = this.ControlsHeight;
                delete(this.ControlPanelFig);
            end
        end
    end
    
    %% Defined in other files
    methods(Access=private)
        loadUsingBFReader(filepath)
        rebuildFrameControls(this)
        updateColormapFromChannelColors(this)
        displayChannelOverlay(this)
        toggleChannelDisplay(this,ChanIdx)
        updateImage(this)
    end
    
end