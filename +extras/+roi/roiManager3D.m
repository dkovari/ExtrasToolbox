classdef roiManager3D < extras.roi.roiManager & extras.widgets.mixin.AssignNV
% Extension of roiManager to work with roiObject3D and create proper
% context menu
% 
% Constructor Options:
% ContextGeneratorOptions = {}
%   You can include a cell array of name-value pairs which are forwarded to
%   the extras.roi.ContextGenerator3D() constructor.
%   This is useful for passing arguments which might change the
%   functionality of nested menus.
%
%   See extras.roi.ContextGenerator3D for more detail.
%
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.
    
    %% Internal Use - overloadable createROI static function
    methods (Static)
        function roi = CreateROI(varargin) %alias function for creating roi objects. NOTE: created rois are not added to the managed list
            roi = extras.roi.roiObject3D(varargin{:});
        end
    end
    
    %% ContextGeneratorOptions
    properties(SetObservable,AbortSet)
        ContextGeneratorOptions cell = {}; %Name,Value options passed to ContextGenerator constructor
    end
    
    %% Create
    methods
        function this = roiManager3D(varargin)
            this.changeObjectClassName('extras.roi.roiObject3D');
            
            %% set public
            this.setPublicProperties(varargin{:});
            
        end
    end
    
    %context generator customization
    methods
        function cg = createContextGenerators(this,roiObjs)
            %redefinable method for creating extras.roi.ContextGenerator
            %objects from roiObjects
            cg = extras.roi.ContextGenerator3D(roiObjs,this,this.ContextGeneratorOptions{:});
        end
    end
end