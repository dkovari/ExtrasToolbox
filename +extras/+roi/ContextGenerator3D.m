classdef ContextGenerator3D < extras.roi.ContextGenerator
    
    %% 
    properties(Access=private)
        RoiManager
    end
    
    %% ctor
    methods
        function this = ContextGenerator3D(RoiObject,RoiManager)
            this@extras.roi.ContextGenerator(RoiObject);
            if isempty(RoiObject)
                delete(this);
                this = extras.roi.ContextGenerator3D.empty();
                return;
            end
            
            this.RoiManager = RoiManager;
            
            assert(isvalid(RoiObject)&&isa(RoiObject,'extras.roi.roiObject3D'),'RoiObject must be valid extras.roi.roiObject3D');
        end
    end
    
    
    %% draw method
    methods(Access=protected)
        function cm = internal_createContextMenu(this,hFig)
        %redefine this method to change items that are included in the
        %context menu
        
            cm = internal_createContextMenu@extras.roi.ContextGenerator(this,hFig);
        

            %% Create menu listing LUTs
            lut_m = uimenu(cm,'Text','Show LUT List',...
                'Separator','on',...
                'MenuSelectedFcn',@(~,~) this.showLUTList());
            
        end
    end
    
    %% Callbacks
    methods(Access=private)
        function showLUTList(this)
            extras.roi.lutListUI(this.RoiObject,this.RoiManager);
        end
    end
end