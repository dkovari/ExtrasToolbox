classdef (Abstract) ObjectManager < handle
    % extras.roi.ObjectManager: Abstract class for creating a managed list
    % of objects of a certain type
    
    properties(SetAccess=private,Hidden)
        ObjectClassName %char array specifying valid class name
    end
    methods (Access=protected)
        function changeObjectClassName(this,NEW_CLASS_NAME)
            %method for changing the type of objects stored
            assert(ischar(CLASS_NAME),'CLASS_NAME must be char array specifying valid class name');
            
            if ~isempty(this.ManagedObjects) && ~isa(this.ManagedObjects(1),NEW_CLASS_NAME)
                error('Cannot change object type from %s to %s; there are currently %d objects stored in the managed list',this.ObjectClassName,NEW_CLASS_NAME,numel(this.ManagedObjects));
            end
            
            try
                this.ManagedObjects = eval([NEW_CLASS_NAME,'.empty()']);
            catch ME
                disp(ME.getReport);
                error('Could not create create empty array of: %s',NEW_CLASS_NAME);
            end
            
            this.ObjectClassName = NEW_CLASS_NAME;
        end
    end
    
    properties (SetAccess=private,SetObservable=true,AbortSet=true,Hidden)
        ManagedObjects; %list of objects
    end
    properties(Access=private)
        ObjectDeleteListeners = event.listener.empty();
    end
    methods(Access=private)
        function Object_Being_Deleted(this,hObj)
            this.ManagedObjects(this.ManagedObjects==hObj) = [];
            this.ObjectDeleteListeners(~isvalid(this.ObjectDeleteListeners)) = [];
        end
    end
    
    %% ctro/dtor
    methods
        function this = ObjectManager(CLASS_NAME)
            assert(ischar(CLASS_NAME),'CLASS_NAME must be char array specifying valid class name');
            this.ObjectClassName = CLASS_NAME;
            try
                this.ManagedObjects = eval([CLASS_NAME,'.empty()']);
            catch ME
                disp(ME.getReport);
                error('Could not create create empty array of: %s',CLASS_NAME);
            end
        end
        function delete(this)
            for m=1:numel(this)
                %% delete object listeners
                try
                    delete(this(m).ObjectDeleteListeners);
                catch ME
                    disp(ME.getReport)
                end
            end
        end
    end
    
    %% Protected Access add/remove/clear
    methods (Access=protected)
        function newObjs = addObjects(this,obj)
            obj = reshape(obj,1,[]);
            %% validate object types
            for m = 1:numel(this)
                for n=1:numel(obj)
                    if ~isa(obj(n),this(m).ObjectClassName)
                        error('cannot add %s to ObjectManager with ObjectClassName: %s',class(obj(n)),this(m).ObjectClassName);
                    end
                end
            end
            
            %% add
            obj = unique(obj);
            newObjs = obj;
            for m=1:numel(this)
                newObjs = setdiff(obj,this(m).ManagedObjects);
                for n=1:numel(newObjs)
                    if isa(newObjs(n),'handle') %object is handle type, add listener for delete
                        lst = addlistener(newObjs(n),'ObjectBeingDestroyed',@(h,~) this(m).Object_Being_Deleted(h));
                        this(m).ObjectDeleteListeners = [this(m).ObjectDeleteListeners,lst];
                    end
                end
                %add to list
                this(m).ManagedObjects = [this(m).ManagedObjects,newObjs];
            end
            
        end
        
        function removeObjects(this,obj)
            obj = reshape(obj,1,[]);
            for m=1:numel(this)
                this(m).ManagedObjects = setdiff(this(m).ManagedObjects,obj);
                
                %% delete listeners
                for n=1:numel(obj)
                    for k=1:numel(this(m).ObjectDeleteListeners)
                        if isvalid(this(m).ObjectDeleteListeners(k))
                            if any(this(m).ObjectDeleteListeners(k).Source{:} == obj(n))
                                delete(this(m).ObjectDeleteListeners(k));
                            end
                        end
                    end
                end
                this(m).ObjectDeleteListeners(~isvalid(this(m).ObjectDeleteListeners)) = [];
            end
        end
        
        function clearObjects(this)
            for m=1:numel(this)
                this(m).ManagedObjects = eval([CLASS_NAME,'.empty()']);
                delete(this(m).ObjectDeleteListeners);
                this(m).ObjectDeleteListeners = event.listener.empty();
            end
        end
    end
    
    
end