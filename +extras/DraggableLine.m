classdef DraggableLine < extras.GraphicsChild
% Create a user draggable line
% Limits can be placed on the line position so that it can only be dragged
% along certain dimensions
    
    properties (SetObservable=true,AbortSet=true)
        X = [NaN,NaN] %X coordinates, If nan, then use axis limits
        Y = [NaN,NaN] %Y coordinated, if nan use axis limits
        
        DragLimitX (2,1) double = [-Inf,Inf]; %limits for dragging along x direction
        DragLimitY (2,1) double = [-Inf,Inf]; %limits for dragging along y direction
        
        UIeditCallback %user defined callback fired after line is dragged by user
        
        Color 
        LineStyle
        LineWidth
        Marker
        MarkerSize
        MarkerEdgeColor
        MarkerFaceColor
        
        UserData; %variable to hold user data
        
        DragEnabled = true; %t/f if line is movable
    end
    properties (SetAccess=private,SetObservable,AbortSet = true)
        LineXData = [NaN,NaN]; %actual x coordinates of the line
        LineYData = [NaN,NaN]; %actual y coordinates of the line
    end
    
    methods
        function set.DragLimitX(this,val)
            this.DragLimitX = sort(val);
        end
        function set.DragLimitY(this,val)
            this.DragLimitY = sort(val);
        end
        function set.X(this,val)
            assert(numel(val)==2,'X must have two elements');
            this.X = val;
            if(any(isnan(this.X)))
                delete(this.XLimListener);
                this.XLimListener = addlistener(this.Parent,'XLim','PostSet',@(~,~) this.UpdateLine);
            end
            this.UpdateLine();
        end
        function set.Y(this,val)
            assert(numel(val)==2,'Y must have two elements');
            this.Y = val;
            if(any(isnan(this.Y)))
                delete(this.YLimListener);
                this.YLimListener = addlistener(this.Parent,'YLim','PostSet',@(~,~) this.UpdateLine);
            end
            this.UpdateLine();
        end
        
        function set.Color(this,val)
            set(this.LineHandle,'Color',val);
            this.Color = this.LineHandle.Color;
        end
        function set.LineStyle(this,val)
            set(this.LineHandle,'LineStyle',val);
            this.LineStyle = this.LineHandle.LineStyle;
        end
        function set.LineWidth(this,val)
            set(this.LineHandle,'LineWidth',val);
            this.LineWidth = this.LineHandle.LineWidth;
        end
        
        function set.Marker(this,val)
            set(this.LineHandle,'Marker',val);
            this.Marker = this.LineHandle.Marker;
        end
        
        function set.MarkerSize(this,val)
            set(this.LineHandle,'MarkerSize',val);
            this.MarkerSize = this.LineHandle.MarkerSize;
        end
        
        function set.MarkerEdgeColor(this,val)
            set(this.LineHandle,'MarkerEdgeColor',val);
            this.MarkerEdgeColor = this.LineHandle.MarkerEdgeColor;
        end
        
        function set.MarkerFaceColor(this,val)
            set(this.LineHandle,'MarkerFaceColor',val);
            this.MarkerFaceColor = this.LineHandle.MarkerFaceColor;
        end
        
        function set.DragEnabled(this,val)
            assert(isscalar(val),'DragEnabled must be scalar and convertable to logical');
            this.DragEnabled = logical(val);
        end
        
    end
    
    properties(Dependent)
        Visible
        UIContextMenu
    end
    methods
        function v = get.Visible(this)
            v = this.LineHadle.Visible;
        end
        function set.Visible(this,v)
            this.LineHadle.Visible = v;
        end
        function set.UIContextMenu(this,v)
            this.LineHandle.UIContextMenu = v;
        end
        function v = get.UIContextMenu(this)
            v = this.LineHandle.UIContextMenu;
        end
    end
    
    properties (Access=protected)
        LineHandle
        DragAxis = 'X'
        
        X_orig
        Y_orig
        
        Orig_MouseMove;
        Orig_MouseUp;
        
        XLimListener
        YLimListener
        
        ClickPoint
    end
    
    events
        UIeditcomplete
    end
    
    %% Create
    methods
        function this = DraggableLine(varargin)
        % DraggableLine(X,Y)
        % DraggableLine(hAx,__)
        % DraggableLine(__,'Parent',hAx)
        %
        % Specify X=[NaN,NaN] to create a horizontal line which spans the
        % axes
        %
        % or Y=[NaN,NaN to create a vertical line which spans the axes
        % 
        % By setting one of the Dimensions to [NaN,NaN] the line acts like
        % a slide and can only be dragged in the non-NaN direction
            
            %% Setup Parent
            %initiate graphics parent related variables
            this@extras.GraphicsChild('axes');
            %look for parent specified in arguments
            varargin = this.CheckParentInput(varargin{:});
            
            %% Create Line
            this.LineHandle = line(this.Parent,[NaN,NaN],[NaN,NaN],...
                'LineWidth',1.5,...
                'HandleVisibility','callback',...
                'DeleteFcn',@(~,~) delete(this),...
                'SelectionHighlight','off',...
                'Interruptible','off',...
                'ButtonDownFcn',@(~,~) this.MouseClick_Line() );
            
            this.Color = this.LineHandle.Color;
            this.LineStyle = this.LineHandle.LineStyle;
            this.LineWidth = this.LineHandle.LineWidth;
            this.Marker = this.LineHandle.Marker;
            this.MarkerSize = this.LineHandle.MarkerSize;
            this.MarkerEdgeColor = this.LineHandle.MarkerEdgeColor;
            this.MarkerFaceColor = this.LineHandle.MarkerFaceColor;
            
            %% Get X and Y locations
            assert(numel(varargin)>=2,'X and Y line coordinates must be specified');
            
            this.X = varargin{1};
            this.Y = varargin{2};
            
            if any(isnan(this.X))&&any(isnan(this.Y))
                error('Both X and Y contained NaNs, cannot create line');
            end
            
            assert(numel(this.X)==2,'X must have two elements');
            assert(numel(this.Y)==2,'Y must have two elements');
            
            %create listeners for lim changes
            if(any(isnan(this.X)))
                this.XLimListener = addlistener(this.Parent,'XLim','PostSet',@(~,~) this.UpdateLine);
            end
            if(any(isnan(this.Y)))
                this.YLimListener = addlistener(this.Parent,'YLim','PostSet',@(~,~) this.UpdateLine);
            end
            
            this.UpdateLine();
            
            if all(isnan(this.X))
                this.DragAxis = 'Y';
            elseif all(isnan(this.Y))
                this.DragAxis = 'X';
            else
                this.DragAxis = 'Parallel';
            end
            
            %% Set other parameters
            set(this,varargin{3:end});
                  
        end
        
        function delete(this)
            delete(this.LineHandle);
            delete(this.XLimListener);
            delete(this.YLimListener);
        end
    end
    
    %% overloads
    methods
        function uistack(this,varargin)
            uistack(this.LineHandle,varargin{:});
        end
    end
    
    %% other
    methods
        function UpdateLine(this)
            if ~isvalid(this)
                return
            end
            x = this.X;
            y = this.Y;
            if isnan(this.X(1))
                x(1) = this.Parent.XLim(1);
            end
            if isnan(this.X(2))
                x(2) = this.Parent.XLim(2);
            end
            if isnan(this.Y(1))
                y(1) = this.Parent.YLim(1);
            end
            if isnan(this.Y(2))
                y(2) = this.Parent.YLim(2);
            end
            
            set(this.LineHandle,'XData',x,'YData',y);
            
            this.LineXData = this.LineHandle.XData;
            this.LineYData = this.LineHandle.YData;
            
        end
    end
    
    %% Callbacks
    methods(Hidden) %these are hidden, but if you know abou them you can use them. It's a good way for external classes to trigger the drag callbacks
        function MouseClick_Line(this)
            
            if ~this.DragEnabled
                return;
            end
            
            this.ClickPoint = get(this.Parent, 'CurrentPoint');
            this.X_orig = this.X;
            this.Y_orig = this.Y;
            %set ui callbacks
            this.Orig_MouseMove = this.ParentFigure.WindowButtonMotionFcn;
            this.Orig_MouseUp = this.ParentFigure.WindowButtonUpFcn;
            this.ParentFigure.WindowButtonUpFcn = @(h,e) this.MouseUp(h,e);
            this.ParentFigure.WindowButtonMotionFcn = @(h,e) this.MouseMove(h,e);
            
            
        end
        function MouseMove(this,~,~)
            if ~this.DragEnabled
                return;
            end
            
            pt = get(this.Parent, 'CurrentPoint');
            
            dPt = pt-this.ClickPoint;
            switch(this.DragAxis)
                case 'X'
                    this.X = max(this.DragLimitX(1),min(this.DragLimitX(2),this.X_orig + dPt(1,1)));
                case 'Y'
                    this.Y = max(this.DragLimitY(1),min(this.DragLimitY(2),this.Y_orig + dPt(1,2)));
                case 'Parallel'
                    this.X = max(this.DragLimitX(1),min(this.DragLimitX(2),this.X_orig + dPt(1,1)));
                    this.Y = max(this.DragLimitY(1),min(this.DragLimitY(2),this.Y_orig + dPt(1,2)));
            end
            %this.UpdateLine();
        end
        function MouseUp(this,~,~)            
            this.ParentFigure.WindowButtonUpFcn = this.Orig_MouseUp;
            this.ParentFigure.WindowButtonMotionFcn = this.Orig_MouseMove;
            
            if ~this.DragEnabled
                return;
            end
            
            this.UpdateLine();
            
            %fire uieditcallback
            hgfeval(this.UIeditCallback,this,struct('Event','DragDone'));
            
            %notify event listeners
            notify(this,'UIeditcomplete');
            
            
        end
    end
end