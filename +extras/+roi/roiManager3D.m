classdef roiManager3D < extras.roi.roiManager
    %% Internal Use - overloadable createROI static function
    methods (Static)
        function roi = CreateROI(varargin) %alias function for creating roi objects. NOTE: created rois are not added to the managed list
            roi = extras.roi.roiObject3D(varargin{:});
        end
    end
end