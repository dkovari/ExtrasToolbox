 classdef jSlider < extras.RequireWidgetsToolbox & ...
         uiw.abstract.JavaControl & ...
         extras.widgets.mixin.HasScalarLimits &...
         extras.widgets.mixin.HasCallback & ...
         extras.widgets.mixin.AssignNV & ...
         extras.widgets.mixin.HasTooltip
     
    properties (AbortSet=true, SetObservable=true)
        Orientation char {mustBeMember(Orientation,{'horizontal','vertical'})} = 'horizontal' % Slider orientation [horizontal|vertical]
        ShowTicks (1,1) logical = true % Whether to show ticks [(true)|false]
        ShowLabels (1,1) logical = true % Whether to show tick labels [(true)|false]
        Focusable (1,1) logical = true % Allow slider to have focus border, and keyboard control [(true)|false]
        
        MajorTickValues = 'auto';
        %SnapToMajorTicks (1,1) logical = false;
        
        OnMotionCallback function_handle = function_handle.empty(0,1) %function handle to call when the widget is edited
    end
    
    methods(Access=protected)
        function callOnMotionCallback(this)
            if ~isempty(this.OnMotionCallback)
                this.OnMotionCallback();
            end
        end
    end
    
    properties(SetAccess=protected,SetObservable=true,AbortSet=true)
        PendingValue
    end
    
    properties (Access=protected)
        Multiplier (1,1) double {mustBePositive, mustBeFinite} = 1 % Multiplier used for internal calculation
        jSlider_IsConstructed = false;
    end
    
       %% Constructor / Destructor
    methods
        
        function obj = jSlider(varargin)
            %% Construct the control
            %obj@uiw.abstract.JavaControl();
            
            obj.Min = 0;
            obj.Max = 100;
            obj.Value = 0;
            
            % Create the slider
            obj.createJControl('javax.swing.JSlider');
            obj.JControl.StateChangedCallback = @(h,e)onSliderMotion(obj,e);
            obj.JControl.MouseReleasedCallback = @(h,e)onSliderChanged(obj,e);
            obj.HGJContainer.Units = 'pixels';
            obj.JControl.setOpaque(false);
            
            % Use the default value
            obj.Value = obj.JControl.getValue();
            
            % Set properties from P-V pairs
            obj.setPublicProperties(varargin{:});
            
            % No new value is pending
            obj.PendingValue = obj.Value;
            
            %% Min/Max Listener
            addlistener(obj,'Min','PostSet',@(~,~) obj.updateSlider());
            addlistener(obj,'Max','PostSet',@(~,~) obj.updateSlider());
            
            % Assign the construction flag
            obj.jSlider_IsConstructed = true;
            
            %% Redraw the widget
            obj.onResized();
            obj.onEnableChanged();
            obj.onStyleChanged();
            obj.redraw();
            
        end % constructor
        
    end %methods - constructor/destructor
    
       %% Protected methods
    methods (Access=protected)
        
        function updateSlider(obj)
            if obj.jSlider_IsConstructed
                obj.onResized();
                obj.onStyleChanged();
                obj.redraw();
            end
        end
        
        function onValueChanged(this)
        % Called by set.Value after changing this.Value, but before
        % exiting set.Value() and executing listener callbacks 
        % default impelementation does nothing.
        % derived classes can use this to perform special operations before
        % set.Value exits.
            this.redraw
        end
        
        function redraw(obj)
            % Handle state changes that may need UI redraw
            
            % Ensure the construction is complete
            if obj.jSlider_IsConstructed
                
                % Update slider value
                jValue = round(obj.Value * obj.Multiplier); %only integers accepted
                javaMethodEDT('setValue',obj.JControl,jValue);
                
                % Are we enforcing the range? If not, we need to recheck
                % coloring.
                if ~obj.EnforceLimits
                    obj.onStyleChanged();
                end
                
            end %if obj.jSlider_IsConstructed
            
        end %function
        
        function onResized(obj,~,~)
            % Handle changes to widget size
            
            % Ensure the construction is complete
            if obj.jSlider_IsConstructed
                
                % Get widget dimensions
                [w,h] = obj.getInnerPixelSize;
                pad = obj.Padding;
                spc = obj.Spacing;
                hT = 0;%min(obj.TextHeight, h);
                wT = 0;%min(obj.TextWidth, w);
                
                % Calculate new positions
                if strcmpi(obj.Orientation,'horizontal')
                    javaMethodEDT('setOrientation',obj.JControl,false);
                    wT = min(wT, w/2);
                    pad = floor( min(pad, wT/8) );
