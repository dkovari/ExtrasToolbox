classdef ContextGenerator < handle & matlab.mixin.SetGet & matlab.mixin.Heterogeneous
% extras.roi.ContextGenerator Class for generating uicontextmenus linked to 
% ROI objects.

    properties (SetAccess=protected)
        RoiObject extras.roi.roiObject = extras.roi.roiObject.empty();
        ContextMenus = gobjects(0);
    end
    properties(Access=protected)
        RoiObjectDeleteListener
    end
    
    %% create/delete
    methods
        function this = ContextGenerator(RoiObject)
        % ContextGenerator(RoiObject)
        %
        % Input:
        %   RoiObject: class handle to roiObject which is linked to the
        %   context menu;
            %% Validate RoiObject
            if nargin<1
                delete(this);
                %this = extras.roi.ContextGenerator.empty();
                return;
            end
            if isempty(RoiObject)
                delete(this);
                this = extras.roi.ContextGenerator.empty();
                return;
            end
            assert(isvalid(RoiObject)&&isa(RoiObject,'extras.roi.roiObject'),'RoiObject must be valid extras.roi.roiObject');
                
            %% handle array
            sz = num2cell(size(RoiObject));

            for n=numel(RoiObject):-1:1
                this(n).RoiObject = RoiObject;
                this(n).RoiObjectDeleteListener = addlistener(RoiObject(n),'ObjectBeingDestroyed',@(~,~) delete(this(n)));
            end
            reshape(this,sz{:});

        end
        
        function delete(this)
            for n=1:numel(this)
                try
                    delete(this(n).RoiObjectDeleteListener);
                catch
                end
                try
                    delete(this(n).ContextMenus);
                catch
                end
            end
        end
    end
    
    %% Public Methods (non-overridable)
    methods (Sealed)
        function out = createContextMenu(this,hFig)
            
            %% remove invalid figure handles
            hFig(~isgraphics(hFig)) = [];
            if isempty(hFig)
                return;
            end
            assert(all(strcmpi('figure',{hFig.Type})),'Specified parent handles must be figures');
            
            out = gobjects(numel(this),numel(hFig));
            %% Loop over all this and all hFig
            for n=1:numel(this)
                for m=1:numel(hFig)
                    %% check if we need to create context menu
                    if ~isempty(this(n).ContextMenus)
                        pars = [this(n).ContextMenus.Parent]; %get a list of all context menu parents
                        fd = find(hFig(m)==pars);
                        if ~isempty(fd) % already created for that figure, return the context menu for hFig(m)
                            out(n,m) = this(n).ContextMenus(fd(1));
                            continue;
                        end
                    end
                    %% Create context menu
                    cm = this(n).internal_createContextMenu(hFig(m));
                    
                    %% add new cm for hFig(m) to list of contextmenus
                    this(n).ContextMenus = [this(n).ContextMenus,cm];
                    
                    %% set out
                    out(n,m) = cm;
                end
            end
            
            
            
        end
    end
    
    %% internal use only
    methods(Access=protected)
        function clearInvalidContextMenus(this,hCM)
            for n=1:numel(this)
                if nargin>1
                    this(n).ContextMenus(this(n).ContextMenus == hCM) = [];
                end
                this(n).ContextMenus(~isvalid(this(n).ContextMenus)) = [];
            end
        end
    end
    
    %% Overloadable delete roiObject callback
    methods (Static,Access=protected)
        function deleteRoiCallback(hROI)
            % redefine this method to change what happens when a user
            % selects delete from context menu
            delete(hROI);
        end
    end
    methods(Access=protected)
        function cm = internal_createContextMenu(this,hFig)
        %redefine this method to change items that are included in the
        %context menu
            cm = uicontextmenu(hFig);
            addlistener(cm,'ObjectBeingDestroyed',@(h,~) this.clearInvalidContextMenus(h));

            % Title
            uimenu(cm,'Text',sprintf('UUID: %s',this.RoiObject.UUID),'Enable','off');
            % delete menu
            uimenu(cm,'Text','Delete ROI',...
                'Separator','on',...
                'ForegroundColor','r',...
                'MenuSelectedFcn',@(~,~) this.deleteRoiCallback(this.RoiObject));
        end
    end
end

