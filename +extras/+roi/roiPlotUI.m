classdef roiPlotUI < extras.GraphicsChild
% extras.roi.roiPlotUI
%=============================
% hobj = extras.roi.roiPlotUI(RoiManager)
% hobj = extras.roi.roiPlotUI(AxesParent,RoiManager)
% hobj = extras.roi.roiPlotUI(__,'Parent',AxesParent)
% hobj = extras.roi.roiPlotUI(... 'RoiManager',RoiManager)

    %% Set Protected
    properties (SetAccess=protected)
        Manager % roi manager
    end
    
    %% Internal Properties
    properties (Access=protected)
        ManagerDeleteListener
        
        SelectionListener;
        RoiListListener;
        ContextGeneratorsListener;
        
        RectList = extras.uirect.empty();
    end
    
    events
        EnteringUIEdit
        ExitingUIEdit
    end
    
    %% Create/Delete
    methods
        function this = roiPlotUI(varargin)
            %% Setup Parent Axes
            
            % initiate graphics parent related variables
            this@extras.GraphicsChild('axes');
            %look for parent specified in arguments
            varargin = this.CheckParentInput(varargin{:});
            
            %% Get Manager
            if isempty(varargin)
                error('ROI Manager was not specified');
            end
            found_manager = false;
            if isa(varargin{1},'extras.roi.roiManager')
                found_manager = true;
                this.Manager = varargin{1};
                varargin(1) = [];
            end
            
            if numel(varargin)>1
                ind = find(strcmpi('Manager',varargin));
                if numel(ind) > 1
                    error('Manager specified more than one time');
                end
                if found_manager && ~isempty(ind)
                    error('Manager specified more than one time');
                end
                
                this.Manager = varargin{ind+1};
                varargin(ind:ind+1) = [];
                found_manager = true;
            end
            
            assert(found_manager,'ROI Manager was not specified');
            this.ManagerDeleteListener = addlistener(this.Manager,'ObjectBeingDestroyed',@(~,~) delete(this));
            
            %% Update Rects
            this.UpdateList();
            
            %% Register self with manager
            if ~(this.Manager.RegisterROIPlotUI(this))
                warning('Manager already has associated PlotUI');
                delete(this)
                error('Could not create PlotUI');
            end
            
            %% Update Listeners
            this.SelectionListener = addlistener(this.Manager,'IndexSelected','PostSet',@(~,~) this.SelectionUpdate());
            this.RoiListListener = addlistener(this.Manager,'roiList','PostSet',@(~,~) this.UpdateList());
            
            this.ContextGeneratorsListener = addlistener(this.Manager,'roiListChanged',@(~,evt) this.GenerateContextMenus(evt));
        end
        
        function delete(this)
            %% Delete listeners
            delete(this.ManagerDeleteListener);
            delete(this.RoiListListener);
            delete(this.SelectionListener);
            
            delete(this.ContextGeneratorsListener);
            
            %% delete rects
            this.DeleteRect(this.RectList);
        end
    end
    
    %% Internal Usage
    methods(Access=protected)
        function GenerateContextMenus(this,roiChangeEvent)
            newInd = reshape(roiChangeEvent.AddedIndexInNew,1,[]);
            
            for id = newInd
                this.RectList(id).UIContextMenu = this.Manager.ContextGenerators(id).createContextMenu(ancestor(this.Parent,'figure'));
            end
            
        end
    end
    methods (Hidden)
        function DeleteRect(this,hRect)
            for n=1:numel(hRect)
                try
                delete(hRect(n).UserData.WindowListener);
                catch
                end
                try
                delete(hRect(n).UserData.DeleteListener);
                catch
                end
            end
            try
            delete(hRect);
            catch
            end
            %Clear any deleted rects from the list
            try
            if ~isempty(this.RectList)
                this.RectList(~isvalid(this.RectList))=[];
            end
            catch
            end
        end
        function UpdateList(this)
            if isempty(this.Manager.roiList)
                this.DeleteRect(this.RectList);
                return
            end
            
            %look for rois that don't have corresponding rectangles
            uuids = cell(1,numel(this.RectList));
            for n=1:numel(this.RectList)
                uuids{n} = this.RectList(n).UserData.UUID;
            end
            for n=1:numel(this.Manager.roiList)
                roi = this.Manager.roiList(n);
                if isvalid(roi) && ~ismember(roi.UUID,uuids)
                    %create new 
                    newRec = extras.uirect('Parent',this.Parent,...
                        'Position',roi.Window,...
                        'EdgeColor','r');
                    win_list = addlistener(roi,'Window','PostSet',@(~,~) set(newRec,'Position',roi.Window));
                    del_list = addlistener(roi,'ObjectBeingDestroyed',@(~,~) this.DeleteRect(newRec));
                    
                    newRec.UserData = struct('UUID',roi.UUID,'WindowListener',win_list,'DeleteListener',del_list);
                    
                    addlistener(newRec,'PositionChangedByUI',@(h,e) this.ResizeRect(roi.UUID,h));
                    
                    addlistener(newRec,'BeingDragged','PostSet',@(~,~) this.NotifyDrag(newRec));
                    addlistener(newRec,'BeingResized','PostSet',@(~,~) this.NotifyResize(newRec));
                    
                    this.RectList = [this.RectList,newRec];
                end
            end         
        end
        
        function NotifyDrag(this,h)
            if h.BeingDragged
                notify(this,'EnteringUIEdit');
            else
                notify(this,'ExitingUIEdit');
            end
        end
        
        function NotifyResize(this,h)
            if h.BeingResized
                notify(this,'EnteringUIEdit');
            else
                notify(this,'ExitingUIEdit');
            end
        end
                
        function ResizeRect(this,UUID,hRect)
            
            ind = find(this.Manager.roiList==UUID);
            this.Manager.IndexSelected = ind;
            this.Manager.roiList(ind).Window = hRect.Position;
        end
        
        function SelectionUpdate(this)
            %if nothing, reset all colors
            if isempty(this.Manager.IndexSelected)
                for n=1:numel(this.RectList)
                    this.RectList(n).EdgeColor = 'r';
                end
                return
            end
            
            uuids = cell(1,numel(this.RectList));
            for n=1:numel(this.RectList)
                uuids{n} = this.RectList(n).UserData.UUID;
            end
            
            ind = this.Manager.IndexSelected;
            %find ind and set color, reset all others
            [lia,lob] = ismember({this.Manager.roiList(ind).UUID},uuids);
            lob=reshape(lob(lia),1,[]);
            nlob = 1:numel(this.RectList);
            nlob(lob) = [];
            
            for n=lob
                this.RectList(n).EdgeColor = 'm';
            end
            for n=nlob
                this.RectList(n).EdgeColor = 'r';
            end

        end
    end
    
    %% Public Methods
    methods
        function roi = DrawROI(this)
            roi = this.Manager.CreateROI();
            
            figure(this.ParentFigure);
            newRec = extras.uirect('Parent',this.Parent,...
                        'EdgeColor','r');
            
            win_list = addlistener(roi,'Window','PostSet',@(~,~) set(newRec,'Position',roi.Window));
            del_list = addlistener(roi,'ObjectBeingDestroyed',@(~,~) this.DeleteRect(newRec));
            
            roi.Window = newRec.Position;

            newRec.UserData = struct('UUID',roi.UUID,'WindowListener',win_list,'DeleteListener',del_list);

            addlistener(newRec,'PositionChangedByUI',@(h,e) this.ResizeRect(roi.UUID,h));
            
            addlistener(newRec,'BeingDragged','PostSet',@(~,~) this.NotifyDrag(newRec));
            addlistener(newRec,'BeingResized','PostSet',@(~,~) this.NotifyResize(newRec));
            
            this.RectList = [this.RectList,newRec];
        end
    end

end