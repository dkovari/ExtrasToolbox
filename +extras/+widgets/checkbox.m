%OBSOLETE!!!!

classdef checkbox < extras.widgets.abstract.CheckboxControl
% Checkbox control, looks similar to controls included in the widgets
% toolbox

    %% construct
    methods
        function obj = checkbox(varargin)
        % Call superclass constructors
            obj@extras.widgets.abstract.CheckboxControl();
            
            % Populate public properties from P-V input pairs
            obj.assignPVPairs(varargin{:});
            
            % Assign the construction flag
            obj.IsConstructed = true;
            
            % Do the following only if the object is not a subclass
            if strcmp(class(obj), 'extras.widgets.checkbox') %#ok<STISA>
                
                % Redraw the widget
                obj.onResized();
                obj.onEnableChanged();
                obj.redraw();
                obj.onStyleChanged();
                obj.updateUI();
                
                
            end
        end
    end
end