%                     if obj.FlipText
%                         div = wT+pad;
%                         tPos = [1 (h-hT)/2 wT hT];
%                         jPos = [1+(div+pad) 1 (w-div-pad) h];
%                     else
                        div = w-wT-pad;
                        jPos = [1 1 (div-pad) h];
%                         tPos = [1+(div+pad) (h-hT)/2 wT hT];
%                     end
                else %vertical
                    javaMethodEDT('setOrientation',obj.JControl,true);
                    hT = min(hT, h/2);
                    pad = floor( min(spc/2, hT/8) );
%                     if obj.FlipText
%                         div = h-hT-pad;
%                         jPos = [1 1 w (div-pad)];
%                         tPos = [1 1+(div+pad) w hT];
%                     else
                        div = hT+pad;
%                         tPos = [1 1 w hT];
                        jPos = [1 1+(div+pad) w (h-div-pad)];
%                     end
                end
                
                % Set positions
                obj.HGJContainer.Position = jPos;
                
                % Redraw ticks
                obj.redrawTicks();
                
                % Update slider value
                jValue = obj.Value * obj.Multiplier;
                javaMethodEDT('setValue',obj.JControl,jValue);
                
            end %if obj.jSlider_IsConstructed
            
        end %function
        
        function onEnableChanged(obj,~)
            % Handle updates to Enable state
            
            % Ensure the construction is complete
            if obj.jSlider_IsConstructed
                
                % Call superclass methods
                onEnableChanged@uiw.abstract.JavaControl(obj);
                
            end %if obj.jSlider_IsConstructed
            
        end %function
        
        function onStyleChanged(obj,~)
            % Handle updates to style and value validity changes
            
            % Ensure the construction is complete
            if obj.jSlider_IsConstructed
                
                % Override edit text colors
%                 obj.TextForegroundColor = obj.ForegroundColor;
%                 obj.TextBackgroundColor = obj.BackgroundColor;
                
                % Call superclass methods
                onStyleChanged@uiw.abstract.JavaControl(obj);
                
                % Are we enforcing the range?
                if ~obj.EnforceLimits && ...
                        (obj.Value>obj.Max) || (obj.Value<obj.Min)
                    % Color yellow if the value is out of range
                    set(obj.hTextFields, 'BackgroundColor', [1 1 .7]);
                end
                
            end %if obj.jSlider_IsConstructed
            
        end %function
        
        function onSliderMotion(obj,~)
            if obj.jSlider_IsConstructed 
                obj.PendingValue = obj.JControl.getValue() / obj.Multiplier;
                if obj.PendingValue ~= obj.Value
                    obj.callOnMotionCallback()
