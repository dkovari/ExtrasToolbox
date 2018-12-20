classdef roiManager < handle
% extras.roi.roiManager

    properties (SetObservable=true,AbortSet=true)
        
        IndexSelected = [];
    end
    
    properties (SetAccess=protected,SetObservable=true,AbortSet=true)
        roiList = extras.roi.roiObject.empty
    end
    
    events
        roiValueChanged;
        StartingROIAdd;
        EndingROIAdd;
    end
    
    %% Internal properties
    properties(Access=protected)
        DeleteListeners;
        PropertyListeners;
    end
    
    %% delete
    methods
        function delete(this)
            delete(this.DeleteListeners);
            delete(this.PlotUIDeleteListener);
            delete(this.PropertyListeners);
        end
    end
    
    %% Internal Use - overloadable createROI static function
    methods (Static)
        function roi = CreateROI(varargin) %alias function for creating roi objects. NOTE: created rois are not added to the managed list
            roi = extras.roi.roiObject(varargin{:});
        end
    end
    
    %% List Modifiers
    methods
        function AddROI(this,roi)
            notify(this,'StartingROIAdd');
            
            if ~exist('roi','var')
                if ~isempty(this.PlotUI)
                    roi = this.PlotUI.DrawROI();
                else
                    warning('No roi was specified and no graphical interface was available for drawing ROIs');
                    roi = this.CreateROI();
                end
            end
            
            roi(~isvalid(roi)) = [];
            this.roiList = union(this.roiList,roi);
            
            % listeners
            delete(this.DeleteListeners)
            this.DeleteListeners = addlistener(this.roiList,'ObjectBeingDestroyed',@(r,~) this.RoiDeleted(r));
            
            delete(this.PropertyListeners)
            this.PropertyListeners = addlistener(this.roiList,'PropertyChanged',@(~,~) notify(this,'roiValueChanged'));
            
            notify(this,'EndingROIAdd');
            notify(this,'roiValueChanged');
        end
        function RemoveROI(this,roi)
            roi(~isvalid(roi)) = [];
            %roi = intersect(roi,this.roiList);
            
            [~,ind] = ismember(roi,this.roiList);
            ind(ind==0) = [];
            
            %adjust selection list
            [~,sind] = ismember(ind,this.IndexSelected);
            sind(sind==0)=[];
            this.IndexSelected(sind) = [];
            
            %remove roi from list
            this.roiList(ind) = [];
            
            %% delete the roi
            try
                delete(roi);
            catch
            end
            
        end
        
        function ClearList(this)
            %clear the list;
            this.IndexSelected = [];
            roi = this.roiList;
            this.roiList = extras.roi.roiObject.empty;
            
            %delete roi
            delete(roi);
        end
        
        function SetList(this,roi)
            this.ClearList;
            this.AddROI(this,roi);
        end
        
    end
    
    %% UI Plot Related
    properties(SetAccess=protected)
        PlotUI
    end
    properties(Access=protected)
        PlotUIDeleteListener
    end
    methods
        function status = RegisterROIPlotUI(this,PlotUI)
            status = false;
            if isempty(this.PlotUI)
                this.PlotUI = PlotUI;
                status = true;
                this.PlotUIDeleteListener = addlistener(PlotUI,'ObjectBeingDestroyed',@(~,~) this.DeregisterPlotUI());
            end
        end
        function DeregisterPlotUI(this)
            try
                delete(this.PlotUIDeleteListener)
            catch
            end
            
            this.PlotUI = [];
        end
    end
    
    
    %% Callbacks
    methods (Hidden)
        function RoiDeleted(this,roi)
            [~,ind]=ismember(roi,this.roiList);
            ind(ind==0) = [];
            this.roiList(ind) = [];
            
            %reset selected index
            [~,sind] = ismember(ind,this.IndexSelected);
            sind(sind==0)=[];
            this.IndexSelected(sind) = [];
        end
    end
end