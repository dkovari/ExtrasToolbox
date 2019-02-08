classdef ImageViewerND < extras.RequireGuiLayoutToolbox & extras.GraphicsChild
    
    
    %% FrameDimension
    properties(SetObservable,AbortSet)
        FrameIndicies = []; %indicies in stack for the current frame
    end
    properties(SetAccess=private,SetObservable,AbortSet)
        NumStackDimensions = 0; %number of dimensions in the image stack (e.g. stack with TZC: NumDim->3)
        StackDimensions = [0,0]; %dimsinons of the image stack (e.g. stack with TZC: [numT,numZ,numC]);
        StackDimensionNames = {}; %cell array with human-readable dimension names (e.g. {'Time','Z','Channel'});
        HasChannel (1,1) logical = false; %flag specifying if one of the dimensions corresponds to an image channel
        ChannelDimension = []; %dimension in stack that corresponds to channel
        
        
        ImageStack = {}; %cell array which holds image plane data, same dimensions as StackDimensions
    end
    properties(Access=private)
        ImageViewerND_BeingConstructed = true;
        Internal_set_FrameIndicies = false; %flag specifying set_FrameIndicies() is executing
    end
    methods %get/set methods
        function set.FrameIndicies(this,value)
            if this.ImageViewerND_BeingConstructed||this.Internal_set_FrameIndicies
                this.FrameIndicies = value;
                return;
            end
            set_FrameIndicies(this,value);
        end
    end
    methods (Access=private) %defined externally
        set_FrameIndicies(this,value); %internal function for setting FrameIndicies
        changeStackIndex(this,stackDim,newIndex); %change index of specified stack dimension (called by gui controls)
        img = getImagePlane(this,indicies); %returns 2d array with image plane data
    end
    
    %% UI controls
    properties(Access=private)
        StackIndexControls = gobjects(0); %array of uicontrols which change the indicies. Same size a StackDimensions. Dimensions which have only 1 element will have a matlab.graphics.GraphicsPlaceholder
        ChannelControl = matlab.graphics.GraphicsPlaceholder; %uicontrol which changes channel index (if there is a channel index)
        ChannelButtonBox
    end
    
    %% Channel Specific Properties
    properties (Access=private)
        ChannelNames = {}; %cellstr array of human-readable channel names
        ChannelColor = {}; %rgb color for each channel, used for alphamaps
        UseAlphaMap = []; %flag for each channel specifying if it should be used as an alpha map when viewing merged image
        ChannelColorspline = extras.colormapspline.empty(); %colormap spline for each channel
        ChannelCLim = {}; %clim for each channel
        ChannelIndex = 1;
    end
    
    %% Control Panel Containers
    properties(Access=private)
        ControlPanel; %dockable pannel containing controls
        ControlPanelFig = gobjects(0); %figure handle for controls parent, when undocked
        ControlVBox; %vbox container holding controls
        
        ControlsHeight = 24; %height of Controls Panel
        ControlLabelWidth = 75; %default label width for control items
        ControlsDefaultRowHeight = 40; %default row height for controls (in pixels)
    end
    
    %% Axes & Image
    properties(Access=private)
        MainOuterVBox; %container in main figure, holds axes and controls (when docked)
        AxesPanel;%container which holds Axes (and axes peers)
        Axes; %main axes on which images are plotted
        Image; %image handle
    end
    
    %% Internal Stack Related
    properties(Access=private)     
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
            this.MainOuterVBox = uix.VBox('Parent',this.Parent);
            
            %% Image
            
            this.AxesPanel = uix.HBox('Parent',this.MainOuterVBox);
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
                'Parent',this.MainOuterVBox,...
                'DockFcn',@(~,~) this.ControlsDockCallback);
            this.ControlVBox = uix.VBox('Parent',this.ControlPanel);
            
            
            this.MainOuterVBox.Heights(end) = this.ControlsHeight;
            
            %% Done with initial construction
            this.ImageViewerND_BeingConstructed = false;
            
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
                    'Parent',this.MainOuterVBox);
                this.MainOuterVBox.Heights(end) = this.ControlsHeight;
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