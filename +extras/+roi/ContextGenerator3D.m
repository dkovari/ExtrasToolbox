classdef ContextGenerator3D < extras.roi.ContextGenerator & extras.widgets.mixin.AssignNV
% Generate Context Menu for ui display of roiObjects
%
% Constructor Options:
% LutListUiOptions = {}
%   You can include a cell array of name-value pairs which are forwarded to
%   the extras.roi.lutListUI() constructor.
%
%   See extras.roi.lutListUI for more detail
%
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.

    %% 
    properties(Access=private)
        RoiManager
    end
    
    properties(SetObservable,AbortSet)
        LutListUiOptions cell = {}; %Name,Value arguments to forward to lutListUI() constructor
    end
    
    
    %% ctor
    methods
        function this = ContextGenerator3D(RoiObject,RoiManager,varargin)
            %Syntax:
            
            
            this@extras.roi.ContextGenerator(RoiObject);
            if isempty(RoiObject)
                delete(this);
                this = extras.roi.ContextGenerator3D.empty();
                return;
            end
            
            this.RoiManager = RoiManager;
            
            assert(isvalid(RoiObject)&&isa(RoiObject,'extras.roi.roiObject3D'),'RoiObject must be valid extras.roi.roiObject3D');
            
            %% Set other props
            this.setPublicProperties(varargin{:});
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
    methods(Access=protected)
        function showLUTList(this)
            extras.roi.lutListUI(this.RoiObject,this.RoiManager,this.LutListUiOptions{:});
        end
    end
end