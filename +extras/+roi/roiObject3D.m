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
    
    %% dtor
    methods
        function delete(this)
            delete(this.LutPropListeners);
            delete(this.LutDestroyListeners);
        end
    end

    %% Aliased Properties
    properties (SetAccess=protected,SetObservable,AbortSet)
        LUT extras.roi.LUTobject = extras.roi.LUTobject.empty() %array of LUT objects
        DefaultLUT = extras.roi.LUTobject.empty();
    end
    methods (Access=private)
        function internal_updateLUT(this)
            this.LUT = this.ManagedObjects;
            
            if isempty(this.DefaultLUT) || all(this.LUT ~= this.DefaultLUT)
                if isempty(this.LUT)
                    this.DefaultLUT = extras.roi.LUTobject.empty();
                else
                    this.DefaultLUT = this.LUT(1);
                end
            end
        end
    end
    
    %%
    events
        LUTChanged
    end
    
    %% 
    properties(Access=private)
        LutPropListeners = event.listener.empty();
        LutDestroyListeners = event.listener.empty();
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
            newobj=addObjects(this,LUT)
            this.LutPropListeners = [this.LutPropListeners, addlistener(newobj,'PropertyChanged',@(h,e) notify(this,'LUTChanged',extras.GenericEvent('LUT',h)))]; %create listener which forwards changes made to LUTs in the lut list
            this.LutDestroyListeners = [this.LutDestroyListeners,addlistener(newobj,'ObjectBeingDestroyed',@(h,e) notify(this,'LUTChanged',extras.GenericEvent('LUT',h)))];
        end
        function removeLUT(this,LUT)
            removeObjects(this,LUT);
        end
        function clearLUT(this)
            clearObjects(this);
        end
    end

end