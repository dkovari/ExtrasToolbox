classdef ParticleFinderUI < extras.RequireGuiLayoutToolbox & ...
                            handle & matlab.mixin.SetGet
    %% Image Processing Controls
    properties
        Prefilter = 'none';
        bpass_lnoise = 1;
        bpass_sz = 7;
        
        pkfnd_sz  =7;
        
        pkfnd_th = '50%'
        
        UseRidgeFilter = false
        CentroidMethod = 'cntrd'
        CentroidWindowSize = NaN;
        
    end
    %% Internal use
    properties(SetAccess=private)
        Viewer
    end
    properties (Access=private)
        OriginalStack
        ControlsFigure
        
        hPlot
        
        %controls
        OuterScrollBox
        ControlsVBox
        
        hPrefilter
        hbpass_lnoise
        hbpass_sz
        hpkfnd_sz;
        hpkfnd_th
        hpkfnd_th_button
        
        hUseRidgeFilter
        hCentroidMethod
        hCentroidWindowSize
    end
                        
    %% constructor
    methods
        function this = ParticleFinderUI(ImageStack,varargin)
        % ParticleFinderUI() - Create UI for determing particle tracking
        % parameters
        % Syntax:
        %   ParticleFinderUI(stack_cell)
        %   ParticleFinderUI(stack_cell,Name,Value)
       
            
            %% Create viewer for stack
            this.OriginalStack = ImageStack;
            this.Viewer = extras.stackviewer(ImageStack);
            %addlistener(this.Viewer,'ObjectBeingDestroyed',@(~,~) delete(this));
            
            % create callback for frame change
            addlistener(this.Viewer,'CurrentFrame','PostSet',@(~,~) this.FrameChanged);
            
            this.hPlot = plot(this.Viewer.ImageAxes,NaN,NaN,'+r');
            
            %% Create Controls Figure
            this.ControlsFigure = figure('MenuBar','none',...
                'Name','Particle Finder Controls',...
                'NumberTitle','off',...
                'CloseRequestFcn',@(~,~) warning(sprintf('Cannot close Partcile Finder Controls.\n\tClose main viewer instead.')),...
                'HandleVisibility','callback');
            
            
            
            %scroll panel
            this.OuterScrollBox = uix.ScrollingPanel('Parent',this.ControlsFigure);
            
            %Vbox to hold organize ui elements
            this.ControlsVBox = uix.VBox('Parent',this.OuterScrollBox,'Spacing',2);
            
            %% Size constants
            RowHeight = 30; %px
            MinWidth = 220;
            LabelWidth = 90;
            
            %% Controls
            
            % Image pre-filter
            this.hPrefilter = uiw.widget.Popup(...
                'Parent',this.ControlsVBox,...
                'Items',{'None','mean-shift abs','mean-shift'},...
                'Label','Prefilter:', ...
                'LabelLocation','left',...
                'LabelWidth',LabelWidth,...
                'Callback',@(h,~) set(this,'Prefilter',h.Value),...
                'LabelTooltipString','Prefilter used for rough identification of particles');
            
            this.ControlsVBox.Heights(end) = RowHeight;
            
            %% bpass
            
            this.hbpass_lnoise = uiw.widget.EditableText(...
                'Parent',this.ControlsVBox,...
                'Value',num2str(this.bpass_lnoise),...
                'Label','bpass_lnoise',...
                'LabelWidth',LabelWidth,...
                'Callback',@(h,~) this.ValidNumberControl(h,'bpass_lnoise'),...
                'LabelTooltipString',sprintf('Size of noise removal filter [px].\n(1-2 is a good choice.)'));
            
            this.ControlsVBox.Heights(end) = RowHeight;
            
            this.hbpass_sz = uiw.widget.EditableText(...
                'Parent',this.ControlsVBox,...
                'Value',num2str(this.bpass_sz),...
                'Label','bpass_sz',...
                'LabelWidth',LabelWidth,...
                'Callback',@(h,~) this.ValidNumberControl(h,'bpass_sz'),...
                'LabelTooltipString',sprintf('Appoximate particel size [px].\nUsed to partially filter small particles from the image.'));
            
            this.ControlsVBox.Heights(end) = RowHeight;
            
            %% pkfnd
            this.hpkfnd_sz = uiw.widget.EditableText(...
                'Parent',this.ControlsVBox,...
                'Value',num2str(this.pkfnd_sz),...
                'Label','pkfnd_sz',...
                'LabelWidth',LabelWidth,...
                'Callback',@(h,~) this.ValidNumberControl(h,'pkfnd_sz'),...
                'LabelTooltipString',sprintf('Appoximate particel size [px].\nUsed to identify approximate particle centers.'));
            
            this.ControlsVBox.Heights(end) = RowHeight;
            
            hb = uix.HBox('Parent',this.ControlsVBox);
            this.hpkfnd_th = uiw.widget.EditableText(...
                'Parent',hb,...
                'Value',num2str(this.pkfnd_th),...
                'Label','pkfnd_th',...
                'LabelWidth',LabelWidth,...
                'Callback',@(h,~) this.SetPkfnd_th(h.Value),...
                'LabelTooltipString',sprintf('Threshold used to identify approximate particle center.\n"50%" uses 50th percentile.\n"2s" uses mean+2*StdDev'));
            
            this.hpkfnd_th_button = uicontrol('Parent',hb,...
                'TooltipString','Select using histogram',...
                'String','HISTO.',...
                'Callback',@(~,~) this.pkfnd_th_histogram());
                
            hb.Widths(end) = 30;
            
            this.ControlsVBox.Heights(end) = RowHeight;
            
            %% UseRidgeFilter
            this.hUseRidgeFilter = extras.widgets.ValueControl('Parent',this.ControlsVBox,...
                'Value',this.UseRidgeFilter,...
                'Label','UseRidgeFilter',...
                'LabelWidth',LabelWidth,...
                'ValueType','boolean',...
                'Callback',@(h,~) set(this,'UseRidgeFilter',h.Value),...
                'LabelTooltipString',sprintf('Threshold used to identify approximate particle center.\n"auto" uses 50th percentile.'));
            
            this.ControlsVBox.Heights(end) = RowHeight;
            
            %% CentroidMethod
            this.hCentroidMethod = extras.widgets.ValueControl('Parent',this.ControlsVBox,...
                'Value',this.CentroidMethod,...
                'Label','UseRidgeFilter',...
                'LabelWidth',LabelWidth,...
                'AllowedValues',{'cntrd'},...
                'ValueType','string',...
                'Callback',@(h,~) set(this,'CentroidMethod',h.Value),...
                'LabelTooltipString',sprintf('Algorithm used to calculate centroids'));
            
            this.ControlsVBox.Heights(end) = RowHeight;
            
            %% CentroidWindowSize
            this.hCentroidWindowSize = extras.widgets.ValueControl('Parent',this.ControlsVBox,...
                'Value',this.CentroidWindowSize,...
                'Label','CentroidWindowSize',...
                'LabelWidth',LabelWidth,...
                'ValueType','float',...
                'Callback',@(h,~) set(this,'CentroidWindowSize',h.Value),...
                'LabelTooltipString',sprintf('Size of window around estimated particle center used to determing sub-pixel centroid.\nNaN defaults to bpass_sz+2'));
            this.ControlsVBox.Heights(end) = RowHeight;
                        
            %% Set size of outerVBox and scrollbox
            VHeight = sum(this.ControlsVBox.Heights)+ 2*(numel(this.ControlsVBox.Heights)-1);
            %this.outerVBox.Heights = [VHeight+20];%,RoiHeights];
            this.OuterScrollBox.MinimumWidths = MinWidth;
            this.OuterScrollBox.MinimumHeights = VHeight+10;

            % change the size of the controls figure
            orig_units =this.ControlsFigure.Units;
            this.ControlsFigure.Units = 'pixels';
            pos = this.ControlsFigure.Position;
            pos(3) = MinWidth+40;
            pos(4) = min(500,this.OuterScrollBox.MinimumHeights+10);
            this.ControlsFigure.Position = pos;
            this.ControlsFigure.Units = orig_units;
            
            
            %% set args
            set(this,varargin{:});
            
        end
    end
    
    %% detructor
    methods
        function delete(this)
            try
                delete(this.ControlsFigure);
            catch
            end
            try
                delete(this.Viewer)
            catch
            end
        end
    end
    
    %% Public
    methods
        function outStruct = getParameters(this)
            outStruct.Prefilter = this.Prefilter;
            outStruct.bpass_lnoise = this.bpass_lnoise;
            outStruct.bpass_sz = this.bpass_sz;

            outStruct.pkfnd_sz  =this.pkfnd_sz;

            outStruct.pkfnd_th = this.pkfnd_th;

            outStruct.UseRidgeFilter = this.UseRidgeFilter;
            outStruct.CentroidMethod = this.CentroidMethod;
            outStruct.CentroidWindowSize = this.CentroidWindowSize;
        end
    end
    
    
    %% internal methods
    methods(Access=private)
        
        function SetPkfnd_th(this,value)
            [n,tf] = str2num(value);
            if tf
                this.pkfnd_th = n;
            else
                this.pkfnd_th = value;
            end
        end
        
        function ValidNumberControl(this,ctrl_handle,ctrl_name)
            [num_value,tf] = str2num(ctrl_handle.Value);
            if ~tf || ~isscalar(num_value)
                ctrl_handle.TextIsValid = false;
                return;
            end
            try
                set(this,ctrl_name,num_value);
            catch ME
                ctrl_handle.TextIsValid = false;
                errordlg(ctrl_name,ME.getReport);
            end
            ctrl_handle.TextIsValid = true;
            
        end
        function FrameChanged(this)
            if ~isvalid(this)
                return;
            end
            if isempty(this.Viewer)||~isvalid(this.Viewer)
                return;
            end
            
            %% retrack frame
            TA = this.getParameters;
            TA.SkipTracking = true;
            trackStruct = extras.matPTV.ptv2D({this.OriginalStack{this.Viewer.CurrentFrame}},TA);
            set(this.hPlot,'XData',trackStruct.Centroids{1}(:,1),...
                'YData',trackStruct.Centroids{1}(:,2));

        end
    end
end