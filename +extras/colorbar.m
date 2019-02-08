classdef colorbar < extras.RequireGuiLayoutToolbox & handle & extras.widgets.mixin.AssignNV
    
    %% contstructor/delete
    methods
        function this = colorbar(varargin)
            
            this.Internal_AxesChange = false;
            if nargin>0 && isgraphics(varargin{1})
                this.Axes = varargin{1};
                varargin(1) = [];
            end
            
            this.setPublicProperties(varargin{:});
        end
        function delete(this)
            this.releaseAxes();
        end
    end
    
    %% dependents
    properties(Dependent)
        Label
        Title
        ColorbarAxes
    end
    methods
        function lb = get.Label(this)
            if ~this.HandlesValid
                lb = gobjects(0);
            else
                lb = this.CmapAxes.XLabel;
            end
        end
        function lb = get.Title(this)
            if ~this.HandlesValid
                lb = gobjects(0);
            else
                lb = this.CmapAxes.Title;
            end
        end
        function hax = get.ColorbarAxes(this)
            if ~this.HandlesValid
                hax = gobjects(0);
            else
                hax = this.CmapAxes;
            end
        end
    end
    
    %% colorbar props
    properties (SetObservable,AbortSet)
        Location = 'right';
        Axes = gobjects(0);
        Width (1,1) double = 75;
        Height (1,1) double = 75;
    end
    methods
        function set.Location(this,value)
            value = validatestring(value,{'right','left','top','bottom'});
            this.Location = value;
            this.updateLocation();
        end
        function set.Axes(this,hax)
            
            if this.Internal_AxesChange
                this.Axes = hax;
                return;
            end

            assert(isgraphics(hax)&&strcmpi(hax.Type,'axes'),'Axes must be valid graphics object of type ''axes''');
            
            %% release axes
            this.releaseAxes();
            
            %% Move axes into uix.Grid
            this.OrigAxesUnits = hax.Units;
            AxesParent = hax.Parent;
            %special case for uix.Container
            pos = hax.OuterPosition;
            if isa(AxesParent,'uix.Container')
                this.Grid = uix.Grid('Parent',AxesParent,'Padding',5);
                set(hax,'Parent',this.Grid,'Units','normalized','OuterPosition',[0,0,1,1]);
