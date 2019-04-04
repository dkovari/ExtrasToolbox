classdef roiObject3D < extras.roi.roiObject
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

    properties
        LUT extras.roi.LUTobject %array of LUT structs
    end
    methods
        function s = toStruct(this)
            s = struct('Window',{this.Window},'UUID',{this.UUID},'LUT',{});
            for n=1:numel(this)
                s(n).LUT = this(n).LUT.toStruct();
            end
        end
        
        function addLUT(this,LUT)
            [~,lob] = ismember({LUT.UUID},{this.LUT.UUID});
            for n=1:numel(lob)
                if(lob(n)~=0)
                    this.LUT(lob(n))=LUT(n);
                end
            end
            this.LUT = [this.LUT,LUT(lob==0)];
        end
        function removeLUT(this,LUT)
            [~,lob] = ismember({LUT.UUID},{this.LUT.UUID});
            lob(lob==0) = [];
            this.LUT(lob) = [];
        end
    end

end