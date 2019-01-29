classdef (HandleCompatible) HasContainer < extras.widgets.mixin.AssingNV & ...
        matlab.mixin.CustomDisplay
    % HasContainer - Mixin class for a graphical widget container
    %
    % This class provides Public-facing common properties and methods 
    % related to graphical containers
    % 
    %
    
    % Inspired by Widget's toolbox uiw.widget.mixin.HasContainer
    % Original:
    %   Copyright 2017-2018 The MathWorks, Inc.
    %
    % Auth/Revision:
    %   MathWorks Consulting
    %   $Author: rjackey $
    %   $Revision: 285 $
    %   $Date: 2018-11-15 07:04:50 -0500 (Thu, 15 Nov 2018) $
    %
    % This version:
    %   Author: Dan Kovari, Emory University.
    %   Revision 1
    %   Date: 1/24/2019
    %   
    % ---------------------------------------------------------------------
    
    %% Properties
    properties (AbortSet=true, SetObsetvable=true)
        Enable = 'on' %Allow interaction with this widget [(on)|off]
        Padding = 0 %Pixel spacing around the widget (applies to some widgets)
        Spacing = 4 %Pixel spacing between controls (applies to some widgets)
    end %properties
    
    properties (SetAccess=protected)
        h struct = struct() %For widgets to store internal graphics objects
        hLayout struct = struct() %For widgets to store internal layout objects
        IsConstructed logical = false %Indicates widget has completed construction, useful for optimal performance to minimize redraws on launch, etc.
    end %properties
    
    
    %% Abstract Methods
    methods (Abstract, Access=protected)
        redraw(obj) %Handle state changes that may need UI redraw - subclass must override
        onResized(obj) %Handle changes to widget size - subclass must override
        onContainerResized(obj) %Triggered on resize of the widget's container - subclass must override
    end %methods
    
    
    %% Protected methods
    methods (Access=protected)
        
        function onEnableChanged(obj,hAdd)
        % Handle updates to Enable state - subclass may override
        % By default, changes to Enable will be applied to all objects
        % contained in obj.h
        %
        % If you override this method, it is recomended you call it in your
        % override code.
            
            % Ensure the construction is complete
            if obj.IsConstructed
                
                % Look for all encapsulated graphics objects in "h" property
                hAll = obj.findHandleObjects();
                
                % Combine them all
                if nargin>1 && ~isempty(hAdd) && all(ishghandle(hAdd))
                    hAll = unique([hAll(:); hAdd(:)]);
                end
                
                % Default behavior: Set all objects with an Enable field
                hHasEnable = hAll( isprop(hAll,'Enable') );
                set(hHasEnable,'Enable',obj.Enable);
                
            end %if obj.IsConstructed
            
        end %function
        
        
        function onVisibleChanged(obj)
            % Handle updates to Visible state - subclass may override
            
            % Ensure the construction is complete
            if obj.IsConstructed
                
            end %if obj.IsConstructed
            
        end %function
        
        
        function onStyleChanged(obj,hAdd)
            % Handle updates to style changes - subclass may override
            
            % Ensure the construction is complete
            if obj.IsConstructed
                
                % Look for all encapsulated graphics objects in "h" property
                hAll = obj.findHandleObjects();
                
                % Combine them all
                if nargin>1 && ~isempty(hAdd)
                    hAll = unique([hAll(:); hAdd(:)]);
                end
                
                % Set all objects that have font props
                if isprop(obj,'FontName')
                    set(hAll( isprop(hAll,'FontName') ),...
                        'FontName',obj.FontName,...
                        'FontSize',obj.FontSize);
                    set(hAll( isprop(hAll,'FontUnits') ),...
                        'FontWeight',obj.FontWeight,...
                        'FontAngle',obj.FontAngle,...
                        'FontUnits',obj.FontUnits);
                end
                
                % Set all objects that have ForegroundColor
                % Exclude boxpanels
                if isprop(obj,'ForegroundColor')
                    isBoxPanel = arrayfun(@(x)isa(x,'uix.BoxPanel'),hAll);
                    set(hAll( isprop(hAll,'ForegroundColor') & ~isBoxPanel ),...
                        'ForegroundColor',obj.ForegroundColor);
                end
                
                % Set all objects that have BackgroundColor
                if isprop(obj,'BackgroundColor')
                    hasBGColor = isprop(hAll,'BackgroundColor');
                    set(hAll( hasBGColor ),...
                        'BackgroundColor',obj.BackgroundColor);
                end
                
            end %if obj.IsConstructed
            
        end %function
        
        
        function hAll = findHandleObjects(obj)
            
            % Look for all encapsulated graphics objects in "h" property
            %hEncapsulatedCell = struct2cell(obj.h);
            hEncapsulatedCell = [struct2cell(obj.h); struct2cell(obj.hLayout)];
            isGraphicsObj = cellfun(@ishghandle,hEncapsulatedCell,'UniformOutput',false);
            isGraphicsObj = cellfun(@all,isGraphicsObj,'UniformOutput',true);
            hAll = [hEncapsulatedCell{isGraphicsObj}]';
            
        end %function
        
    end %methods
    
    
    
    %% Get/Set methods
    methods
        
        % Enable
        function set.Enable(obj,value)
            value = extras.validateOnOff(value,'ArrayInput',false);
            evt = struct('Property','Enable',...
                'OldValue',obj.Enable,...
                'NewValue',value);
            obj.Enable = value;
            obj.onEnableChanged(evt);
        end
        
        % Padding
        function set.Padding(obj,value)
            validateattributes(value,{'numeric'},{'real','nonnegative','scalar','finite'})
            obj.Padding = value;
            obj.onContainerResized();
        end
        
        % Spacing
        function set.Spacing(obj,value)
            validateattributes(value,{'numeric'},{'real','nonnegative','scalar','finite'})
            obj.Spacing = value;
            obj.onContainerResized();
        end
        
        % IsConstructed
        function value = get.IsConstructed(obj)
            value = isvalid(obj) && obj.IsConstructed;
        end
        
    end % Get/Set methods
    
    
end % classdef