%                 if ~isa(AxesParent,'uix.Box')
%                     'not box'
%                     set(this.Grid,'Units',this.OrigAxesUnits,'Position',pos)
%                 end
            else
                this.Grid = uix.Grid('Parent',AxesParent,'Units',this.OrigAxesUnits,'Position',pos,'Padding',5);
                set(hax,'Parent',this.Grid,'Units','normalized','OuterPosition',[0,0,1,1]);
            end
            
            this.Axes = hax;
            
            %% Add listeners
            this.ColormapListener = addlistener(hax,'Colormap','PostSet',@(~,~) this.updateCBarImage);
            this.ColorscaleListener = addlistener(hax,'ColorScale','PostSet',@(~,~) this.updateCBarImage);
            this.CLimListener = addlistener(hax,'CLim','PostSet',@(~,~) this.updateCBarImage);
            
            %% create
            this.createCBar();
            this.updateCBarImage();
            this.updateLocation();
            
        end
        
        function set.Width(this,value)
            this.Width = value;
            this.updateLocation();
        end
        function set.Height(this,value)
            this.Height = value;
            this.updateLocation();
        end
        
    end
    
    properties(Access=private)
        ColormapListener
        ColorscaleListener
        CLimListener
        
        Grid
        OrigAxesUnits
        
        CmapAxes
        CmapImage
        ContextMenu
        LocationLeftMenu
        LocationRightMenu
        LocationTopMenu
        LocationBottomMenu
        
        Internal_AxesChange = true;
    end
    properties(Dependent,Access=protected)
        HandlesValid
    end
    methods
        function tf = get.HandlesValid(this)
            tf = ~isempty(this.Axes)&&isgraphics(this.Axes)&&...
                ~isempty(this.CmapAxes)&&isvalid(this.CmapAxes);
        end
    end
    
    methods (Access=private)
        
        function deleteListeners(this)
            delete(this.ColormapListener)
            delete(this.ColorscaleListener);
            delete(this.CLimListener);
        end
        
        function releaseAxes(this)
            
            %% delete existing cbar
            try
            delete(this.CmapAxes);
            catch
            end
            try
                delete(this.ContextMenu)
            catch
            end
            %% relase old axes, put everything back to normal
            if ~isempty(this.Axes)&&isvalid(this.Axes)
                pos = this.Grid.Position;
                set(this.Axes,'Parent',this.Grid.Parent,...
                    'Units',this.OrigAxesUnits,...
                    'OuterPosition',pos);
                
                this.Internal_AxesChange = true;
                this.Axes = gobjects(0);
                this.Internal_AxesChange = false;
                
                this.deleteListeners();
                
                delete(this.Grid);

            end
        end
        
        function createCBar(this)
            if ~isempty(this.CmapAxes)&&isgraphics(this.CmapAxes)
                return;
            end
            
            this.CmapAxes = axes(this.Grid,'HandleVisibility','callback','NextPlot','replacechildren');
            this.CmapAxes.YAxis.Visible = 'off';
            %axis(this.CmapAxes,'tight');
            
            this.CmapImage = image(this.CmapAxes,reshape([0,0,0],1,1,3));
            set(this.CmapImage,...
                'PickableParts','none',...
                'HitTest','off',...
                'HandleVisibility','callback');
            
            axis(this.CmapAxes,'tight');
            
            %% context menu
            this.ContextMenu = uicontextmenu(ancestor(this.CmapAxes,'figure'));
            m = uimenu('Parent',this.ContextMenu,'Text','Location');
            this.LocationLeftMenu = uimenu(m,'Text','Left','Checked','off','Callback',@(~,~) set(this,'Location','left'));
            this.LocationRightMenu = uimenu(m,'Text','Right','Checked','on','Callback',@(~,~) set(this,'Location','right'));
            this.LocationTopMenu = uimenu(m,'Text','Top','Checked','off','Callback',@(~,~) set(this,'Location','top'));
            this.LocationBottomMenu = uimenu(m,'Text','Bottom','Checked','off','Callback',@(~,~) set(this,'Location','bottom'));
            
            this.CmapAxes.UIContextMenu = this.ContextMenu;
        end
        
        function updateCBarImage(this)
            cmap = colormap(this.Axes);
            cmap = permute(cmap,[3,1,2]);
            set(this.CmapImage,'CData',cmap,...
                'XData',this.CmapAxes.CLim);
            set(this.CmapAxes,'XScale',this.CmapAxes.ColorScale);
        end
        
        function updateLocation(this)
            if ~this.HandlesValid
                return;
            end
            switch(this.Location)
                case 'right'
                    view(this.CmapAxes,[-90,90]);
                    this.CmapAxes.XAxisLocation = 'top';
                    
                    set(this.Grid,...
                        'Heights',-1,...
                        'Widths',[-1,this.Width],...
                        'Contents',[this.Axes,this.CmapAxes]);
                    
                    this.LocationLeftMenu.Checked = 'off';
                    this.LocationRightMenu.Checked = 'on';
                    this.LocationTopMenu.Checked = 'off';
                    this.LocationBottomMenu.Checked = 'off';
                    
                case 'left'
                    view(this.CmapAxes,[-90,90]);
                    this.CmapAxes.XAxisLocation = 'bottom';
                    
                    set(this.Grid,...
                        'Heights',-1,...
                        'Widths',[this.Width,-1],...
                        'Contents',[this.CmapAxes,this.Axes]);
                    
                    this.LocationLeftMenu.Checked = 'on';
                    this.LocationRightMenu.Checked = 'off';
                    this.LocationTopMenu.Checked = 'off';
                    this.LocationBottomMenu.Checked = 'off';
                case 'top'
                    view(this.CmapAxes,[0,90]);
                    this.CmapAxes.XAxisLocation = 'top';
                    set(this.Grid,...
                        'Widths',-1,...
                        'Heights',[this.Height,-1],...
                        'Contents',[this.CmapAxes,this.Axes]);
                    
                    this.LocationLeftMenu.Checked = 'off';
                    this.LocationRightMenu.Checked = 'off';
                    this.LocationTopMenu.Checked = 'on';
                    this.LocationBottomMenu.Checked = 'off';
                case 'bottom'
                    view(this.CmapAxes,[0,90]);
                    this.CmapAxes.XAxisLocation = 'bottom';
                    set(this.Grid,...
                        'Widths',-1,...
                        'Heights',[-1,this.Height],...
                        'Contents',[this.Axes,this.CmapAxes]);
                    
                    this.LocationLeftMenu.Checked = 'off';
                    this.LocationRightMenu.Checked = 'off';
                    this.LocationTopMenu.Checked = 'off';
                    this.LocationBottomMenu.Checked = 'on';
            end
        end
    end
end