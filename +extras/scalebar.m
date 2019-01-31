classdef scalebar < extras.GraphicsChild
    
    properties (SetAccess=protected,SetObservable=true,AbortSet=true)
        Line
        Text
    end
    properties(Access=protected)
        PositionListenerX
        PositionListenerY
        DragEnabeledListener
        AxesOrderListener;
    end
    
    properties(SetObservable=true,AbortSet=true)
        Scale = 1; % [scalebar Units]/[axes Units]
        Unit = 'px' %string specifying Units (e.g. µm, nm, px, etc.)
        Length = 50; %Scalebar Length in [scalebar Units]
        Orientation = 'horizontal';
        XY = [NaN,NaN]; %Location of the center of the scalebar, in axes Units
        Location = 'southeast';
        DragEnabled = true;
        Visible = true;
        KeepOnTop = true;
    end
    properties(Access=protected)
        BeingConstructed = true;%flag indicating class is being constructed
        
        changeXYfromPCL = false; %flag if internal change to XY
        changeXYfromLocation = false;
        ContextMenu
    end
    properties(Dependent)
        Color;
        Label
    end
    methods
        function set.KeepOnTop(this,val)
            assert(isscalar(val),'KeepOnTop must be scalar and convertable to logical');
            this.KeepOnTop = logical(val);
            if this.KeepOnTop
                this.bringToFront();
            end
        end
        function set.Visible(this,val)
            this.Line.Visible = val;
            this.Text.Visible = val;
            this.Visible = val;
        end
        function set.Unit(this,val)
            assert(ischar(val),'Unit must be char array');
            this.Unit = val;
            this.Text.String = this.Label;
        end
        function set.Length(this,val)
            assert(isscalar(val)&&isnumeric(val)&&val>0,'Length must be numeric scalar >0');
            this.Length = val;
            %% update label
            this.Text.String = this.Label;
            %% update line
            this.updateLinePosition;
            %% update text position
            this.updateTextPosition;
        end
        function set.Scale(this,val)
            assert(isscalar(val)&&isnumeric(val)&&val>0,'Scale must be numeric scalar >0');
            this.Scale = val;
            %% update line
            this.updateLinePosition;
            %% update text position
            this.updateTextPosition;
        end
        function set.Orientation(this,val)
            if this.BeingConstructed
                this.Orientation = val;
                return;
            end
            assert(ischar(val),'Orientation must be valid char array');
            val = lower(val);
            assert(ismember(val,{'vertical','horizontal'}),'Orientation must be ''vertical'' or ''horizontal''');
            this.Orientation = val;
            
            this.updateLinePosition;
            this.updateTextPosition;
            
            if ~this.changeXYfromLocation
                this.Location = 'manual';
            end
            
        end
        function set.XY(this,val)
            
            %% validate
            if this.BeingConstructed
                this.XY = val;
                return;
            end
            assert(isnumeric(val)&&numel(val)==2,'XY must by 1x2 or 2x1 array specifying X and Y center position of the scalebar');
            this.XY = val;
            
            if ~this.changeXYfromLocation
                this.Location = 'manual';
            end
            
            %% update Line
            this.updateLinePosition();
            
            %% update text
            this.updateTextPosition();
            
        end
        function set.Location(this,val)
            if this.BeingConstructed
                this.Location = val;
                return;
            end
            
            assert(ischar(val),'Location must be valid char array');
            val = lower(val);
            
            assert(ismember(val,{'manual','northwest','north','northeast','east','southeast','south','southwest','west'}),'Location must be a valid "compass" direction (e.g. ''southeast'')');
            
            if strcmpi(val,'manual')
                this.Location = val;
                return;
            end
            
            this.changeXYfromLocation = true;
            this.Location = val;
            XL = this.Parent.XLim;
            dX = diff(XL);
            YL = this.Parent.YLim;
            dY = diff(YL);
            dL = this.Length/this.Scale;
            switch val
                case 'northwest'
                    X = dL/2 + 0.04*dX + XL(1);
                    Y = 0.96*dY + YL(1);
                    this.XY = [X,Y];
                case 'north'
                    X = mean(XL);
                    Y = 0.96*dY + YL(1);
                    this.XY = [X,Y];
                case 'northeast'
                    X = -dL/2 + 0.96*dX + XL(1);
                    Y = 0.96*dY + YL(1);
                    this.XY = [X,Y];
                case 'east'
                    X = -dL/2 + 0.96*dX + XL(1);
                    Y = mean(YL);
                    this.XY = [X,Y];
                case 'southeast'
                    X = -dL/2 + 0.96*dX + XL(1);
                    Y = 0.04*dY + YL(1);
                    this.XY = [X,Y];
                case 'south'
                    X = mean(XL);
                    Y = 0.04*dY + YL(1);
                    this.XY = [X,Y];
                case 'southwest'
                    X = dL/2 + 0.04*dX + XL(1);
                    Y = 0.04*dY + YL(1);
                    this.XY = [X,Y];
                case 'west'
                    X = dL/2 + 0.04*dX + XL(1);
                    Y = mean(YL);
                    this.XY = [X,Y];
            end
            this.changeXYfromLocation = false;
            
        end
        function set.Color(this,val)
            if ~iscell(val)
                val = {val,val};
            end
            this.Line.Color = val{1};
            this.Text.Color = val{2};
        end
        function clrs = get.Color(this)
            clrs = {[],[]};
            clrs{1} = this.Line.Color;
            clrs{2} = this.Text.Color;
        end
        function txt = get.Label(this)
            txt = sprintf('%g %s',this.Length,this.Unit);
        end
        function set.DragEnabled(this,val)
            assert(isscalar(val),'DragEnable must be scalar and convertable to logical');
            val = logical(val);
            this.DragEnabled = val;

            this.Line.DragEnabled = val;
        end
    end
    
    %% create
    methods
        function this = scalebar(varargin)
            %% Setup Parent
            
            % initiate graphics parent related variables
            this@extras.GraphicsChild('axes');
            %look for parent specified in arguments
            varargin = this.CheckParentInput(varargin{:});
            
            %% Create Line
            
            XL = this.Parent.XLim;
            YL = this.Parent.YLim;
            
            Y = 0.04*(YL(2)-YL(1))+YL(1);
            this.Orientation = 'horizontal';
            
            %fraction of X
            dX = (this.Length/this.Scale); %Length in axis Units
            xfrac = dX/(XL(2)-XL(1));
            
            if xfrac>1 %just place in center
               this.Location = 'south';
               X = mean(XL);
            else %place off to right
                this.Location = 'southeast';
                X = XL(2) - 0.04*(XL(2)-XL(1)) - dX/2;
            end
            this.XY = [X,Y];
            this.Line = extras.DraggableLine(this.Parent,X+[-dX/2,dX/2],[Y,Y],'Color','r','LineWidth',3);
            
            %position listeners
            this.PositionListenerX = addlistener(this.Line,'X','PostSet',@(~,~) this.PositionChanged);
            this.PositionListenerY = addlistener(this.Line,'Y','PostSet',@(~,~) this.PositionChanged);
            this.DragEnabeledListener = addlistener(this.Line,'DragEnabled','PostSet',@(~,~) set(this,'DragEnabled',this.Line.DragEnabled));
            
            %% Create Text
            this.Text = text(this.Parent,X,Y,this.Label,...
                'HorizontalAlignment','center',...
                'VerticalAlignment','bottom',...
                'SelectionHighlight','off',...
                'ButtonDownFcn',@(~,~) this.Line.MouseClick_Line,...
                'Interruptible','off',...
                'HandleVisibility','callback',...
                'Margin',5,...
                'Color','r');
            
            %% Create context menu
            this.ContextMenu = uicontextmenu(this.ParentFigure);
            uimenu(this.ContextMenu,'Text','Edit Scalebar','Callback',@(~,~) this.editUI);
            
            this.Text.UIContextMenu = this.ContextMenu;
            this.Line.UIContextMenu = this.ContextMenu;
            
            %% Axes ordering listener
            this.AxesOrderListener = addlistener(this.Parent,'ChildAdded',@(~,~) this.bringToFront);
            %% Turn off BeingConstructed flag
            this.BeingConstructed = false;
            
            %% Set others
            set(this,varargin{:});
        end
    end
    
    %% delete
    methods
        function delete(this)
            delete(this.AxesOrderListener);
            delete(this.ContextMenu);
            delete(this.PositionListenerX);
            delete(this.PositionListenerY);
            delete(this.DragEnabeledListener);
            delete(this.Line);
            delete(this.Text);
        end
    end
    
    %% Callbacks
    methods (Hidden)
        function bringToFront(this)
            if isvalid(this)&&this.KeepOnTop
                uistack(this.Line,'top');
                uistack(this.Text,'top');
            end
        end
        function editUI(this)
            resp = extras.inputdlg(...
                {'Length',...
                'Unit',...
                'Scale [SB_Units/AxesUnits]',...
                'Color'},...
                'Edit Scalebar',...
                1,...
                {num2str(this.Length),...
                this.Unit,...
                num2str(this.Scale),...
                num2str(this.Color{1})});
            if isempty(resp)
                return;
            end
            %length
            L = str2double(resp{1});
            try
            if ~isnan(L)
                this.Length = L;
            end
            catch
                warning('Invalid Length');
            end
            %unit
            U = resp{2};
            try
            this.Unit = U;
            catch
                warning('Invalid Unit');
            end
            %Scale
            S = str2double(resp{3});
            try
            if ~isnan(S)
                this.Scale = S;
            end
            catch
                warning('Invalid Scale');
            end
            %color
            str = resp{4};
            str(str=='''')=[];
            RGB = extras.colorname2rgb(str);
            if any(isnan(RGB))
                RGB = str2num(str);
            end
            try
                this.Color = RGB;
            catch
                warning('Invalid Color');
            end
            
            
        end
        function PositionChanged(this)
            this.changeXYfromPCL = true;
            
            switch(this.Orientation)
                case 'horizontal'
                    X = mean(this.Line.X);
                    Y = this.Line.Y(1);
                case 'vertical'
                    Y = mean(this.Line.Y);
                    X = this.Line.X(1);
            end
            
            this.XY = [X,Y];
            this.changeXYfromPCL = false;
        end
        function updateTextPosition(this)
            if ~isvalid(this)
                return;
            end
            XL = this.Parent.XLim;
            YL = this.Parent.YLim;
            this.Text.Position = this.XY;
            switch(this.Orientation)
                case 'horizontal'
                    if this.XY(2) <= mean(YL) %below middle of axes
                        this.Text.VerticalAlignment = 'bottom';
                        this.Text.HorizontalAlignment = 'center';
                        this.Text.Rotation = 0;
                    else %above middle
                        this.Text.VerticalAlignment = 'top';
                        this.Text.HorizontalAlignment = 'center';
                        this.Text.Rotation = 0;
                    end
                case 'vertical'
                    if this.XY(1) <= mean(XL) %Left side
                        this.Text.VerticalAlignment = 'top';
                        this.Text.HorizontalAlignment = 'center';
                        this.Text.Rotation = 90;
                    else %right side
                        this.Text.VerticalAlignment = 'bottom';
                        this.Text.HorizontalAlignment = 'center';
                        this.Text.Rotation = 90;
                    end
            end
        end
        
        function updateLinePosition(this)
            if ~this.changeXYfromPCL %don't set line pos if XY is being updated by changes in line pos
                dX = this.Length/this.Scale;
                switch(this.Orientation)
                    case 'horizontal'
                        YY = [this.XY(2),this.XY(2)];
                        XX = [this.XY(1)-dX/2,this.XY(1)+dX/2];
                        this.Line.X = XX;
                        this.Line.Y = YY;
                    case 'vertical'
                        YY = [this.XY(2)-dX/2,this.XY(2)+dX/2];
                        XX = [this.XY(1),this.XY(1)];
                        this.Line.Y = YY;
                        this.Line.X = XX;
                end
            end
        end
    end
end