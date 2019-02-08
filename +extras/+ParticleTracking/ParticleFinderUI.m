classdef ParticleFinderUI < extras.RequireGuiLayoutToolbox & ...
                            handle    
    %% Image Processing Controls
    properties
        Prefilter = 'None';
        bpass_lnoise = 1;
        bpass_sz = 7;
        
        pkfnd_sz  =7;
        
        pkfnd_th = 'auto'
    end
    %% Internal use
    properties(SetAccess=private)
        Viewer
    end
    properties (Access=private)
        ControlsFigure
        
        %controls
        OuterScrollBox
        ControlsVBox
        
        hPrefilter
        hbpass_lnoise
        hbpass_sz
        hpkfnd_sz;
        hpkfnd_th
        hpkfnd_th_button
    end
                        
    %% constructor
    methods
        function this = ParticleFinderUI(varargin)
        % ParticleFinderUI() - Create UI for determing particle tracking
        % parameters
        % Syntax:
        %   ParticleFinderUI(stack_cell)
        %   ParticleFinderUI(stack_cell,Name,Value)
       
            
            %% Create viewer for stack
            this.Viewer = extras.stackviewer();
            addlistener(this.Viewer,'ObjectBeingDestroyed',@(~,~) delete(this));
            
            % create callback for frame change
            addlistener(this.Viewer,'CurrentFrame','PostSet',@(~,~) this.FrameChanged);
            
            
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
                'Callback',@(h,~) this.ValidNumberControl(h,'pkfnd_sz'),...
                'LabelTooltipString',sprintf('Threshold used to identify approximate particle center.\n"auto" uses 50th percentile.'));
            
            this.hpkfnd_th_button = uicontrol('Parent',hb,...
                'TooltipString','Select using histogram',...
                'String','HISTO.',...
                'Callback',@(~,~) this.pkfnd_th_histogram());
                
            hb.Widths(end) = 30;
            
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
    
    %% internal methods
    methods(Access=private)
        
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
        end
    end
end