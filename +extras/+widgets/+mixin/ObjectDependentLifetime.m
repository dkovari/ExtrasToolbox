classdef ObjectDependentLifetime < handle
    % Mixin class for linking the lifetime of this object to another object
    
    
    properties (Access=protected)
        LifetimeLinkedObject %handle to linked object
        
        LinkedObjectDestructionListener
        
    end
    
    %% constructor
    methods
        function this = ObjectDependentLifetime(LinkedObject,MultipleFlag)
            %Construct ObjectDependentLifetime object
            % ObjectDependentLifetime(LinkedObject)
            %   LinkedObject can be an array of (optionally heterogeneous)
            %   handle objects
            %   or a cellarray of different-typed handle objects
            %
            % When the linked objects are deleted, this object will also be
            % deleted
            %
            % Deleting this object has no effect on the linked object
            %
            % If is specified and MultipleFlag=='ConstructMultiple' 
            % then a distinct ObjectDependentLifetime object is created for
            % each LinkedObject
            
            if isempty(LinkedObject)
                delete(this);
                this = extras.widgets.mixin.ObjectDependentLifetime.empty();
                return;
            end
            
            if nargin>1 && strcmpi(MultipleFlag,'ConstructMultiple') %multi construction
                for n = numel(LinkedObject):-1:1
                    if iscell(LinkedObject)
                        assert(isa(LinkedObject{n},'handle'),'LinkedObject must be handle object');
                        assert(isvalid(LinkedObject{n}),'LinkedObject must be valid (not yet deleted)');
                        
                        this(n).LifetimeLinkedObject = LinkedObject{n};
                        this(n).LinkedObjectDestructionListener = addlistener(LinkedObject{n},'ObjectBeingDestroyed',@(~,~) delete(this(n)));
                    else
                        assert(isa(LinkedObject(n),'handle'),'LinkedObject must be handle object');
                        assert(isvalid(LinkedObject(n)),'LinkedObject must be valid (not yet deleted)');
                        
                        this(n).LifetimeLinkedObject = LinkedObject(n);
                        this(n).LinkedObjectDestructionListener = addlistener(LinkedObject(n),'ObjectBeingDestroyed',@(~,~) delete(this(n)));
                    end
                end
                this = reshape(this,size(LinkedObject));
            else %single construction with multiple lifetime dependences
            
                if iscell(LinkedObject)
                    for n=1:numel(LinkedObject)
                        assert(isa(LinkedObject{n},'handle'),'LinkedObject must be handle object');
                        assert(isvalid(LinkedObject{n}),'LinkedObject must be valid (not yet deleted)');
                    end

                    this.LifetimeLinkedObject = LinkedObject;

                    for n=numel(LinkedObject):-1:1
                        this.LinkedObjectDestructionListener(n) = addlistener(LinkedObject{n},'ObjectBeingDestroyed',@(~,~) delete(this));
                    end
                else    
                    assert(all(isa(LinkedObject,'handle')),'LinkedObject must be handle object');
                    assert(all(isvalid(LinkedObject)),'LinkedObject must be valid (not yet deleted)');

                    this.LifetimeLinkedObject = LinkedObject;
                    this.LinkedObjectDestructionListener = addlistener(LinkedObject,'ObjectBeingDestroyed',@(~,~) delete(this));
                end
            end
                        
        end
    end
    
    %%destructor
    methods
        function delete(this)
            delete(this.LinkedObjectDestructionListener);
        end
    end
    
    
end