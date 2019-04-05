classdef roiObject3D < extras.roi.roiObject & extras.roi.ObjectManager
% roi = ImageTracker.roiObject3D(Window)
% 
% ROI window object, with unique identifier
%
% Construction:
%============================
% ROI(1:n) = ImageTracker.roiObject3D( [n x 4])
% Windows must be n x 4 array
%   number of rows corresponds to the number of roi objects created
%
% Window: 1x4 array specifying [x0,y0,w,h] of rectangle


    %% create method
    methods
        function this = roiObject3D()
            this@extras.roi.ObjectManager('extras.roi.LUTobject');
            
            addlistener(this,'ManagedObjects','PostSet',@(~,~) this.internal_updateLUT());
        end
    end

    %% Aliased Properties
    properties (SetAccess=protected,SetObservable,AbortSet)
        LUT extras.roi.LUTobject = extras.roi.LUTobject.empty() %array of LUT structs
        DefaultLUT = extras.roi.LUTobject.empty();
    end
    methods (Access=private)
        function internal_updateLUT(this)
            this.LUT = this.ManagedObjects;
            
            if isempty(this.DefaultLUT) || all(this.LUT ~= this.DefaultLUT)
                this.DefaultLUT = this.LUT(1);
            end
        end
    end
   
    
    %% public
    methods
        function s = toStruct(this)
            assert(numel(this)==1,'toStruct only works on one roiObject at a time');
            s = struct('Window',{this.Window},'UUID',{this.UUID},'LUT',{});
            for n=1:numel(this)
                s(n).LUT = this(n).LUT.toStruct();
            end
        end
        function addLUT(this,LUT)
            addObjects(this,LUT);
        end
        function removeLUT(this,LUT)
            removeObjects(this,LUT);
        end
        function clearLUT(this)
            clearObjects(this);
        end
    end

end