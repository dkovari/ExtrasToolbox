classdef lutListUI < extras.GraphicsChild & extras.RequireWidgetsToolbox & extras.RequireGuiLayoutToolbox
% Generate ui for managing LUT used by roiObject3D
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.

    %% 
    properties (Access=protected)
        RoiObject = extras.roi.roiObject3D.empty();
    end
    
    %%
    properties(SetObservable,AbortSet)
        RoiManager = extras.roi.roiManager.empty();
    end
    methods 
        function set.RoiManager(this,val)
            if isempty(val)
               this.RoiManager = extras.roi.roiManager.empty();
               if ~isempty(this.hBtn_AddLUT) && isvalid(this.hBtn_AddLUT)
                   this.hBtn_AddLUT.Enable = 'off';
               end
               return;
            end
            
            assert(numel(val)==1 && isa(val,'extras.roi.roiManager'),'RoiManager must be a valid extras.roi.roiManager');
            this.RoiManager = val;
            if ~isempty(this.hBtn_AddLUT) && isvalid(this.hBtn_AddLUT)
               this.hBtn_AddLUT.Enable = 'on';
            end
            
        end
    end
    
    
    %% GUI Related
    properties(Access=private)
        OuterPanel
        hBtn_AddLUT
        hBtn_DeleteLUT
        jTab_lutList        
    end
    
    %% listeners
    properties(Access=private)
        LUTListListener
        LUTChangeListener
        RoiObjectDeleteObserver
    end

    %% ctor
    methods
        
        function this = lutListUI(varargin)
            % Create list of LUT for the associated ROI Object
            % lutListUI(roiObject)
            % lutListUI(parent,roiObject)
            % lutListUI(__,roiManager)
            % lutListUI('Name',value,...)
            %   'Parent'
            %   'RoiObject'
            %   'RoiManager'
            
            %% initiate graphics parent related variables
            this@extras.GraphicsChild(@() ...
                figure(...
                    'Name','LUT List',...
                    'NumberTitle','off',...
                    'ToolBar','none',...
                    'MenuBar','none',...
                    'HandleVisibility','Callback')...
                );
            
            %% look for RoiObject
            if isempty(varargin)
                error('roiObject was not specified');
            end
            
            found_roi = false;
            if ~isempty(varargin) && isa(varargin{1},'extras.roi.roiObject3D')
                
                RoiObject = varargin{1};
                varargin(1) = [];
                found_roi = true;
            elseif isgraphics(varargin{1}) && isa(varargin{2},'extras.roi.roiObject3D')
                RoiObject = varargin{2};
                varargin(2) = [];
                found_roi = true;
            else
                ind = find(strcmpi('RoiObject',varargin));
                if numel(ind) > 1
                    error('RoiObject specified more than one time');
                end
                if found_roi && ~isempty(ind)
                    error('RoiObject specified more than one time');
                end
                
                RoiObject = varargin{ind+1};
                varargin(ind:ind+1) = [];
                found_roi = true;
            end
            
            assert(found_roi,'RoiObject was not specified');

            
            %% Check if we already created a lutListUI for this object
            persistent uiList; %listing of all lutListUIs created
            if isempty(uiList)
                uiList = extras.roi.lutListUI.empty();
            else
                uiList(~isvalid(uiList)) = []; %get rid of deleted lutListUI from listing
            end
            
            % look for force-creation flag
            force_creation = false;
            if ~isempty(varargin)
                ind = find(strcmpi('ForceCreation',varargin));
                if ~isempty(ind)
                    force_creation = true;
                    varargin(ind) = [];
                end
            end
            
            [lia,lob] = ismember(RoiObject,[uiList.RoiObject]);
            
            if lia && ~force_creation %already created ui, return handle to that object
                delete(this);
                this = uiList(lob);
                return;
            end
            
            %% Set RoiObject
            this.RoiObject = RoiObject;
            % delete listener
            this.RoiObjectDeleteObserver = addlistener(this.RoiObject,'ObjectBeingDestroyed',@(~,~) delete(this));
            
            %% Setup Parent, create ui container
            %look for parent specified in arguments
            varargin = this.CheckParentInput(varargin{:});
            
            % change figure name
            if this.CreatedParent
                this.ParentFigure.Name = sprintf('LUT List for ROI: %s',this.RoiObject.UUID);
            end
            
            %% Look for optional manager
            found_roimanager = false;
            if ~isempty(varargin) && isa(varargin{1},'extras.roi.roiManager')
                this.RoiManager = varargin{1};
                varargin(1) = [];
                found_roimanager = true;
            end
            if ~isempty(varargin)
                ind = find(strcmpi('RoiManager',varargin));
                if numel(ind) > 1
                    error('RoiManager specified more than one time');
                end
                if found_roimanager && ~isempty(ind)
                    error('RoiManager specified more than one time');
                end
                
                this.RoiManager = varargin{ind+1};
                varargin(ind:ind+1) = [];
                found_roimanager = true;
            end
                
            
            %% Create GUI
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
            this.hBtn_AddLUT = uicontrol(sub_left,...
                'Style','pushbutton',...
                'String','Add',...
                'Callback',@(~,~) this.AddLUT(),...
                'TooltipString','Display list of ROIs which can be used to define LUTs',...
                'Interruptible','off');
            
            if isempty(this.RoiManager)
                this.hBtn_AddLUT.Enable = 'off';
            end
            
            this.hBtn_DeleteLUT = uicontrol(sub_left,...
                'Style','pushbutton',...
                'String','Delete',...
                'Callback',@(~,~) this.DeleteLUT(),...
                'ToolTipString','Remove selected LUT from the ROI''s List',...
                'Interruptible','off');
            
            %Create Table
            this.jTab_lutList = uiw.widget.Table('Parent',sub_right,...
                'Data',this.lut2cell(),...
                'SelectionMode','discontiguous',...
                'ColumnName',{' ','UUID','Calibrated','MinR','MaxR'},...
                'ColumnEditable',logical([0,0,0,1,1]),...
                'CellEditCallback',@(h,e) this.UIEditCallback(h,e),...
                'ColumnMinWidth',[25,60,25,40,40],...
                'ColumnPreferredWidth',[25,80,25,60,60],...
                'ColumnFormat',{'integer','char','numeric','numeric','numeric'});
            
            % Context menu
            cm = uicontextmenu(this.ParentFigure);
            
            uimenu(cm,...
                'Text','Delete Selected LUT',...
                'ForegroundColor','r',...
                'MenuSelectedFcn',@(~,~) this.DeleteLUT());
            
            uimenu(cm,...
                'Separator','on',...
                'Text','Display Selected LUT',...
                'MenuSelectedFcn',@(~,~) this.DisplayLUT());
            
            this.jTab_lutList.UIContextMenu = cm;
            
            
            %% Update Listeners
            this.LUTListListener = addlistener(this.RoiObject,'LUT','PostSet',@(~,~) this.UpdateList());
            this.LUTChangeListener = addlistener(this.RoiObject,'LUTChanged',@(~,~) this.UpdateList());
            
        end
        
    end
    
    %% dtor
    methods
        function delete(this)
            delete(this.LUTListListener)
            delete(this.LUTChangeListener)
            delete(this.RoiObjectDeleteObserver);
            
        end
    end
    
    %% internal helper methods
    methods(Access=private)
        function UpdateList(this)
            this.jTab_lutList.Data = this.lut2cell();
        end
        
        function DisplayLUT(this)
            sel_ind = this.jTab_lutList.SelectedRows;
            
            if isempty(sel_ind)
                return;
            end
            LUT = this.RoiObject.LUT(sel_ind);
            extras.roi.lutViewer(LUT);
        end
        
        function AddLUT(this)
            %GUI tool for adding LUT based on ROIs listed with an
            %RoiManager
            
            if isempty(this.RoiManager) || ~isvalid(this.RoiManager)
                error('Cannot call AddLUT() because there is valid RoiManager associated with the lutListUI');
            end
            
            dat = cell(numel(this.RoiManager.roiList),2);
            for n=1:numel(this.RoiManager.roiList)
                dat{n,1} = n;
                dat{n,2} = this.RoiManager.roiList.UUID;
            end
            dat = cell2table(dat,'VariableNames',{'Index','UUID'});
            
            d = uiw.dialog.TableSelection(...
                'Title','Select ROI to use for LUT',...
                'MultiSelect',true,...
                'DataTable',dat);
            [out, action] = d.waitForOutput(); % wait for menu to return

            if strcmpi(action,'cancel')
                return;
            end
            
            %check if user selected roi which are already associated with
            %LUT
            if any(ismember({this.RoiManager.roiList(out).UUID},{this.RoiObject.LUT.UUID}))
                resp = questdlg(sprintf('One the selected ROIs are already associated with an LUT.\nDo you want to create an additional LUT?'),'Duplicate LUT','Yes','Cancel','Yes');
                if ~strcmpi(resp,'Yes')
                    return;
                end
            end
            
            this.RoiObject.addLUT(extras.roi.LUTobject(this.RoiManager.roiList(out))); %create LUT object referenced to selected roi

        end
        
        function DeleteLUT(this)
            % UI tool to delete selected roi
            
            sel_ind = this.jTab_lutList.SelectedRows;
            
            if isempty(sel_ind)
                return;
            end
            
            LUT = this.RoiObject.LUT(sel_ind);
            this.RoiObject.removeLUT(LUT);
        end
        
        
        function c = lut2cell(this)
            c = cell(numel(this.RoiObject.LUT),5);
            for n=1:numel(this.RoiObject.LUT)
                c{n,1} = n;
                c{n,2} = this.RoiObject.LUT(n).UUID;
                c{n,3} = this.RoiObject.LUT(n).IsCalibrated;
                c{n,4} = this.RoiObject.LUT(n).MinR;
                c{n,5} = this.RoiObject.LUT(n).MaxR;
            end
        end
        
        function UIEditCallback(this,~,evt)  
            LUT_ind = evt.Indices(1);
            switch(evt.Indices(2))
                case 4 % MinR
                    this.RoiObject.LUT(LUT_ind).MinR = this.jTab_lutList.Data{LUT_ind,4};
                case 5 % MaxR
                    this.RoiObject.LUT(LUT_ind).MaxR = this.jTab_lutList.Data{LUT_ind,5};
            end
            this.UpdateList();
        end
    end
    
end