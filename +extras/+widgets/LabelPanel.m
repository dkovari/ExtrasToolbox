classdef LabelPanel < handle & ...
        extras.RequireGuiLayoutToolbox &...
        uix.Container & ...
        extras.widgets.mixin.abstract.Container & ...
        extras.widgets.mixin.AssignNV & ...
        extras.widgets.mixin.HasCallback &...
        extras.widgets.mixin.abstract.HasLabel
    
    %% Grid Related
    properties (Access=public, AbortSet, Dependent)
        Widths % widths of contents, in pixels and/or weights
        MinimumWidths % minimum widths of contents, in pixels
        Heights % heights of contents, in pixels and/or weights
        MinimumHeights % minimum heights of contents, in pixels
    end
    
    properties( Access = protected )
        Widths_ = zeros(0,1) % backing for Widths
        MinimumWidths_ = zeros(0,1) % backing for MinimumWidths
        Heights_ = zeros(0,1) % backing for Heights
        MinimumHeights_ = zeros(0,1) % backing for MinimumHeights
    end
    
    %% Grid Related get/set
    methods
        
        function value = get.Widths( obj )
            value = obj.Widths_;
        end
        
        function set.Widths( this, value )
            
            % For those who can't tell a column from a row...
            if isrow( value )
                value = transpose( value );
            end
            
            % Check
            assert( isa( value, 'double' ), 'uix:InvalidPropertyValue', ...
                'Property ''Widths'' must be of type double.' )
            assert( all( isreal( value ) ) && ~any( isinf( value ) ) && ...
                ~any( isnan( value ) ), 'uix:InvalidPropertyValue', ...
                'Elements of property ''Widths'' must be real and finite.' )
            n = numel( this.Contents_ ) - double(this.LabelPanel_hLabel_Contstructed);
            b = numel( this.Widths_ );
            q = numel( this.Heights_ );
            c = numel( value );
            r = ceil( n / c );
            if c < min( [1 n] )
                error( 'uix:InvalidPropertyValue' , ...
                    'Property ''Widths'' must be non-empty for non-empty contents.' )
            elseif ceil( n / r ) < c
                error( 'uix:InvalidPropertyValue' , ...
                    'Size of property ''Widths'' must not lead to empty columns.' )
            elseif c > n
                error( 'uix:InvalidPropertyValue' , ...
                    'Size of property ''Widths'' must be no larger than size of contents.' )
            end
            
            % Set
            this.Widths_ = value;
            if c < b % number of columns decreasing
                this.MinimumWidths_(c+1:end,:) = [];
                if r > q % number of rows increasing
                    this.Heights_(end+1:r,:) = -1;
                    this.MinimumHeights_(end+1:r,:) = 1;
                end
            elseif c > b % number of columns increasing
                this.MinimumWidths_(end+1:c,:) = -1;
                if r < q % number of rows decreasing
                    this.Heights_(r+1:end,:) = [];
                    this.MinimumHeights_(r+1:end,:) = [];
                end
            end
            
            % Mark as dirty
            this.Dirty = true;
            
        end
        
        function value = get.MinimumWidths( obj )
            value = obj.MinimumWidths_;
        end
        
        function set.MinimumWidths( obj, value )
            
            % For those who can't tell a column from a row...
            if isrow( value )
                value = transpose( value );
            end
            
            % Check
            assert( isa( value, 'double' ), 'uix:InvalidPropertyValue', ...
                'Property ''MinimumWidths'' must be of type double.' )
            assert( all( isreal( value ) ) && ~any( isinf( value ) ) && ...
                all( value >= 0 ), 'uix:InvalidPropertyValue', ...
                'Elements of property ''MinimumWidths'' must be non-negative.' )
            assert( isequal( size( value ), size( obj.Widths_ ) ), ...
                'uix:InvalidPropertyValue', ...
                'Size of property ''MinimumWidths'' must match size of contents.' )
            
            % Set
            obj.MinimumWidths_ = value;
            
            % Mark as dirty
            obj.Dirty = true;
            
        end % set.MinimumWidths
        
        function value = get.Heights( obj )
            
            value = obj.Heights_;
            
        end % get.Heights
        
        function set.Heights( this, value )
            
            % For those who can't tell a column from a row...
            if isrow( value )
                value = transpose( value );
            end
            
            % Check
            assert( isa( value, 'double' ), 'uix:InvalidPropertyValue', ...
                'Property ''Heights'' must be of type double.' )
            assert( all( isreal( value ) ) && ~any( isinf( value ) ) && ...
                ~any( isnan( value ) ), 'uix:InvalidPropertyValue', ...
                'Elements of property ''Heights'' must be real and finite.' )
            n = numel( this.Contents_ ) - double(this.LabelPanel_hLabel_Contstructed);
            b = numel( this.Widths_ );
            q = numel( this.Heights_ );
            r = numel( value );
            c = ceil( n / r );
            if r < min( [1 n] )
                error( 'uix:InvalidPropertyValue' , ...
                    'Property ''Heights'' must be non-empty for non-empty contents.' )
            elseif r > n
                error( 'uix:InvalidPropertyValue' , ...
                    'Size of property ''Heights'' must be no larger than size of contents.' )
            end
            
            % Set
            this.Heights_ = value;
            if r < q % number of rows decreasing
                this.MinimumHeights_(r+1:end,:) = [];
                if c > b % number of columns increasing
                    this.Widths_(end+1:c,:) = -1;
                    this.MinimumWidths_(end+1:c,:) = 1;
                end
            elseif r > q % number of rows increasing
                this.MinimumHeights_(end+1:r,:) = 1;
                if c < b % number of columns decreasing
                    this.Widths_(c+1:end,:) = [];
                    this.MinimumWidths_(c+1:end,:) = [];
                end
            end
            
            % Mark as dirty
            this.Dirty = true;
            
        end % set.Heights
        
        function value = get.MinimumHeights( obj )
            value = obj.MinimumHeights_;
        end % get.MinimumHeights
        
        function set.MinimumHeights( obj, value )
            
            % For those who can't tell a column from a row...
            if isrow( value )
                value = transpose( value );
            end
            
            % Check
            assert( isa( value, 'double' ), 'uix:InvalidPropertyValue', ...
                'Property ''MinimumHeights'' must be of type double.' )
            assert( all( isreal( value ) ) && ~any( isinf( value ) ) && ...
                all( value >= 0 ), 'uix:InvalidPropertyValue', ...
                'Elements of property ''MinimumHeights'' must be non-negative.' )
            assert( isequal( size( value ), size( obj.Heights_ ) ), ...
                'uix:InvalidPropertyValue', ...
                'Size of property ''MinimumHeights'' must match size of contents.' )
            
            % Set
            obj.MinimumHeights_ = value;
            
            % Mark as dirty
            obj.Dirty = true;
            
        end
    end
    
    %% Spacing (from uix.Box)
    properties( Access = public, Dependent, AbortSet )
        Spacing = 0 % space between contents, in pixels
    end
    
    properties( Access = protected )
        Spacing_ = 0 % backing for Spacing
    end
    
    methods
        
        function value = get.Spacing( obj )
            
            value = obj.Spacing_;
            
        end % get.Spacing
        
        function set.Spacing( obj, value )
            
            % Check
            assert( isa( value, 'double' ) && isscalar( value ) && ...
                isreal( value ) && ~isinf( value ) && ...
                ~isnan( value ) && value >= 0, ...
                'uix:InvalidPropertyValue', ...
                'Property ''Spacing'' must be a non-negative scalar.' )
            
            % Set
            obj.Spacing_ = value;
            
            % Mark as dirty
            obj.Dirty = true;
            
        end % set.Spacing
        
    end % accessors
    
    %% LabelRelated
    properties  (Access=public, AbortSet, SetObservable)
        LabelPosition = 'left'
        LabelOrientation = 'horizontal'
        LabelVerticalAlignment = 'center';
        LabelHorizontalAlignment = 'leading';
        LabelWidth = -0.5;
        LabelHeight = -0.5
    end
    
    %% label related get/set
    methods
        function set.LabelPosition(this,value)
            this.LabelPosition = validatestring(value,{'left','right','top','bottom'});
            this.redraw();
        end
        
        function set.LabelOrientation(this,value)
            if this.LabelPanel_hLabel_Contstructed
                this.hLabel.Orientation = value;
                this.LabelOrientation = this.hLabel.Orientation;
            else
                this.LabelOrientation =validatestring(value,{'horizontal','vertical','clockwise','counterclockwise'});
            end
        end
        
        function set.LabelVerticalAlignment(this,value)
            if this.LabelPanel_hLabel_Contstructed
                this.hLabel.VerticalAlignment = value;
                this.LabelVerticalAlignment = this.hLabel.VerticalAlignment;
            else
                this.LabelVerticalAlignment =validatestring(value,{'top','center','bottom'});
            end
        end
        
        function set.LabelHorizontalAlignment(this,value)
            if this.LabelPanel_hLabel_Contstructed
                this.hLabel.HorizontalAlignment = value;
                this.LabelHorizontalAlignment = this.hLabel.HorizontalAlignment;
            else
                this.LabelHorizontalAlignment =validatestring(value,{'left','center','right','leading','trailing'});
            end
        end
    end
    
    %% internal only
    properties(Access=private)
        hLabel
        adding_hLabel = false;
        LabelPanel_ParentSetListener
        LabelPanel_hLabel_Contstructed = false;
    end
    
    %% Constructor
    methods
        function this = LabelPanel(varargin)
            %% add temporary parent-set listener
            this.LabelPanel_ParentSetListener = addlistener(this,'Parent','PostSet',@(~,~) this.LabelPanel_ParentSetCallback());
            
            %% set properties
            
            if nargin>0 && ~ischar(varargin{1}) && numel(varargin{1})==1 && isgraphics(varargin{1})
                this.Parent = varargin{1};
                varargin(1) = [];
            end
            
            this.setPublicProperties(varargin{:});
        end
    end
    
    %% Temp ParentSet Callback
    methods (Access=private)
        function LabelPanel_ParentSetCallback(this)
            if ~isvalid(this)
                return;
            end
            if this.LabelPanel_hLabel_Contstructed
                return;
            end
            if ~isempty(this.Parent) && isvalid(this.Parent)
                this.adding_hLabel = true;
                this.hLabel = extras.widgets.jText('Parent',this,...
                    'Orientation',this.LabelOrientation,...
                    'VerticalAlignment',this.LabelVerticalAlignment,...
                    'HorizontalAlignment',this.LabelHorizontalAlignment,...
                    'String',this.Label);
                this.adding_hLabel = false;
                this.addLabelStringObject(this.hLabel);
                
                %% delete parent set listener
                try
                    delete(this.LabelPanel_ParentSetListener)
                catch
                end
                
                %% set constructed flag
                this.LabelPanel_hLabel_Contstructed = true;
                
                %% redraw
                this.redraw();
            end
        end
    end
    
    %% Container Related
    methods( Access = protected )
        
        function value = getContents(obj)
            value = obj.Contents_;
        end
        function setContents(obj,value)
        %overloadable function for set.Contents
        
            %% For those who can't tell a column from a row...
            value = reshape(value,[],1);
            value = unique(value);
            
            %% Check
            [tf, indices] = ismember( value, obj.Contents_ );
            assert(all(tf) && ... %new contents must be contained in Contents_
                numel(value)==numel(obj.Contents_)-1,...
                'uix:InvalidOperation', ...
                'Property ''Contents'' may only be set to a permutation of itself.');
                
            % Call reorder
            obj.reorder( indices )
        end
        
        function reorder( obj, indices )
            %reorder  Reorder contents
            %
            %  c.reorder(i) reorders the container contents using indices
            %  i, c.Contents = c.Contents(i).
            
            if obj.LabelPanel_hLabel_Contstructed
                indices = reshape(indices,1,[]);
                indices = setdiff(indices,1,'stable');
                indices = [1,indices];
            end
            
            % Reorder contents
            obj.Contents_ = obj.Contents_(indices);
            
            % Reorder listeners
            obj.ActivePositionPropertyListeners = ...
                obj.ActivePositionPropertyListeners(indices,:);
            
            % Mark as dirty
            obj.Dirty = true;
            
        end % reorder
        
        function redraw( this )
            % Impelment redraw()
            %  c.redraw() redraws the container c.
            
            if isempty(ancestor( this, 'figure' )) || ~isvalid(ancestor( this, 'figure' ))
                return;
            end
            
            %% Outer bounds
            bounds = hgconvertunits( ancestor( this, 'figure' ), ...
                [0 0 1 1], 'normalized', 'pixels', this );
            %% set position of Label
            switch this.LabelPosition
                case 'left'
                    xSz = uix.calcPixelSizes(bounds(3),[this.LabelWidth,-1],[1,1],this.Padding_,0);
                    x0 = bounds(1)+this.Padding_;
                    xSz(1) = xSz(1);
                    if isempty(this.Heights_)||any(this.Heights_<0)
                        height = -1;
                    else
                        height = sum(this.Heights_);
                    end
                    ySz = uix.calcPixelSizes(bounds(4),height,1,this.Padding_,0);
                    y0 = bounds(2)+this.Padding_;
                    if this.LabelPanel_hLabel_Contstructed
                        uix.setPosition(this.hLabel,[x0,y0,xSz(1),ySz],'pixels');
                    end
                    bounds = [x0+xSz(1),1,xSz(2)+this.Padding_,bounds(4)];
                case 'right'
                    xSz = uix.calcPixelSizes(bounds(3),[-1,this.LabelWidth],[1,1],this.Padding_,0);
                    x0 = bounds(1)+this.Padding_+xSz(1);
                    xSz(2) = xSz(2);
                    if isempty(this.Heights_)||any(this.Heights_<0)
                        height = -1;
                    else
                        height = sum(this.Heights_);
                    end
                    ySz = uix.calcPixelSizes(bounds(4),height,1,this.Padding_,0);
                    y0 = bounds(2)+this.Padding_+1;
                    if this.LabelPanel_hLabel_Contstructed
                        uix.setPosition(this.hLabel,[x0,y0,xSz(1),ySz],'pixels');
                    end
                    bounds = [1,1,xSz(1)+this.Padding_,bounds(4)];
                case 'top'
                    ySz = uix.calcPixelSizes(bounds(4),[-1,this.LabelHeight],[1,1],this.Padding_,0);
                    y0 = bounds(2)+this.Padding_+ySz(1);
                    ySz(2) = ySz(2);
                    
                    if isempty(this.Widths_)||any(this.Widths_<0)
                        widths = -1;
                    else
                        widths = sum(this.Widths_);
                    end
                    xSz = uix.calcPixelSizes(bounds(3),widths,1,this.Padding_,0);
                    x0 = bounds(1)+this.Padding_+1;
                    if this.LabelPanel_hLabel_Contstructed
                        uix.setPosition(this.hLabel,[x0,y0,xSz,ySz(2)],'pixels');
                    end
                    bounds = [1,1,bounds(3),ySz(1)+this.Padding_];
                case 'bottom'
                    ySz = uix.calcPixelSizes(bounds(4),[this.LabelHeight,-1],[1,1],this.Padding_,0);
                    y0 = bounds(2)+this.Padding_;
                    ySz(1) = ySz(1);
                    
                    if isempty(this.Widths_)||any(this.Widths_<0)
                        widths = -1;
                    else
                        widths = sum(this.Widths_);
                    end
                    xSz = uix.calcPixelSizes(bounds(3),widths,1,this.Padding_,0);
                    x0 = bounds(1)+this.Padding_+1;
                    if this.LabelPanel_hLabel_Contstructed
                        uix.setPosition(this.hLabel,[x0,y0,xSz,ySz(1)],'pixels');
                    end
                    bounds = [1,y0+ySz(1),bounds(3),ySz(2)+this.Padding_];
            end
            
            %% Set other elements
            widths = this.Widths_;
            minimumWidths = this.MinimumWidths_;
            heights = this.Heights_;
            minimumHeights = this.MinimumHeights_;
            padding = this.Padding_;
            spacing = this.Spacing_;
            c = numel( widths );
            r = numel( heights );
            n = numel( this.Contents_ ) - double(this.LabelPanel_hLabel_Contstructed);
            xSizes = uix.calcPixelSizes( bounds(3), widths, ...
                minimumWidths, padding, spacing );
            xPositions = [cumsum( [0; xSizes(1:end-1,:)] ) + padding + ...
                spacing * transpose( 0:c-1 ) + bounds(1), xSizes];
            ySizes = uix.calcPixelSizes( bounds(4), heights, ...
                minimumHeights, padding, spacing );
            yPositions = [bounds(4) - cumsum( ySizes ) - padding - ...
                spacing * transpose( 0:r-1 ) + bounds(2), ySizes];
            [iy, ix] = ind2sub( [r c], transpose( 1:n ) );
            positions = [xPositions(ix,1), yPositions(iy,1), ...
                xPositions(ix,2), yPositions(iy,2)];
            
            % Set positions
            if this.LabelPanel_hLabel_Contstructed
                children = this.Contents_(2:end);
            else
                children = this.Contents_;
            end
            for ii = 1:numel( children )
                uix.setPosition( children(ii), positions(ii,:), 'pixels' );
            end
            
        end
        
        function addChild(this,child)
            %impelments addChild
            
            if ~this.LabelPanel_hLabel_Contstructed && this.adding_hLabel %adding hLabel
                assert(~ismember(child,this.Contents_),'child (should be hLabel) is already in contents');
                assert(numel(child)==1,'child (should be hLabel) must have only one handle');
                this.Contents_ = [child;this.Contents_];
                this.ActivePositionPropertyListeners = cat(1,{[]},this.ActivePositionPropertyListeners);
                return;
            end
            
            child = unique(child);
            child = setdiff(child,this.Contents_,'stable');
            
            if isempty(child)
                return;
            end
            
            n_child = numel(child);
            
            old_n = numel(this.Contents_);
            
            if this.LabelPanel_hLabel_Contstructed
                old_n = old_n-1;
            end
            
            old_c = numel(this.Widths_);
            old_r = numel(this.Heights_);
            
            if old_n==0 %first object
                this.Widths_(end+1:end+n_child,:) = -1;
                this.MinimumWidths_(end+1:end+n_child,:) = 1;
                this.Heights_(end+1,:) = -1;
                this.MinimumHeights_(end+1,:) = 1;
            else
                if old_r==1 %only a single row, add horizontally
                    this.Widths_(end+1:end+n_child,:) = -1;
                    this.MinimumWidths_(end+1:end+n_child,:) = 1;
                elseif old_r>1 && old_c==1 %column array
                    this.Heights_(end+1:end+n_child,:) = -1;
                    this.MinimumHeights_(end+1:end+n_child,:) = 1;
                else %grid, add along col, then increase cols
                    for ii=1:n_child
                        if old_n<old_r*old_c %extra room, add to row
                            old_n=old_n+1;
                        else %need new row
                            this.Widths_(end+1,:) = -1;
                            this.MinimumWidths_(end+1,:) = 1;
                        end
                    end
                end
            end
            
            %% Add to contents
            this.Contents_ = reshape(union(this.Contents_,child,'stable'),[],1);
            %% Add listeners
            for n=1:n_child
                if isa( child(n), 'matlab.graphics.axis.Axes' )
                    this.ActivePositionPropertyListeners{end+1,:} = ...
                        event.proplistener( child(n), ...
                        findprop( child, 'ActivePositionProperty' ), ...
                        'PostSet', @this.onActivePositionPropertyChanged );
                else
                    this.ActivePositionPropertyListeners{end+1,:} = [];
                end
            end
            %% Mark as dirty
            this.Dirty = true;
            
        end
        function removeChild(this,child)
            %implements removeChild
            
            %% handle hLabel
            if this.LabelPanel_hLabel_Contstructed
                ind = find(child==this.hLabel);
                if ~isempty(ind)
                    this.Contents_(1) = [];
                    this.ActivePositionPropertyListeners(1) = [];
                end
                child(ind) = [];
            end
            
            if isempty(child)
                return;
            end
            
            %% remove from contents
            [~,ind] = ismember(child,this.Contents_);
            this.Contents_(ind) = [];
            
            %% remove listeners
            ind = sort(reshape(ind,1,[]),'descend');
            for ii=ind
                try
                    delete(this.ActivePositionPropertyListeners{ii})
                catch
                end
                this.ActivePositionPropertyListeners(ii) = [];
            end
            
            %% Mark as dirty
            this.Dirty = true;
            
        end
    end
    
    
end