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
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.

    %% create method
    methods
        function this = roiObject3D()
            this@extras.roi.ObjectManager('extras.roi.LUTobject');
            
            addlistener(this,'ManagedObjects','PostSet',@(~,~) this.internal_updateLUT());
            
            addlistener(this,'LUTChanged',@(~,~) notify(this,'PropertyChanged'));
            
            %% By Default add LUT for self
            %this.addLUT(extras.roi.LUTobject(this));
            %handled in EzTweezy instead
        end
    end
    
    %% dtor
    methods
        function delete(this)
            delete(this.LutPropListeners);
            delete(this.LutDestroyListeners);
        end
    end
    
    properties(SetObservable,AbortSet)
        DefaultLUT = extras.roi.LUTobject.empty();
    end
    methods
        function set.DefaultLUT(this,obj)
            assert(isa(obj,'extras.roi.LUTobject'));
            if isempty(this.LUT)
                this.DefaultLUT = extras.roi.LUTobject.empty();
            else
                assert(ismember(obj,this.LUT),'Obj must be added to LUT List before it can be set as default LUT');
                this.DefaultLUT = obj;
            end
            notify(this,'LUTChanged',extras.GenericEvent('DefaultLUT',this.DefaultLUT))
        end
    end

    %% Aliased Properties
    properties (SetAccess=protected,SetObservable,AbortSet)
        LUT extras.roi.LUTobject = extras.roi.LUTobject.empty() %array of LUT objects
    end
    methods (Access=private)
        function internal_updateLUT(this)
            this.LUT = this.ManagedObjects;
            %'internal update'
            %
            
            if isempty(this.LUT)
                this.DefaultLUT = extras.roi.LUTobject.empty();
            elseif isempty(this.DefaultLUT) || ~ismember(this.DefaultLUT,this.LUT)
                this.DefaultLUT = this.LUT(1);
            end
            
            notify(this,'LUTChanged',extras.GenericEvent('LUT',this.LUT));
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
            %assert(numel(this)==1,'toStruct only works on one roiObject at a time');
            s = toStruct@extras.roi.roiObject(this);
            for n=1:numel(this)
                s(n).LUT = this(n).LUT.toStruct();
                s(n).DefaultLUT = struct('UUID',{this(n).DefaultLUT.UUID});
            end
        end
        function addLUT(this,LUT,varargin)
        % add LUT object to list
        % Syntax:
        %   this.addLUT(): add empty LUT
        %   this.addLUT(LUT): add LUT object to list
        
            
            if nargin<2||~isa(LUT,'extras.roi.LUTobject')
                LUT = extras.roi.LUTobject(LUT,varargin{:});
            end
            
            newobj=addObjects(this,LUT);
            setParentROI(LUT,this);
            
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