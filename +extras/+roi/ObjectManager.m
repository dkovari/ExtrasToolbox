classdef (Abstract) ObjectManager < handle
% extras.roi.ObjectManager: Abstract class for creating a managed list
% of objects of a certain type
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.

    properties(SetAccess=private,Hidden)
        ObjectClassName %char array specifying valid class name
    end
    methods (Access=protected)
        function changeObjectClassName(this,NEW_CLASS_NAME)
            %method for changing the type of objects stored
            assert(ischar(NEW_CLASS_NAME),'CLASS_NAME must be char array specifying valid class name');
            
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
        DeletableObjects; %list of objects which should be deleted upon destruction
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
                    delete(this(m).DeletableObjects);
                    delete(this(m).ObjectDeleteListeners);
                catch ME
                    disp(ME.getReport)
                end
            end
        end
    end
    
    %% Protected Access add/remove/clear
    methods (Access=protected)
        
        function addToDeleteList(this,obj)
            obj = unique(obj,'stable');
            for m=1:numel(this)
                this(m).DeletableObjects(~isvalid(this(m).DeletableObjects)) = [];
                this(m).DeletableObjects = union(this(m).DeletableObjects,obj,'stable');
            end
        end
        
        function removeFromDeleteList(this,obj)
            obj = unique(obj,'stable');
            for m=1:numel(this)
                this(m).DeletableObjects = setdiff(this(m).DeletableObjects,obj,'stable');
            end
        end
        
        function newObjs = addObjects(this,obj,DeleteObjs)
            %add Object to management list
            % optionally specify logical array indicating if objects should
            % be deleted when the ObjectManager is deleted, or when the
            % objects are removed.
            
            obj = reshape(obj,1,[]);
            if nargin < 3
                DeleteObjs = false(size(obj));
            else
                DeleteObjs = reshape(DeleteObjs,1,[]);
                DeleteObjs = logical(DeleteObjs);
                if isscalar(DeleteObjs)
                    DeleteObjs = repmat(DeleteObjs,size(obj));
                end
                assert(all(size(obj) == size(DeleteObjs)),'DeleteObjs must logical and same size as obj');
            end
            
            [obj,ia] = unique(obj,'stable');
            DeleteObjs = DeleteObjs(ia);
            
            %% validate object types
            for m = 1:numel(this)
                for n=1:numel(obj)
                    if ~isa(obj(n),this(m).ObjectClassName)
                        error('cannot add %s to ObjectManager with ObjectClassName: %s',class(obj(n)),this(m).ObjectClassName);
                    end    
                end
            end
            
            %% add
            %obj = unique(obj,'stable');
            newObjs = obj;
            for m=1:numel(this)
                [newObjs,ia] = setdiff(obj,this(m).ManagedObjects,'stable');
                newDeleteObjs = DeleteObjs(ia);
                for n=1:numel(newObjs)
                    if isa(newObjs(n),'handle') %object is handle type, add listener for delete
                        lst = addlistener(newObjs(n),'ObjectBeingDestroyed',@(h,~) this(m).Object_Being_Deleted(h));
                        this(m).ObjectDeleteListeners = [this(m).ObjectDeleteListeners,lst];
                    end
                    if newDeleteObjs(n)
                        this(m).DeletableObjects = union(this(m).DeletableObjects,newObjs(n),'stable');
                    end
                end
                %add to list
                this(m).ManagedObjects = [this(m).ManagedObjects,newObjs];
            end
            
        end
        
        function removeObjects(this,obj)
            % Remove the specified objects from the management list
            % If objects are listed in the DeletableObjects List then they
            % will be destroyed.
            
            obj = reshape(obj,1,[]);
            obj = unique(obj,'stable');
            for m=1:numel(this)
                this(m).ManagedObjects = setdiff(this(m).ManagedObjects,obj,'stable');
                
                %% delete objects on the delete list
                if ~isempty(this(m).DeletableObjects)
                    this(m).DeletableObjects(~isvalid(this(m).DeletableObjects)) = [];
                    del_list = intersect(this(m).DeletableObjects,obj);
                    delete(del_list);
                    this(m).DeletableObjects(~isvalid(this(m).DeletableObjects)) = [];
                end
                
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
                %% delete objects on delete list
                this(m).DeletableObjects(~isvalid(this(m).DeletableObjects)) = [];
                delete(this(m).DeletableObjects);
                this(m).DeletableObjects(~isvalid(this(m).DeletableObjects)) = [];
                
                this(m).ManagedObjects = eval([CLASS_NAME,'.empty()']);
                delete(this(m).ObjectDeleteListeners);
                this(m).ObjectDeleteListeners = event.listener.empty();
            end
        end
    end
    
    
end