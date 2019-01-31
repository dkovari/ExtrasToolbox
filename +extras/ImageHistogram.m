classdef ImageHistogram < extras.GraphicsChild & extras.RequireWidgetsToolbox & extras.uixDerivative
% A realtime image histogram

    properties (SetObservable=true,AbortSet=true)
        Image; %handle to image
        
    end
    methods
        function set.Image(this,imh)
            assert(isgraphics(imh)&&strcmpi(imh.Type,'image'));
            
            delete(this.ImageDeleteListener);
            delete(this.CDataListener);
            
            this.Image = imh;
            
            %this.ImageDeleteListener = addlistener(imh,'ObjectBeingDestroyed',@(~,~) delete(this));
            this.CDataListener = addlistener(imh,'CData','PostSet',@(~,~) this.CDataChanged);
            
            try
                set(this.Area,'XData',[NaN,NaN],'YData',[NaN,NaN]);
            catch
            end
            this.CDataChanged();
        end
    end
    
    %% user modifiable
    properties (SetObservable,AbortSet)
        LogBase = 1;
    end
    methods
        function set.LogBase(this,val)
            assert(isnumeric(val)&&isscalar(val),'LogBase must be numeric scalar');
            this.LogBase = val;
            try
                this.Slider.Value = val;
            catch
            end
            try
                this.updatePlot;
            catch
            end
        end
    end
    
    %% user viewable
    properties(SetAccess=protected,SetObservable=true,AbortSet=true)
        Area %area plot with histogram data
        Counts %counts in histogram
        Edges %edges of histogram bars
        Axes %axes holding historgam
    end
    
    %% internal
    properties (Access=protected)
        ImageDeleteListener
        CDataListener
        OuterHBox
        Slider
    end
    

    %% delete
    methods
        function delete(this)
            delete(this.ImageDeleteListener);
            delete(this.CDataListener);
            delete(this.OuterHBox);
        end
    end
    
    %% Create
    methods
        function this = ImageHistogram(varargin)
            %% initiate graphics parent related variables
            this@extras.GraphicsChild(@() figure('Name','ImageHistogram',...
                    'NumberTitle','off',...
                    'MenuBar','none',...
                    'ToolBar','none',...
                    'HandleVisibility','callback'));
                
            %% Validate Parent
            %look for parent specified in arguments
            varargin = this.CheckParentInput(varargin{:});
                     
            %% Buid GUI
            this.OuterHBox = uix.HBox('Parent',this.Parent);
            this.Slider = uiw.widget.Slider(...
                'Parent',this.OuterHBox,...
                'Orientation','vertical',...
                'ShowLabels',false,...
                'ShowTicks',false,...
                'Min',1,...
                'Max',3,...
                'LabelVisible','off',...
                'Callback',@(~,~)this.SliderChanged,...
                'Value',1);
            
            this.Axes = axes(this.OuterHBox,'NextPlot','replacechildren');
            ylabel(this.Axes,'Counts');
            
            this.OuterHBox.Widths = [30,-1];
            
            %% Set Properties
            set(this,varargin{:});
            
            %% update
            this.CDataChanged
        end
    end
    
    %% Callbacks
    methods(Hidden)
        function CDataChanged(this)
            
            if isempty(this.Image) || ~isgraphics(this.Image)
                return;
            end
            
            if isa(this.Image.CData,'double')         
                    [cnt,edges] = histcounts(this.Image.CData(:));
            else                    
                [cnt,edges] = histcounts(this.Image.CData(:),intmin(class(this.Image.CData)):intmax(class(this.Image.CData)));
            end
            
            this.Counts = cnt;
            this.Edges = edges;
            
            this.updatePlot;
        end
        function SliderChanged(this)
            v = this.Slider.Value;
            this.LogBase = v;
        end
        
        function updatePlot(this)
            cnt = this.Counts;
            edges = this.Edges;
            
            if this.LogBase>1
                org_max = max(cnt);
                cnt = cnt.^(1/this.LogBase);
                cnt = cnt*org_max/(org_max^(1/this.LogBase));
                try
                    ylabel(this.Axes,sprintf('\\propto Count^{1/%g}',this.LogBase));
                catch
                end
            else
                try
                    ylabel(this.Axes,'Count');
                catch
                end
            end
            
            
            %convert to line points
            y= [0;reshape([cnt;cnt],[],1);0];
            x = reshape([edges;edges],[],1);
            
            if isempty(this.Area)||~isgraphics(this.Area)
                this.Area = area(this.Axes,x,y);
            else         
                this.Area.XData = x;
                this.Area.YData = y;
            end
        end
    end

end