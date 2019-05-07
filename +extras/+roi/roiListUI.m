classdef roiListUI < extras.GraphicsChild & extras.RequireWidgetsToolbox & extras.RequireGuiLayoutToolbox
%UI menu for editing ROI List
    
    properties(SetAccess=protected)
        Manager;
    end
    
    properties(Access=protected)
       ManagerDeleteObserver;

       OuterPanel
       hBtn_AddTrack;
       hBtn_DeleteTrack;
       jTab_roiList;
       
       SelectionListener;
       RoiListListener;
       ROIChangeListener;
       
       DefaultContextMenu
    end
    
    methods
        function this = roiListUI(varargin)
        % Create UI with list of ROI
        % roiListUI(roiManager)
        % roiListUI(Parent,__)
        % roiListUI(__,'Parent',Parent)
            
            %% Setup Parent
            
            %initiate graphics parent related variables
            this@extras.GraphicsChild(@() ...
                figure(...
                    'Name','ROI Manager',...
                    'NumberTitle','off',...
                    'ToolBar','none',...
                    'MenuBar','none',...
                    'HandleVisibility','Callback')...
                );
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
            this.ManagerDeleteObserver = addlistener(this.Manager,'ObjectBeingDestroyed',@(~,~) delete(this));

            %% Build GUI
            %Create Outer panel
            this.OuterPanel = uix.Panel('Parent',this.Parent',...
                'BorderType','none',...
                'Padding',2);
            
            %horizontal flex box for buttons (column on left) and table
            %(panel on right)
            hb1 = uix.HBox('Parent',this.OuterPanel,...
                'Spacing',3);
            
            sub_left = uix.VButtonBox('Parent',hb1);
            sub_right = uix.ScrollingPanel('Parent',hb1);
            
            hb1.Widths = [50,-1]; %set horizontal flex column widths
            
            % Add buttons
            this.hBtn_AddTrack = uicontrol(sub_left,...
                'Style','pushbutton',...
                'String','Add',...
                'Callback',@(~,~) this.AddTrack(),...
                'Interruptible','off');
            
            this.hBtn_DeleteTrack = uicontrol(sub_left,...
                'Style','pushbutton',...
                'String','Delete',...
                'Callback',@(~,~) this.DeleteTrack(),...
                'Interruptible','off');
            
            %Create Table
            this.jTab_roiList = uiw.widget.Table('Parent',sub_right,...
                'Data',this.roiObj2cell(this.Manager.roiList),...
                'SelectionMode','discontiguous',...
                'CellSelectionCallback',@(~,~) this.UISelectionCallback(),...
                'ColumnName',{' ','X','Y','Width','Height'},...
                'ColumnEditable',logical([0,1,1,1,1]),...
                'CellEditCallback',@(h,e) this.UIEditCallback(h,e),...
                'ColumnMinWidth',[25,40,40,40,40],...
                'ColumnPreferredWidth',[25,60,60,60,60],...
                'ColumnFormat',{'integer','numeric','numeric','numeric','numeric'});
            
            %% Create Default Context Menu
            this.DefaultContextMenu =uicontextmenu(ancestor(this.Parent,'figure'));
            
            uimenu(this.DefaultContextMenu,...
                'Text','Delete Selected',...
                'ForegroundColor','r',...
                'MenuSelectedFcn',@(~,~) this.DeleteTrack());
            
            
                
           %% Update Listeners
            this.SelectionListener = addlistener(this.Manager,'IndexSelected','PostSet',@(~,~) this.UpdateSelected());
            this.RoiListListener = addlistener(this.Manager,'roiList','PostSet',@(~,~) this.UpdateList());
            
            this.ROIChangeListener = addlistener(this.Manager,'roiValueChanged',@(~,~) this.UpdateList());
            
            %% Update the table
            this.UpdateList();
           
        end
        
        function delete(this)
            %% Delete Listeners
            delete(this.ROIChangeListener);
            delete(this.RoiListListener);
            delete(this.SelectionListener);
            delete(this.ManagerDeleteObserver);
            
            %% delete gui elements
            try
            delete(this.jTab_roiList);
            catch
            end
            try
                %delete(this.jTab_roiList);
                delete(this.OuterPanel);
                if this.MadeFig
                   delete(this.Parent);
                end
            catch
            end

        end
    end
    
    %% Button Callbacks
    methods(Hidden)
        function AddTrack(this)
            this.Manager.AddROI();
        end
        function DeleteTrack(this)
            
            this.Manager.RemoveROI(this.Manager.roiList(this.Manager.IndexSelected));

        end
        function UISelectionCallback(this)
            %'here'
            this.Manager.IndexSelected = this.jTab_roiList.SelectedRows;
        end
        function UIEditCallback(this,~,evt)            
            switch(evt.Indices(2))
                case 2 %X
                    this.Manager.roiList(evt.Indices(1)).Window_X1 = this.jTab_roiList.Data{evt.Indices(1),evt.Indices(2)};
                case 3 %Y
                    this.Manager.roiList(evt.Indices(1)).Window_Y1 = this.jTab_roiList.Data{evt.Indices(1),evt.Indices(2)};
                case 4 % Width
                    this.Manager.roiList(evt.Indices(1)).Window_Width = this.jTab_roiList.Data{evt.Indices(1),evt.Indices(2)};
                case 5 % Height
                    this.Manager.roiList(evt.Indices(1)).Window_Height = this.jTab_roiList.Data{evt.Indices(1),evt.Indices(2)};
            end
            this.UpdateList();
        end
        function UpdateList(this)
            this.jTab_roiList.Data = this.roiObj2cell(this.Manager.roiList);
            
        end
        function UpdateSelected(this)
            this.jTab_roiList.SelectedRows = this.Manager.IndexSelected;
            
            %% Set Context menu
            
            if isempty(this.Manager.IndexSelected) %no context menu
                this.jTab_roiList.UIContextMenu = gobjects(0);
            elseif numel(this.Manager.IndexSelected)==1 %context menu for selected roi
                this.jTab_roiList.UIContextMenu = this.Manager.ContextGenerators(this.Manager.IndexSelected).createContextMenu(ancestor(this.Parent,'figure'));
            else %multiple, use default
                this.jTab_roiList.UIContextMenu = this.DefaultContextMenu;
            end
                
        end
    end
    
    %% static Helpers
    methods (Static)
        function c = roiObj2cell(roi)
            c = cell(numel(roi),5);
            for n=1:numel(roi)
                c{n,1} = n;
                c{n,2} = roi(n).Window_X1;
                c{n,3} = roi(n).Window_Y1;
                c{n,4} = roi(n).Window_Width;
                c{n,5} = roi(n).Window_Height;
            end
            
        end
    end
    
    
end