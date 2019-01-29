classdef LabeledPanel < handle & ...
        extras.RequireGuiLayoutToolbox &...
        extras.GraphicsChild & ...
        extras.widgets.mixin.AssignNV & ...
        extras.widgets.mixin.HasCallback &...
        extras.widgets.mixin.abstract.HasLabel
    
    properties (SetObservable=true,AbortSet=true)
        LabelPosition = 'left';
        LabelWidth (1,1) {mustBeNumeric} = -1;
        LabelHeight (1,1) {mustBeNumeric} = 25;
    end
    %% get
    methods
        function lw = get.LabelWidth(this)
            if isempty(this.Label)
                lw = 0;
            else
                lw = this.LabelWidth;
            end
        end
        function lh = get.LabelHeight(this)
            if isempty(this.Label)
                lh = 0;
            else
                lh = this.LabelHeight;
            end
        end
    end
    
    
    properties(Access=protected)
        MainPanel
        
        OuterGrid
                
        hLabel
        
        LanelPanel_IsConstructed = false;
    end
    
    %% get/set
    methods
        function set.LabelPosition(this,val)
            this.LabelPosition = validatestring(val,{'left','right','top','bottom'});
            this.updateLabelPosition;
        end
        function set.LabelWidth(this,val)
            this.LabelWidth = val;
            this.updateLabelPosition;
        end
        function set.LabelHeight(this,val)
            this.LabelHeight = val;
            this.updateLabelPosition;
        end
    end
    
    methods
        function updateLabelPosition(this)
            if this.LanelPanel_IsConstructed
                switch(this.LabelPosition)
                    case 'left'
                        set( this.OuterGrid, ...
                            'Contents',[this.hLabel,this.MainPanel],...
                            'Widths', [this.LabelWidth,-1], 'Heights', -1 );
                    case 'right'
                        set( this.OuterGrid, ...
                            'Contents',[this.MainPanel,this.hLabel],...
                            'Widths', [-1,this.LabelWidth], 'Heights', -1 );
                    case 'top'
                        set( this.OuterGrid, ...
                            'Contents',[this.hLabel,this.MainPanel],...
                            'Heights', [this.LabelHeight,-1], 'Widths', -1 );
                    case 'bottom'
                        set( this.OuterGrid, ...
                            'Contents',[this.MainPanel,this.hLabel],...
                            'Heights', [-1,this.LabelHeight], 'Widths', -1 );
                end
            end
        end
        
        function this = LabeledPanel(varargin)
            %% setup graphics child
            this@extras.GraphicsChild();
            
            %% validate parent
            varargin=this.CheckParentInput(varargin{:});
            
            %% construct panels
            this.OuterGrid = uix.Grid('Parent',this.Parent);
            
            this.hLabel = uicontrol(this.OuterGrid,...
                'Style','text');
            
            this.addLabelStringObject(this.hLabel);
            %this.Label = 'Test 123';
            
            this.MainPanel = uix.HBox('Parent',this.OuterGrid);

            
            
            %% finalize
            this.LanelPanel_IsConstructed = true;
            
            %% update positions
            this.updateLabelPosition();
            
            %% set public properties
            this.setPublicProperties(varargin{:});
            
        end
    end
    
    %% Override
    methods(Access=protected)
        function onLabelChanged(this)
            this.updateLabelPosition();
        end
    end
end