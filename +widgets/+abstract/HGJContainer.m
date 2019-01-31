classdef HGJContainer <  extras.RequireGuiLayoutToolbox & uix.Container
% Container class for java GUI objects
% handles constructing graphics elements inside a figure or panel
%
% classes implementing javaObjects should derive from this class
% In their constructors they should initialize HGJContainer by passing the
% javaObject or string specifying java class to construct
%
% GUI elements are constructed using javacomponent() (Undocumented MATLAB
% function). The resulting component and container is stored in the
% properties:
%   HGJContainer.jComponent
%   HGJContainer.jContainer

    properties(SetAccess=private,GetAccess=protected)
        HGJcontainer_IsConstructed = false; %flag specifying if java container has been constructed
        jContainer %HG container constructed to hold javaObject
        jComponent % javaObject passed to / constructed by javacomponent(), when creating jContainer
    end
    
    %% internal use only
    properties(Access=private)
        HGJContainer_ParentSetListener %temporary listener, which looks for first time Parent is set
    end
    
    %% construct/delete
    methods
        
        function this = HGJContainer(javaObj)
        % Construct HGJContainer
        % Input:
        %   javaObj: char array specifying class to construct
        %            or javaObject constructed with javaObject or
        %            javaObjectEDT
            
            %% add listener for parent set
            this.HGJContainer_ParentSetListener = addlistener(this,'Parent','PostSet',@(~,~) this.constructGraphicsObject(javaObj));
            
            
        end
        
        function delete(this)
            try
                delete(this.HGJContainer_ParentSetListener)
            catch
            end
            try
                delete(this.jContainer)
            catch
            end
        end
    end
    
    methods (Access=protected)
        function constructGraphicsObject(this,javaObj)
        % Constructs HG container and creates GUI for javaObject.
        % uses javacomponent() [Undocumented MATLAB function]
        % Called when parent of this object is initially set.
        % Usage
        %  this.constructGraphicsObject(javaObj)
        %   javaObj: string specifying class name or object constructed via
        %            javaObject or javaObjectEDT
            if ~isvalid(this)
                return
            end
            
            if ~this.HGJcontainer_IsConstructed && ~isempty(this.Parent) && isvalid(this.Parent)
                
                org_units = this.Units;
                this.Units = 'pixels';
                org_pos = this.Position;
                [this.jComponent,this.jContainer] = javacomponent(javaObj,org_pos,this);
                this.Units = org_units;
                this.jContainer.Units = 'normalized';
                this.jContainer.Position = [0,0,1,1];
                
                %% setup container
                this.jContainer.HandleVisibility = 'callback';
                
            
                %% delete listener
                addlistener(this.jContainer,'ObjectBeingDestroyed',@(~,~) delete(this));

                %% set constructed flag
                this.HGJcontainer_IsConstructed = true;
                
                %% Delete parent listener
                try
                    delete(this.HGJContainer_ParentSetListener)
                catch
                end
            end
        end
    end
    
    %% Graphics related
    methods( Access = protected )
        function redraw(this)
        % resize and redraw container
            
            if ~this.HGJcontainer_IsConstructed
                return;
            end
            
            bounds = hgconvertunits( ancestor( this, 'figure' ), ...
                [0 0 1 1], 'normalized', 'pixels', this );
            padding = obj.Padding_;
            
            position = bounds-padding;
            
            uix.setPosition( this.jContainer, position, 'pixels' );
            
        end
        
        function addChild( this, child )
            %addChild  Add child
            %  c.addChild(d) adds the child d to the container c.'
            
            if this.HGJcontainer_IsConstructed
                warning('cannot add child once HGJContainer is constructed');
                return;
            end
            
            
            addChild@uix.Box( this, child );
        end
    end
end