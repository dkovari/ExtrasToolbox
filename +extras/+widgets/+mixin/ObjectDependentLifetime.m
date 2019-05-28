classdef ObjectDependentLifetime < handle
% Mixin class for linking the lifetime of this object to another object
%   Objects derived from this class will automaticalled be deleted whenever
%   one of the LifetimeLinkedObjects is destroyed.
%% Copyright 2019, Daniel T. Kovari, Emory University
%  All rights reserved.
    
    %% Props
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
    
    %% add listener objects
    methods(Access=protected)
        function linkObjectLifetime(this,LinkedObject)
        % Add destruction monitor for handles in LinkedObject
        % Input:
        %   LinkedObject: array of handles or cell with each element
        %   containing a valid handle
            
            %% Validate LinkedObject
            if iscell(LinkedObject)
                for n=1:numel(LinkedObject)
                    assert(isa(LinkedObject{n},'handle'),'LinkedObject must be handle object');
                    assert(isvalid(LinkedObject{n}),'LinkedObject must be valid (not yet deleted)');
                end
            else
                assert(all(isa(LinkedObject,'handle')),'LinkedObject must be handle object');
                assert(all(isvalid(LinkedObject)),'LinkedObject must be valid (not yet deleted)');
            end
            
            %% Set LifetimeLinkedObject & create destruction listeners
            for n=1:numel(this)
                if isempty(this(n).LifetimeLinkedObject)
                    if iscell(LinkedObject) %new is cell
                        this(n).LifetimeLinkedObject = LinkedObject;

                        for m=numel(LinkedObject):-1:1
                            this(n).LinkedObjectDestructionListener(m) = addlistener(LinkedObject{m},'ObjectBeingDestroyed',@(~,~) delete(this(n)));
                        end
                    else %new obj is not cell
                        this(n).LifetimeLinkedObject = LinkedObject;
                        this(n).LinkedObjectDestructionListener = addlistener(LinkedObject,'ObjectBeingDestroyed',@(~,~) delete(this(n))); 
                    end
                elseif iscell(this(n).LifetimeLinkedObject) %already contains items,of different type
                    if iscell(LinkedObject)
                        for m=1:numel(LinkedObject)
                            this(n).LifetimeLinkedObject{end+1} = LinkedObject{m};
                            this(n).LinkedObjectDestructionListener(end+1) = addlistener(LinkedObject{m},'ObjectBeingDestroyed',@(~,~) delete(this(n)));
                        end
                    else
                        for m=1:numel(LinkedObject)
                            this(n).LifetimeLinkedObject{end+1} = LinkedObject(m);
                            this(n).LinkedObjectDestructionListener(end+1) = addlistener(LinkedObject(m),'ObjectBeingDestroyed',@(~,~) delete(this(n)));
                        end
                    end
                else %Stored LO not cell
                    if iscell(LinkedObject)
                        new_obj = cell(numel(this(n).LifetimeLinkedObject),1);%+numel(LinkedObject),1);
                        for m=1:numel(this(n).LifetimeLinkedObject)
                            new_obj{m} = this(n).LifetimeLinkedObject(m);
                        end
                        new_obj = [new_obj;reshape(LinkedObject,[],1)];
                        this(n).LifetimeLinkedObject = new_obj;
                        for m=1:numel(LinkedObject)
                            this(n).LinkedObjectDestructionListener(end+1) = addlistener(LinkedObject{m},'ObjectBeingDestroyed',@(~,~) delete(this(n)));
                        end
                    else %sored list not cell
                        if isa(LinkedObject,class(this(n).LifetimeLinkedObject)) %same type
                            for m=1:numel(LinkedObject)
                                this(n).LifetimeLinkedObject(end+1) = LinkedObject(m);
                                this(n).LinkedObjectDestructionListener(end+1) = addlistener(LinkedObject(m),'ObjectBeingDestroyed',@(~,~) delete(this(n)));
                            end
                        else % not same type
                            new_obj = cell(numel(this(n).LifetimeLinkedObject),1);
                            for m=1:numel(this(n).LifetimeLinkedObject) %convert stored to cell
                                new_obj{m} = this(n).LifetimeLinkedObject(m);
                            end
                            if iscell(LinkedObject)
                                for m=1:numel(LinkedObject)
                                    new_obj{end+1} = LinkedObject{m};
                                    this(n).LinkedObjectDestructionListener(end+1) = addlistener(LinkedObject{m},'ObjectBeingDestroyed',@(~,~) delete(this(n)));
                                end
                            else
                                for m=1:numel(LinkedObject)
                                    new_obj(end+1) = LinkedObject(m);
                                    this(n).LinkedObjectDestructionListener(end+1) = addlistener(LinkedObject(m),'ObjectBeingDestroyed',@(~,~) delete(this(n)));
                                end
                            end
                            this(n).LifetimeLinkedObject = new_obj;       
                        end
                    end
                end
                    
            end
        end
    end
    
    
end