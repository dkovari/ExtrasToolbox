classdef (Abstract) CheckboxControl < extras.RequireWidgetsToolbox & ...
        uiw.abstract.WidgetContainer & ...
        uiw.mixin.HasCallback
% Checkbox control, looks similar to controls included in the widgets
% toolbox

    properties (AbortSet,SetObservable=true)
        Value = false% (1,1) logical = false; % t/f is checkbox is set
        %Enable - > inherited from HasContainer
        Tooltip = '';
        TooltipString = ''; %legacy alias of Tooltip
    end
    methods
        function set.TooltipString(obj,val)
            obj.Tooltip = val;
            obj.TooltipString = val;
        end
    end
    
    properties (Access=protected)
        hCheckbox matlab.graphics.Graphics % the checkbox control
    end
    
    %% constructor
    methods
        function obj = CheckboxControl()
            % Call superclass constructors
            obj@uiw.abstract.WidgetContainer();
            
            obj.hCheckbox = uicontrol(...
                'Parent',obj.hBasePanel,...
                'Style','checkbox',...
                'Value',obj.Value,...
                'String','',...
                'SelectionHighlight','off',...
                'Selected','off',...
                'Tooltip',obj.Tooltip,...
                'Units','normalized',...
                'Position',[0 0 1 1],...
                'Callback', @(h,e)obj.onEdit() );
        end
        
        function delete(obj)
            try
                delete(obj.hCheckbox);
            catch
            end
        end
    end
    
    
    %% Callbacks
    methods (Access=protected)
        function onEdit(obj)
            %triggered on text changes
            
            % Ensure the construction is complete
            if obj.IsConstructed
                old_value = obj.Value;
                obj.Value = obj.hCheckbox.Value;
                evt = struct('Source', obj, ...
                            'Interaction', 'Edit', ...
                            'OldValue', old_value, ...
                            'NewValue', obj.Value);
                obj.callCallback(evt);
            end
            
        end
        
        function updateUI(obj)
            obj.hCheckbox.Value = obj.Value;
            obj.redraw();
        end
        
        function onStyleChanged(obj)
            if obj.IsConstructed
                onStyleChanged@uiw.abstract.WidgetContainer(obj);
                try
                    obj.hCheckbox.Tooltip = obj.Tooltip;
                catch
                    obj.hCheckbox.TooltipString = obj.Tooltip;
                end
            end
        end
        
        function onEnableChanged(obj,~)
            if obj.IsConstructed
                %onEnableChanged@uiw.abstract.WidgetContainer(obj);
                obj.hCheckbox.Enable = obj.Enable;
            end
        end
    end
    
    %% get/set
    methods
        function set.Value(obj,val)
            if ischar(val)
                if strcmpi(val,'on')
                    val = true;
                elseif strcmpi(val,'off')
                    val = false;
                else
                    error('Value should be logical scalar or ''on''/''off''');
                end
            end
            assert(isscalar(val),'Value must be a scalar and convertible to logical');
            val = logical(val);
            obj.Value = val;
            
            obj.updateUI();
        end
        
        function set.Tooltip(obj,val)
            assert(isempty(val)||ischar(val),'Tooltip must be char');
            obj.Tooltip = val;
            obj.TooltipString = obj.Tooltip;
            obj.onStyleChanged();
        end
    end
end