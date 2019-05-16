classdef lutViewer < extras.GraphicsChild & extras.RequireGuiLayoutToolbox & extras.widgets.mixin.ObjectDependentLifetime
% UI viewer for displaying spline LUT data
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.
    
    %%
    properties (Access=private)
        LUT = extras.roi.LUTobject.empty();
    end
    
    %% Graphics handles
    properties(Access=private)
        hImg_Spline %image showing spline data
        hLn_R_Profile %draggable line for R profile
        hLn_Z_Profile %draggable line for z profile
        hLn_R_Data %data line for r profile
        hLn_Z_Data %z profile data line
    end
    
    %% Listeners
    properties(Access=private)
        LUT_MinR_Listener
        LUT_MaxR_Listener
        LUT_pp_Listener
        hAx_R_Profile
        hAx_Spline
        hAx_Z_Profile
    end
    
    %% constructor
    methods
        function this = lutViewer(varargin)
            
            %% Parse Inputs
            found_parent = false;
            found_lut = false;
            Parent = gobjects(0);
                LUT = extras.roi.LUTobject.empty();
            if nargin>0
                if isgraphics(varargin{1})
                    found_parent = true;
                    Parent = varargin{1};
                    varargin(1) = [];
                end
                if isa(varargin{1},'extras.roi.LUTobject')
                    found_lut = true;
                    LUT = varargin{1};
                    varargin(1) = [];
                end
            end
                
            
            
            p = inputParser;
            p.addParameter('Parent',gobjects(0),@isgraphics);
            p.addParameter('LUT',extras.roi.LUTobject.empty(),@(x) isa(x,'extras.roi.LUTobject'));
            
            p.parse(varargin{:});
            
            if found_parent && ~ismember('Parent',p.UsingDefaults)
                error('Parent specified twice.')
            end
            
            if found_lut && ~ismember('LUT',p.UsingDefaults)
                error('LUT specified twice.')
            end
            
            Parent = [Parent,p.Results.Parent];
            LUT = [LUT,p.Results.LUT];
            
            %% initial setup
            this@extras.widgets.mixin.ObjectDependentLifetime(LUT,'ConstructMultiple');
            this@extras.GraphicsChild(@() ...
                figure(...
                    'Name','LUT Viewer',...
                    'NumberTitle','off',...
                    'HandleVisibility','Callback')...
                    );
            
            %% if LUT Empty return empty handle array
            if isempty(LUT)
                delete(this);
                this = extras.roi.lutViewer.empty();
                return;
            end
            
            this = reshape(this,size(LUT));
            
            for n=1:numel(this)
                this(n).LUT = LUT(n);
            end
            
            %% construct parent fig(s)
            assert(isempty(Parent)||numel(LUT)==numel(Parent),'Parent must be unspecified or same number as LUT');
            for n=1:numel(LUT)
                if isempty(Parent)
                    this(n).CheckParentInput();
                else
                    this(n).CheckParentInput(Parent(n));
                end
            end
            
            %% Look for Previously constructed views
            persistent prev_construct_views;
            if isempty(prev_construct_views)
                prev_construct_views = extras.roi.lutViewer.empty();
            end
            prev_construct_views(~isvalid(prev_construct_views)) = [];
            
            [lia,lob] = ismember(LUT,[prev_construct_views.LUT]);
            need_construction = ~lia;
            
            % if already constructed, delete and set this(...) to the
            % previously constructed viewer
            delete(this(lia));
            this(lia) = prev_construct_views(lob(lia));

            
            %% Construct GUI for each viewer
            for n=1:numel(this)
                if ~need_construction(n)
                    continue;
                end
                this(n).constructGui();
            end
            
            
            %% Add newly constructed views to the list
            prev_construct_views = [prev_construct_views,this(need_construction)];
        end
    end
    
    %% Internal
    methods(Access=private)
        function constructGui(this)
            %% Change Figure Title
            if this.CreatedParent
                this.ParentFigure.Name = sprintf('LUT for ROI: %s',this.LUT.UUID);
            end

            %% Create Subplots
            flex = uix.GridFlex('Parent',this.Parent,'Spacing', 5);

            this.hAx_R_Profile = axes('Parent',flex,...
                'HandleVisibility','Callback',...
                'NextPlot','add');
            extras.expandAxes(this.hAx_R_Profile);
            ylabel(this.hAx_R_Profile,'Intensity [A.U.]');
            
            this.hAx_Spline = axes('Parent',flex,...
                'HandleVisibility','Callback',...
                'NextPlot','add');
            extras.expandAxes(this.hAx_Spline);
            ylabel(this.hAx_Spline,['Z Position [',this.LUT.Z_Units,']']);
            xlabel(this.hAx_Spline,['R Position [',this.LUT.R_Units,']']);
            
            uix.Empty( 'Parent', flex );
            this.hAx_Z_Profile = axes('Parent',flex,...
                'HandleVisibility','Callback',...
                'NextPlot','add');
            extras.expandAxes(this.hAx_Z_Profile);
            xlabel(this.hAx_Z_Profile,'Intensity [A.U.]');

            set(flex,'Widths',[-3,-1],'Heights',[-1,-4]);

            % Spline Image
            this.hImg_Spline = imagesc(this.hAx_Spline,'XData',[0,1],'YData',[0,1],'CData',[0,0;0,0]);
            colormap(this.hAx_Spline,'gray');

            this.hLn_R_Profile = extras.DraggableLine(this.hAx_Spline,[NaN,NaN],[0,0],'Color',[0 0.447 0.741]); %r line
            addlistener(this.hLn_R_Profile,'Y','PostSet',@(~,~) this.updateRprofile());

            this.hLn_Z_Profile = extras.DraggableLine(this.hAx_Spline,[0,0],[NaN,NaN],'Color',[0.85 0.325 0.098]);
            addlistener(this.hLn_Z_Profile,'X','PostSet',@(~,~) this.updateZprofile());

            this.hLn_R_Data = plot(this.hAx_R_Profile,[NaN,NaN],[NaN,NaN],'-','LineWidth',1,'Color',this.hLn_R_Profile.Color);
            this.hLn_Z_Data = plot(this.hAx_Z_Profile,[NaN,NaN],[NaN,NaN],'-','LineWidth',1,'Color',this.hLn_Z_Profile.Color);
            
            %% listeners for changes to lut
            this.LUT_MinR_Listener = addlistener(this.LUT,'MinR','PostSet',@(~,~) this.MinMaxRChanged);
            this.LUT_MaxR_Listener = addlistener(this.LUT,'MaxR','PostSet',@(~,~) this.MinMaxRChanged);
            this.LUT_pp_Listener = addlistener(this.LUT,'pp','PostSet',@(~,~) this.ppChanged);
            
            %% force redraw
            this.ppChanged();
        end
    end
    
    
    %% destructor
    methods
        function delete(this)
            delete([this.LUT_MinR_Listener]);
            delete([this.LUT_MaxR_Listener]);
            delete([this.LUT_pp_Listener]);
        end
    end
    
    %% Graphics Callbacks
    methods(Access=private)
        function updateRprofile(this)
            if ~this.LUT.IsCalibrated
                this.hImg_Spline.CData = NaN;
                return;
            end
            
            ZZ  = this.hLn_R_Profile.Y(1);
            
            vals = ppval(this.LUT.pp,ZZ)';
            
            set(this.hLn_R_Data,'XData',this.LUT.rr,'YData',vals);
        end
        
        function updateZprofile(this)
            if ~this.LUT.IsCalibrated
                this.hImg_Spline.CData = NaN;
                return;
            end
            
            RR = this.hLn_Z_Profile.X(1);
            [~,r_ind] = min(abs(RR-this.LUT.rr));
            
            zz = linspace(this.LUT.zlim(1),this.LUT.zlim(2),500);
            vals = ppval(this.LUT.pp,zz)';
            vals = vals(:,r_ind);
            
            set(this.hLn_Z_Data,'XData',vals,'YData',zz);
            
            
            
        end
        
        function ppChanged(this)
            if ~this.LUT.IsCalibrated
                this.hImg_Spline.CData = NaN;
                return;
            end
            
            zz = linspace(this.LUT.zlim(1),this.LUT.zlim(2),500);
            vals = ppval(this.LUT.pp,zz)';
            this.hImg_Spline.CData = vals;
            set(this.hAx_Spline,'YLim',this.LUT.zlim);
            
            this.hImg_Spline.YData = this.LUT.zlim;
            this.hLn_R_Profile.DragLimitY = this.LUT.zlim;
            this.hLn_R_Profile.Y = repmat(max(this.LUT.zlim(1),min(this.LUT.zlim(2),this.hLn_R_Profile.Y(1))),1,2);
            
            this.MinMaxRChanged();

        end
        
        function MinMaxRChanged(this)
            if isempty(this.LUT.MinR)||~isfinite(this.LUT.MinR)||isempty(this.LUT.MaxR)||~isfinite(this.LUT.MaxR)
                return;
            end
            this.hAx_R_Profile.XLim = [this.LUT.MinR,this.LUT.MaxR];
            this.hAx_Spline.XLim = [this.LUT.MinR,this.LUT.MaxR];
            
            this.hImg_Spline.XData = [this.LUT.MinR,this.LUT.MaxR];
            
            
            %set dragg limits
            LIM = sort([this.LUT.MinR,this.LUT.MaxR]);
            this.hLn_Z_Profile.DragLimitX = LIM;
            RR = this.hLn_Z_Profile.X(1);
            RR = max(LIM(1),min(LIM(2),RR));
            this.hLn_Z_Profile.X = [RR,RR];
            
            set(this.hAx_Spline,'XLim',LIM);
            
            this.updateRprofile();
            this.updateZprofile();
        end
    end
    
end