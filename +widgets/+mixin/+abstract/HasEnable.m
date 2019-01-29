classdef (Abstract) HasEnable < handle & extras.widgets.mixin.AssignNV
    
    properties(SetObservable=true,AbortSet=true)
        Enable
    end
    %% Get/Set methods
    methods
        
        % Enable
        function set.Enable(obj,value)
            value = extras.validateOnOff(value,'ArrayInput',false);
            evt = struct('Property','Enable',...
                'OldValue',obj.Enable,...
                'NewValue',value);
            obj.Enable = value;
            obj.onEnableChanged();
        end
    end
    
    methods (Access=protected)
        function onEnableChanged(this)
            set(this.HasEnable_Objects(isprop(this.HasEnable_Objects,'Enable')),'Enable',this.Enable);
        end
    end
    
    properties(Access=protected)
        HasEnable_Objects = gobjects(0)
    end
    methods (Access=protected)
        function addHasEnableObjects(this,obj)
            this.HasEnable_Objects = union(this.HasEnable_Objects,obj);
        end
        function removeHasEnableObjects(this,obj)
            this.HasEnable_Objects = setdiff(this.HasEnable_Objects,obj);
        end
    end
    
end