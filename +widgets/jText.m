classdef jText <  extras.RequireGuiLayoutToolbox & ...
         extras.widgets.abstract.HGJContainer & ...
         extras.widgets.mixin.AssignNV & ...
         matlab.mixin.CustomDisplay
     
    properties (AbortSet=true, SetObservable=true)
        String %char
        Orientation char ='horizontal'%{mustBeMember(Orientation,{'horizontal','clockwise','counterclockwise'})};
        HorizontalAlignment = 'leading'
        VerticalAlignment = 'center';
    end
    
    %% Display Customization
    methods (Access=protected)
        
        function propGroup = getPropertyGroups(this)
            titleTxt = 'extras.widgets.jText Properties:';
            thisProps = struct(...
                'String',this.String,...
                'Orientation',this.Orientation,...
                'HorizontalAlignment',this.HorizontalAlignment,...
                'VerticalAlignment',this.VerticalAlignment...
                );
            propGroup = matlab.mixin.util.PropertyGroup(thisProps,titleTxt);
        end %function
      
    end %methods   
    
    %% internal use only
    properties(Access=private)
        jLabel_IsConstructed = false; %flag if object is constructed
        jLabel %java object
    end
    properties(Constant,Access=private)
        jLabel_RotateUIccw = javaObjectEDT('com.jidesoft.plaf.xerto.VerticalLabelUI',false);
        jLabel_RotateUIcw = javaObjectEDT('com.jidesoft.plaf.xerto.VerticalLabelUI',true);
    end
    
    %% constructor
    methods
        function this = jText(varargin)
        % Constructs jText ui object.
        % Usage:
        %   jT = jText(); constructs uninitialized jText object
        %   jT = jText(parent); construct jText inside parent object
        %   jT = jText('Name',value,...) construct and initialize jText properties
            
            %% Create jLabel
            jLabel = javaObjectEDT('javax.swing.JLabel');
            this@extras.widgets.abstract.HGJContainer(jLabel)
            
            this.jLabel = jLabel;

            %% set constructed flag
            this.jLabel_IsConstructed = true;
            obj.IsConstructed = true;
            
            %% Validate parent as first arg
            if nargin>0 && ~ischar(varargin{1})&&isscalar(varargin{1}) && isgraphics(varargin{1})
                this.Parent = varargin{1};
                varargin(1) = [];
            end
            
            %% default to center alignment
            this.HorizontalAlignment = 'center';
            
            %% set parameters
            this.setPublicProperties(varargin{:});
        end
    end
    
    %% get/set
    methods
        function set.String(this,value)
            this.String = num2str(value);
            if this.jLabel_IsConstructed
                this.jLabel.setText(this.String);
            end
        end
        function set.Orientation(this,value)
            value = validatestring(value,{'horizontal','vertical','clockwise','counterclockwise'});
            %this.Orientation = value;
            switch value
                case 'horizontal'
                    if this.jLabel_IsConstructed
                        this.jLabel.updateUI();
                    end
                    this.Orientation = 'horizontal';
                case 'clockwise'
                    if this.jLabel_IsConstructed
                        this.jLabel.setUI(this.jLabel_RotateUIcw);
                    end
                    this.Orientation = 'clockwise';
                case {'vertical','counterclockwise'}
                    if this.jLabel_IsConstructed
                        this.jLabel.setUI(this.jLabel_RotateUIccw);
                    end
                    this.Orientation = 'counterclockwise';
            end
                    
                    
        end
        
        function set.HorizontalAlignment(this,value)
            
            numeric_val = javax.swing.SwingConstants.LEADING;
            if isnumeric(value)
                assert(isscalar(value),'setting HorizontalAlignment via numeric requires scalar');
                switch value
                    case javax.swing.SwingConstants.LEFT
                        this.HorizontalAlignment = 'left';
                    case javax.swing.SwingConstants.RIGHT
                        this.HorizontalAlignment = 'right';
                    case javax.swing.SwingConstants.CENTER
                        this.HorizontalAlignment = 'center';
                    case javax.swing.SwingConstants.LEADING
                        this.HorizontalAlignment = 'leading';
                    case javax.swing.SwingConstants.TRAILING
                        this.HorizontalAlignment = 'trailing';
                    otherwise
                        error('Undefined Horizontal Alignment');
                end
                numeric_val = value;
            else
                this.HorizontalAlignment = validatestring(value,{'left','right','center','leading','trailing'});
                switch this.HorizontalAlignment
                    case 'left'
                        numeric_val = javax.swing.SwingConstants.LEFT;
                    case 'right'
                        numeric_val = javax.swing.SwingConstants.RIGHT;
                    case 'center'
                        numeric_val = javax.swing.SwingConstants.CENTER;
                    case 'leading'
                        numeric_val = javax.swing.SwingConstants.LEADING;
                    case 'trailing'
                        numeric_val = javax.swing.SwingConstants.TRAILING;
                end
            end
            if this.jLabel_IsConstructed
                this.jLabel.setHorizontalAlignment(numeric_val)
            end
        end
        
        function set.VerticalAlignment(this,value)
            
            numeric_val = javax.swing.SwingConstants.CENTER;
            if isnumeric(value)
                assert(isscalar(value),'setting VerticalAlignment via numeric requires scalar');
                switch value
                    case javax.swing.SwingConstants.TOP
                        this.VerticalAlignment = 'top';
                    case javax.swing.SwingConstants.CENTER
                        this.VerticalAlignment = 'center';
                    case javax.swing.SwingConstants.BOTTOM
                        this.VerticalAlignment = 'bottom';
                    otherwise
                        error('Undefined Vertical Alignment');
                end
                numeric_val = value;
            else
                this.VerticalAlignment = validatestring(value,{'top','center','bottom'});
                switch this.VerticalAlignment
                    case 'top'
                        numeric_val = javax.swing.SwingConstants.TOP;
                    case 'center'
                        numeric_val = javax.swing.SwingConstants.CENTER;
                    case 'bottom'
                        numeric_val = javax.swing.SwingConstants.BOTTOM;
                end
            end
            if this.jLabel_IsConstructed
                this.jLabel.setVerticalAlignment(numeric_val)
            end
        end
    end 
end