%                     
%                     if obj.NotifyOnMotion %Undocumented - may be removed
%                         obj.onSliderChanged();
%                     else
%                         obj.onValueChanging(obj.PendingValue);
%                     end
                end
            end
        end
        
        function onSliderChanged(obj,~)
            % Handle interaction with slider
            newValue = obj.JControl.getValue() / obj.Multiplier;
            if ~isequal(newValue,obj.Value)
                evt = struct('Source', obj, ...
                    'Interaction', 'Slider Changed', ...
                    'OldValue', obj.Value, ...
                    'NewValue', newValue);
                %disp(obj.Value)
                obj.Value = newValue;
                %disp(obj.Value)
                obj.redraw();
                obj.callCallback(evt);
            end
        end %function

        function redrawTicks(obj)
            
            % We want to have up to 10 major ticks and five minor ticks in
            % between. We try to get major ticks on powers of ten.
            
            % Ensure the construction is complete
            if obj.jSlider_IsConstructed
                
                % Get the widget width and use it to determine the maximum
                % number of tick marks. We allow a minimum of 25 pixels between
                % each major tick-mark.
                % Get widget dimensions
                [w,h] = obj.getInnerPixelSize;
                if strcmpi(obj.Orientation,'horizontal')
                    maxMajorTicks = floor(w/25);
                    len_dim = w;
                else
                    maxMajorTicks = floor(h/25);
                    len_dim = h;
                end
                maxMajorTicks = max(maxMajorTicks-1, 2);
                
                % Work out our desired spacing
                range = (obj.Max - obj.Min);
                major = power( 10, ceil( log10( range/100 ) ) );
                
                if major <= 0%obj.MinTickStep
                    major = 0;%obj.MinTickStep;
                end
                
                % Increase the spacing until we have sufficiently few
                while range/major > maxMajorTicks
                    if range/(major*2) <= maxMajorTicks
                        major = major*2;
                    elseif range/(major*5) <= maxMajorTicks
                        major = major*5;
                    else
                        major = major*10;
                    end
                end
                
                % Minor ticks are 5 per major tick, or use minumum
                minor = max(0, major/5);
                
                % We need to use integers so use a multiplier if spacing is
                % fractional
                obj.Multiplier = max(1/minor, 1);
                mMin = obj.Min;
                mMax = obj.Max;
                jMin = mMin * obj.Multiplier;
                jMax = mMax * obj.Multiplier;
                
                % The Java integer equivalent of the tick spacing
                jMinor = fix(minor * obj.Multiplier);
                jMajor = fix(major * obj.Multiplier);
                
                % Now set the min/max and spacing
                javaMethodEDT('setMinimum',obj.JControl,jMin);
                javaMethodEDT('setMaximum',obj.JControl,jMax);
                %javaMethodEDT('setMinorTickSpacing',obj.JControl,jMinor);
                javaMethodEDT('setMajorTickSpacing',obj.JControl,jMajor);
                
                % Set ticks display on/off
                javaMethodEDT('setPaintTicks',obj.JControl,obj.ShowTicks);
                javaMethodEDT('setPaintLabels',obj.JControl,obj.ShowLabels);
                %javaMethodEDT('setSnapToTicks',obj.JControl,false);%obj.SnapToTicks);
                
                if jMinor==1 && 1/100*len_dim < 2 %minor ticks will be 3pixels apart, don't show and don't snap
                    javaMethodEDT('setMinorTickSpacing',obj.JControl,0);
                    javaMethodEDT('setSnapToTicks',obj.JControl,false);%obj.SnapToTicks);
                elseif jMinor==1
                    javaMethodEDT('setMinorTickSpacing',obj.JControl,1);
                    javaMethodEDT('setSnapToTicks',obj.JControl,true);
                else
                    javaMethodEDT('setMinorTickSpacing',obj.JControl,jMinor);
                    javaMethodEDT('setSnapToTicks',obj.JControl,false);
                end
                    
                
                % The labels need to recreated to lie on the major ticks
                if obj.ShowTicks || obj.ShowLabels
                    
                    jHash = java.util.Hashtable();
                    fgCol = obj.ForegroundColor;
                    
                    if isempty(obj.MajorTickValues)||strcmpi(obj.MajorTickValues,'auto') %strcmpi(obj.LabelMode,'auto') || isempty(obj.CustomLabels.values)
                         
                        % Make the ticks fall on even multiples of major tick
                        % spacing. This only works if tick marks are off,
                        % as they do not have a way to offset them.
                        if mod(mMin,major)>0 && ~obj.ShowTicks
                            jFirstMajor = ceil(mMin/major)* major * obj.Multiplier;
                            jTicks = int32( jFirstMajor : jMajor : jMax );
                            if (jFirstMajor - jMin) < jMajor/2
                                jTicks(1) = jMin;
                            else
                                jTicks = [jMin jTicks];
                            end
                        else
                            jTicks = int32( jMin : jMajor : jMax );
                        end
                        
                        mTicks = double(jTicks) / obj.Multiplier;
                        
                        for idx=1:numel(jTicks)
                            jThisLabel = javax.swing.JLabel(num2str(mTicks(idx)));
                            jThisLabel.setForeground( java.awt.Color(fgCol(1), fgCol(2), fgCol(3)) )
                            jHash.put(jTicks(idx),jThisLabel);
                        end
                    else
                        mTicks = obj.MajorTickValues;
                        mTicks = mTicks(mTicks>=obj.Min&mTicks<=obj.Max);
                        
                        jTicks = int32(mTicks*obj.Multiplier);
                        
                        for idx=1:numel(jTicks)
                            jThisLabel = javax.swing.JLabel(num2str(mTicks(idx)));
                            jThisLabel.setForeground( java.awt.Color(fgCol(1), fgCol(2), fgCol(3)) )
                            jHash.put(jTicks(idx),jThisLabel);
                        end
                        
