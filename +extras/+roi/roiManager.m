classdef roiManager < handle & extras.roi.ObjectManager
% extras.roi.roiManager

    %% constructor
    methods
        function this = roiManager()
            this@extras.roi.ObjectManager('extras.roi.roiObject');
            
            addlistener(this,'ManagedObjects','PostSet',@(~,~) this.internal_updateROI());
        end
    end

    %% Aliased Properties
    properties (SetAccess=private,SetObservable,AbortSet)
        roiList = extras.roi.roiObject.empty %array of roi
    end
    methods (Access=private)
        function internal_updateROI(this)
            oldList = this.roiList;
            newList = this.ManagedObjects;
            
            %% change roiList
            this.roiList = newList;
            
            %% update selection list
            [~,ia,~] = intersect(oldList,newList); %overlapping indicies
            this.IndexSelected = intersect(ia,this.IndexSelected); %only keep selected that are in the overlapping indicies
        end
    end
    
    %% Other props
    properties (SetObservable=true,AbortSet=true)
        IndexSelected = [];
    end

    events
        roiValueChanged;
        StartingROIAdd;
        EndingROIAdd;
    end
    
    %% Internal properties
    properties(Access=protected)
        PropertyListeners;
    end
    
    %% delete
    methods
        function delete(this)
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
            
            createdROI = false;
            if ~exist('roi','var')
                if ~isempty(this.PlotUI)
                    roi = this.PlotUI.DrawROI();
                else
                    warning('No roi was specified and no graphical interface was available for drawing ROIs');
                    roi = this.CreateROI();
                end
                createdROI = true;
            end
            try
                roi(~isvalid(roi)) = [];

                %% add
                this.addObjects(roi);

                %% handle PropertyListeners
                delete(this.PropertyListeners)
                this.PropertyListeners = addlistener(this.roiList,'PropertyChanged',@(~,~) notify(this,'roiValueChanged'));

                notify(this,'EndingROIAdd');
                notify(this,'roiValueChanged');
            catch ME
                if createdROI
                    try
                        delete(roi);
                    catch
                    end
                end
                rethrow(ME);
            end
        end
        function RemoveROI(this,roi)
            roi(~isvalid(roi)) = [];
            
            [~,ind] = ismember(roi,this.roiList);
            ind(ind==0) = [];
            
            %remove roi from list
            this.removeObjects(roi);
            
            %% delete the roi
            try
                delete(roi);
            catch
            end
            
            if ~isempty(ind)
                notify(this,'roiValueChanged');
            end
            
        end
        
        function ClearList(this)
            %clear the list;
            this.IndexSelected = [];
            roi = this.roiList;
            
            
            this.clearObjects();
            
            %delete roi
            delete(roi);
            if ~isempty(roi)
                notify(this,'roiValueChanged');
            end
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
end