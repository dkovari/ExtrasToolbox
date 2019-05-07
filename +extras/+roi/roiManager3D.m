classdef roiManager3D < extras.roi.roiManager
    %% Internal Use - overloadable createROI static function
    methods (Static)
        function roi = CreateROI(varargin) %alias function for creating roi objects. NOTE: created rois are not added to the managed list
            roi = extras.roi.roiObject3D(varargin{:});
        end
    end
    
    %creator
    methods
        function this = roiManager3D()
            this.changeObjectClassName('extras.roi.roiObject3D');
        end
    end
    
    %context generator customization
    methods(Static)
        function cg = createContextGenerators(roiObjs)
            %redefinable method for creating extras.roi.ContextGenerator
            %objects from roiObjects
            cg = extras.roi.ContextGenerator(roiObjs);
        end
    end
end