%                     else
%                         
%                         % Manual tick labels
%                         values = obj.CustomLabels.values;
%                         if iscell(values)
%                             values = cell2mat(values);
%                         end
%                         jTicks = int32(values);
%                         keys = obj.CustomLabels.keys;
%                         for idx=1:numel(jTicks)
%                             jThisLabel = javax.swing.JLabel(keys{idx});
%                             jThisLabel.setForeground( java.awt.Color(fgCol(1), fgCol(2), fgCol(3)) )
%                             jHash.put(jTicks(idx),jThisLabel);
%                         end
                        
                    end %if strcmpi(obj.LabelMode,'auto') || isempty(obj.CustomLabels.values)
                    
                    javaMethodEDT('setLabelTable',obj.JControl,jHash);
                    
                end %if obj.ShowTicks
                
            end %if obj.jSlider_IsConstructed
        end %function
        
        function onTooltipChanged(this)
            this.JControl.setToolTipText(this.Tooltip);
        end
    end % Protected methods
    
    %% Get/set methods
    methods
        function tf = get.jSlider_IsConstructed(this)
            tf = isvalid(this) && this.jSlider_IsConstructed;
        end
        
        % Callback
        function set.OnMotionCallback(obj,value)
            if isempty(value)
                obj.OnMotionCallback = function_handle.empty(0,1);
            else
                obj.OnMotionCallback = value;
            end
        end %function
        
        function set.Orientation(obj,value)
            obj.Orientation = value;
            obj.onResized();
        end
        
        function set.ShowTicks(obj,value)
            obj.ShowTicks = value;
            obj.redrawTicks();
        end
        
        function set.ShowLabels(obj,value)
            obj.ShowLabels = value;
            obj.redrawTicks();
        end
        
        function set.Focusable(obj, value)
            obj.JControl.setFocusable(value);
            obj.Focusable = value;
        end
        
        function set.MajorTickValues(obj, value)
            assert(isempty(value)||isnumeric(value)||strcmpi(value,'auto'));
            if isnumeric(value)
                value = sort(value);
            end
            obj.MajorTickValues = value;
            obj.redrawTicks();
        end
        
        
    end %methods
    
    %% Min Max validation
    methods (Access=protected)
        function [value,enfl] = validateMin(this,value)
            
            enfl = false;
            if this.jSlider_IsConstructed
                assert(isscalar(value)&&isnumeric(value)&&isfinite(value),'Min must be numeric scalar and finite');
                [value,enfl] = validateMin@extras.widgets.mixin.HasValueLimits(this,value);
            end
        end
        
        function [value,enfl] = validateMax(this,value)
            enfl = false;
            if this.jSlider_IsConstructed
                assert(isscalar(value)&&isnumeric(value)&&isfinite(value),'Max must be numeric scalar and finite');
                [value,enfl] = validateMax@extras.widgets.mixin.HasValueLimits(this,value);
            end
        end
    end
 end