classdef plotCaliper < extras.GraphicsChild & extras.widgets.mixin.AssignNV
% UI tool for measuring distances on a plot    
    
    %% 
    properties(Access=protected)
        PrimaryEdgeLine = extras.DraggableLine.empty();
        SecondaryEdgeLine = extras.DraggableLine.empty();
        SpanLine = gobjects(1);
    end
    
    %% public
    properties(SetObservable,AbortSet)
        BaseLineX (1,2) double = [NaN,NaN]; %coordinates for caliper base line
        BaseLineY (1,2) double= [NaN,NaN]; %coordinates for caliper base line
        Span (1,1) double = NaN; %span of the caliper
    end
    
    %% Dependent
    properties(Dependent)
        Color 
        LineStyle
        LineWidth
        Marker
        MarkerSize
        MarkerEdgeColor
        MarkerFaceColor
    end
    %get/set methods
    methods
        function set.Color(this,val)
            this.PrimaryEdgeLine.Color = val;
            this.SecondaryEdgeLine.Color = val;
        end
        function val = get.Color(this)
            val = this.PrimaryEdgeLine.Color;
        end
        
        function set.LineStyle(this,val)
            this.PrimaryEdgeLine.LineStyle = val;
            this.SecondaryEdgeLine.LineStyle = val;
        end
        function val = get.LineStyle(this)
            val = this.PrimaryEdgeLine.LineStyle;
        end
        
        function set.LineWidth(this,val)
            this.PrimaryEdgeLine.LineWidth = val;
            this.SecondaryEdgeLine.LineWidth = val;
        end
        function val = get.LineWidth(this)
            val = this.PrimaryEdgeLine.LineWidth;
        end
        
        function set.Marker(this,val)
            this.PrimaryEdgeLine.Marker = val;
            this.SecondaryEdgeLine.Marker = val;
        end
        function val = get.Marker(this)
            val = this.PrimaryEdgeLine.Marker;
        end
        
        function set.MarkerSize(this,val)
            this.PrimaryEdgeLine.MarkerSize = val;
            this.SecondaryEdgeLine.MarkerSize = val;
        end
        function val = get.MarkerSize(this)
            val = this.PrimaryEdgeLine.MarkerSize;
        end
        
        function set.MarkerEdgeColor(this,val)
            this.PrimaryEdgeLine.MarkerEdgeColor = val;
            this.SecondaryEdgeLine.MarkerEdgeColor = val;
        end
        function val = get.MarkerEdgeColor(this)
            val = this.PrimaryEdgeLine.MarkerEdgeColor;
        end
        
        function set.MarkerFaceColor(this,val)
            this.PrimaryEdgeLine.MarkerFaceColor = val;
            this.SecondaryEdgeLine.MarkerFaceColor = val;
        end
        function val = get.MarkerFaceColor(this)
            val = this.PrimaryEdgeLine.MarkerFaceColor;
        end
    end
    
    %% Create
    methods
        function this = plotCaliper(varargin)
        % Construct Caliper
        % Syntax:
        %   plotCaliper(): construct in current axes, prompt for
        %   coordinates
        %   plotCaliper(hax): construct in axes hax
        %   plotCaliper('Parent',hax,...)
        %
            
            %% Handle Inputs
            iH = extras.inputHandler;
            iH.KeepUnmatched = true;
            
            iH.addOptionalVariable('Parent',[],@(x) isgraphics(x)&&strcmpi(x.Type,'axes'),true);
            iH.addOptionalVariable('BaseLineX',[],@(x) isnumeric&&numel(x)==2);
            iH.addOptionalVariable('BaseLineY',[],@(x) isnumeric&&numel(x)==2);
            iH.addOptionalVariable('Span',[],@(x) isnumeric&&numel(x)==2);
            
            iH.parse(varargin{:});
            
            prompt_for_coords = false;
            if any(ismember({'BaseLineX','BaseLineY','Span'},iH.UsingDefaults))
                prompt_for_coords = true;
                assert(all(ismember({'BaseLineX','BaseLineY','Span'},iH.UsingDefaults)),'BaseLineX,BaseLineY,and Span must all be specified, or none must be specified');
            end
            
            %% Setup Parent
            %initiate graphics parent related variables
            this@extras.GraphicsChild('axes');
            
            if isempty(iH.Results.Parent)
                this.CheckParentInput();
            else
                this.CheckParentInput(iH.Results.Parent);
            end
            
            %% Prompt for Coords if needed
            if prompt_for_coords
                hLn = line('Parent',this.Parent,'XData',NaN(1,2),'YData',NaN(1,2),...
                    'Color','r','LineWidth',1.5);
                orig_title = this.Parent.Title.String;
                this.Parent.Title.String = 'Select Caliper Base Line. (Press Esc. to cancel.)';
                axes(this.Parent);
                [x,y] = ginput(1);
                if isempty(x)
                    this.Parent.Title.String = orig_title;
                    delete(hLn);
                    error('User canceled selecting base line');
                end
                hLn.XData(1) = x;
                hLn.YData(1) = y;
                
                %% select second point of baseline
                orig_motion = this.ParentFigure.WindowButtonMotionFcn;
                this.ParentFigure.WindowButtonMotionFcn = @(~,~) this.mouse_mv_prompt(hLn,this.Parent);
                pt = extras.getClick(this.Parent);
                this.ParentFigure.WindowButtonMotionFcn = orig_motion;
                if isempty(pt)
                    this.Parent.Title.String = orig_title;
                    delete(hLn);
                    error('User canceled selecting base line');
                end
                x2 = pt(1,1);
                y2 = pt(1,2);

                hLn.XData(2) = x2;
                hLn.YData(2) = y2;
                
                
                this.BaseLineX = [x,x2];
                this.BaseLineY = [y,y2];
                %% select span
                hSp = line('Parent',this.Parent,'XData',[mean(this.BaseLineX),NaN],'YData',[mean(this.BaseLineY),NaN],...
                    'Color','k','LineWidth',1.5,'LineStyle','--');
                orig_motion = this.ParentFigure.WindowButtonMotionFcn;
                this.ParentFigure.WindowButtonMotionFcn = @(~,~) this.span_motion(hSp,this.Parent,this.BaseLineX,this.BaseLineY);
                
                this.Parent.Title.String = 'Select Caliper Span. (Press Esc. to cancel.)';
                pt = extras.getClick(this.Parent);
                this.ParentFigure.WindowButtonMotionFcn = orig_motion;
                if isempty(pt)
                    this.Parent.Title.String = orig_title;
                    delete(hLn);
                    delete(hSp);
                    error('User canceled selecting span');
                end
                %compute span
                a = [this.BaseLineX(1),this.BaseLineY(1)];
                b = [this.BaseLineX(2),this.BaseLineY(2)];
                L = b-a;
                L_hat = L/sqrt(sum(L.^2));
                m = [mean(this.BaseLineX),mean(this.BaseLineY)];
                c = [pt(1,1),pt(1,2)];
                R = c-m;
                vv = L_hat*R';
                V = vv*L_hat;
                S = R-V;
                this.Span =sqrt(sum(S.^2));
                
                %% cleanup
                this.Parent.Title.String = orig_title;
                delete(hLn);
                delete(hSp);
            end
            
            %% construct draggable lines
            this.PrimaryEdgeLine = extras.DraggableLine(...
                this.BaseLineX,this.BaseLineY,...
                'Parent',this.Parent,...
                'DragEnabled',true,...
                'DragEndpoints',true,...
                'DragEndpointsCallback',@(h,~) set(this,'BaseLineX',h.X,'BaseLineY',h.Y),...
                'DragBodyCallback',[]);
            
            this.SecondaryEdgeLine = extras.DraggableLine(...
                this.BaseLineX,this.BaseLineY,...
                'Color',this.PrimaryEdgeLine.Color,...
                'MarkerEdgeColor',this.PrimaryEdgeLine.MarkerEdgeColor,...
                'Parent',this.Parent,...
                'DragEnabled',true,...
                'DragEndpoints',true,...
                'UIeditCallback',@(h,~) set(this,'BaseLineX',h.X,'BaseLineY',h.Y));
            
                
        end
    end
    
    %% 
    methods(Access=private)
        
        function DragBodyPrimary(this)
            
        end
        
        function updateLines(this)
            %% Compute span coord
            a = [this.BaseLineX(1),this.BaseLineY(1)];
            b = [this.BaseLineX(2),this.BaseLineY(2)];
            B = b-a; %base line vector
            m = [mean(this.BaseLineX),mean(this.BaseLineY)]; %midpoint of baseline
            phi = atan2(B(2),B(1)); %angle of baseline
            gam = phi-pi/2;
            S = [cos(gam),sin(gam)]*this.Span;
            
            X2 = this.BaseLineX + S(1);
            Y2 = this.BaseLineY + S(2);
            
            m2 = m+S;
            
            this.PrimaryEdgeLine.X = this.BaseLineX;
            this.PrimaryEdgeLine.Y = this.BaseLineY;
            
            this.SecondaryEdgeLine.X = X2;
            this.SecondaryEdgeLine.Y = Y2;
            
            this.SpanLine.XData = [m(1),m2(1)];
            this.SpanLine.YData = [m(2),m2(2)];
            
        end
    end
    
    %% Callbacks for constructor Coord prompt
    methods(Static,Access=private)
        function mouse_mv_prompt(hLn,hax)
            pt = get(hax, 'CurrentPoint');
            hLn.XData(2) = pt(1,1);
            hLn.YData(2) = pt(1,2);
            drawnow()
        end
        function span_motion(hSp,hax,blX,blY)
            pt = get(hax, 'CurrentPoint');
            a = [blX(1),blY(1)];
            b = [blX(2),blY(2)];
            L = b-a;
            L_hat = L/sqrt(sum(L.^2));
            m = [mean(blX),mean(blY)];
            c = [pt(1,1),pt(1,2)];
            R = c-m;
            vv = L_hat*R';
            V = vv*L_hat;
            S = R-V;
            Sm = m+S;
            
            
            
            
            hSp.XData(2) = Sm(1);
            hSp.YData(2) = Sm(2);
            drawnow();
        end
    end
end