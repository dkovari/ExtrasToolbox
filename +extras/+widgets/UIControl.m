classdef UIControl < extras.RequireGuiLayoutToolbox &...
        extras.RequireWidgetsToolbox & ...
        extras.widgets.LabelPanel & ...
        extras.widgets.mixin.HasCallback & ...
        extras.widgets.mixin.AssignNV & ...
        extras.widgets.mixin.HasTooltip
    % Wrapper around standard uicontrol, provide look & feel that matches
    % other extras.widgets
    
    
    %% forwarded properties
    properties (SetObservable, AbortSet)
        Style = 'pushbutton'
        Value = 0
        Max (1,1) double = 1;
        Min (1,1) double = 0;
        SliderStep double = [0.01,0.1];
        ListboxTop (1,1) double = 1;
        String = '';
        CData
    end
    methods %get/set
        function set.Style(this,value)
            value = validatestring(value,{'pushbutton','togglebutton','checkbox','radiobutton','edit','text','slider','listbox','popupmenu'});
            this.Style = value;
            try
                set(this.control,'Style',value);
            catch
            end
        end
        function set.Value(this,value)
            if ~isempty(this.control)&& isvalid(this.control)
                try
                    this.control.Value = value;
                catch ME
                    this.Value = this.control.Value;
                    error('Could not set Value:\n%s',ME.getReport);
                end
                this.Value = this.control.Value;
            else
                this.Value = value;
            end
        end
        function set.Max(this,value)
            if ~isempty(this.control)&& isvalid(this.control)
                try
                    this.control.Max = value;
                catch ME
                    this.Max = this.control.Max;
                    error('Could not set Max:\n%s',ME.getReport);
                end
                this.Max = this.control.Max;
            else
                this.Max = value;
            end
        end
        function set.Min(this,value)
            if ~isempty(this.control)&& isvalid(this.control)
                try
                    this.control.Min = value;
                catch ME
                    this.Min = this.control.Min;
                    error('Could not set Min:\n%s',ME.getReport);
                end
                this.Min = this.control.Min;
            else
                this.Min = value;
            end
        end
        
        function set.SliderStep(this,value)
            assert(numel(value)==2,'SliderStep must two elements');
            if ~isempty(this.control)&& isvalid(this.control)
                try
                    this.control.SliderStep = value;
                catch ME
                    this.SliderStep = this.control.SliderStep;
                    error('Could not set SliderStep:\n%s',ME.getReport);
                end
                this.SliderStep = this.control.SliderStep;
            else
                this.SliderStep = value;
            end
        end
        
        function set.ListboxTop(this,value)
            if ~isempty(this.control)&& isvalid(this.control)
                try
                    this.control.ListboxTop = value;
                catch ME
                    this.ListboxTop = this.control.ListboxTop;
                    error('Could not set ListboxTop:\n%s',ME.getReport);
                end
                this.ListboxTop = this.control.ListboxTop;
            else
                this.ListboxTop = value;
            end
        end
        
        function set.String(this,value)
            if isstring(value)
                value = char(value);
            elseif isnumeric(value)
                value = num2str(value);
            elseif ~ischar(value)&&~iscellstr(value)
                value = char(value);
            end
            if ~isempty(this.control)&& isvalid(this.control)
                try
                    this.control.String = value;
                catch ME
                    this.String = this.control.String;
                    error('Could not set String:\n%s',ME.getReport);
                end
                this.String = this.control.String;
            else
                this.String = value;
            end
        end
        
        function set.CData(this,value)
            if ~isempty(this.control)&& isvalid(this.control)
                try
                    this.control.CData = value;
                catch ME
                    this.CData = this.control.CData;
                    error('Could not set Max:\n%s',ME.getReport);
                end
                this.CData = this.control.CData;
            else
                this.CData = value;
            end
        end
        
    end
    
    %%
    properties (SetAccess=protected)
        control = gobjects(0);
    end
    
    %% Internal use only
    properties(Access=private)
        parentlistener
        parentFirstSet = false;
    end
    
    %% constructor
    methods
        function this = UIControl(varargin)
            %create UIControl
            % hCtrl = UIControl(Name,Value)
            
            %% listener for first time parent is set
            this.parentlistener = addlistener(this,'Parent','PostSet',@(~,~) this.setparentcallback);
            
            %% set public args
            this.setPublicProperties(varargin{:});
        end
        
        function delete(this)
            try
                delete(this.control);
            catch
            end
        end
    end
    
    
    methods(Access=private)
        function setparentcallback(this)
            if this.parentFirstSet
                return;
            end
            
            %% create control
            this.control = uicontrol('Parent',this,...
                'HandleVisibility','callback',...
                'Callback',@(h,e) this.callCallback(e));
            
            %% set control props
            try
                this.control.Style = this.Style;
            catch ME
                warning('Could not set Style:\n%s\nUsing default Style.',ME.getReport);
                this.Style = this.control.Style;
            end
            
            try
                this.control.Value = this.Value;
            catch ME
                warning('Could not set Value:\n%s\nUsing default Value.',ME.getReport);
                this.Value = this.control.Value;
            end
            
            try
                this.control.Max = this.Max;
            catch ME
                warning('Could not set Max:\n%s\nUsing default Max.',ME.getReport);
                this.Max = this.control.Max;
            end
            
            try
                this.control.Min = this.Min;
            catch ME
                warning('Could not set Max:\n%s\nUsing default Max.',ME.getReport);
                this.Min = this.control.Min;
            end
            
            try
                this.control.SliderStep = this.SliderStep;
            catch ME
                warning('Could not set SliderStep:\n%s\nUsing default SliderStep.',ME.getReport);
                this.SliderStep = this.control.SliderStep;
            end
            
            try
                this.control.ListboxTop = this.ListboxTop;
            catch ME
                warning('Could not set ListboxTop:\n%s\nUsing default ListboxTop.',ME.getReport);
                this.ListboxTop = this.control.ListboxTop;
            end
            
            try
                this.control.String = this.String;
            catch ME
                warning('Could not set String:\n%s\nUsing default String.',ME.getReport);
                this.String = this.control.String;
            end
            
            try
                this.control.CData = this.CData;
            catch ME
                warning('Could not set CData:\n%s\nUsing default CData.',ME.getReport);
                this.CData = this.control.CData;
            end
            
            %% get rid of parent listener
            try
               delete(this.parentlistener)
            catch
            end
            %% set flag
            this.parentFirstSet = true;
        end
    end
    
    %% Override
    methods (Access=protected)
        function onTooltipChanged(this)
            
            if ~this.parentFirstSet
                return;
            end
            
            try
                set(this.hLabel,'TooltipString',this.Tooltip);
            catch
                try
                    set(this.hLabel,'Tooltip',this.Tooltip);
                catch
                end
            end
            
            try
                set(this.control,'TooltipString',this.Tooltip);
            catch
                try
                    set(this.control,'Tooltip',this.Tooltip);
                catch
                end
            end
            
        end
    end
    
end