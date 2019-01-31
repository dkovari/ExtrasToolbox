classdef colormapspline < matlab.mixin.SetGet
%colormapspline dynamic colormap which interpolates between successive
%value-color pairs.

    
    properties (SetObservable=true,AbortSet=true)
        NodeValue = []; % colormap index points
        NodeColor = zeros(0,3); %colormap colors corresponding to the NodeValue index points
        
        Mode = 'RGB'; %specifies whether to interpolate between colors values in HSV or RGB color spaces
    end
    methods
        function set.NodeValue(this,nv)
            assert(isvector(nv),'NodeValue must be a vector');
            assert(isnumeric(nv),'NodeValue must be numeric');
            this.NodeValue = reshape(nv,[],1);
            
            this.updatepchip();
        end
        function set.NodeColor(this,color)
            assert(isnumeric(color),'NodeColor must be numeric');
            if numel(color)==3
                color = reshape(color,1,3);
            end
            assert(size(color,2)==3,'NodeColor must be [n x 3], RGB color');
            
            assert(all(all(color>=0&color<=1)),'NodeColor must be valid RGB color in 0-1 range');

            
            this.NodeColor = color;
            
            this.updatepchip();
        end
        
        function setColor(this,value,color)
        %sepecify the color at a corresponding value
        
            assert(isvector(value),'value must be a vector');
            assert(isnumeric(value),'value must be numeric');
            
            
            value = reshape(value,[],1);
            
            if isnumeric(color) && numel(color)==3
                color = reshape(color,1,3);
            end
            
            if ischar(color)
                color = {color};
            end
            
            assert( isnumeric(color)&&size(color,1)==numel(value) || numel(color)==numel(value), 'Number of values must match number of specified colors');
            
            color_num = NaN(numel(value),3);
            if iscell(color)
                for n=1:numel(color)
                    if isnumeric(color{n})
                        if numel(color{n})~=3
                            warning('color value was numeric but not RGB');
                        else
                            color_num(n,:) = color{n};
                        end
                    else
                        color_num(n,:) = extras.colorname2rgb(color{n});
                    end
                end
            else
                color_num = color;
            end
            
            %% add or change color values
            [oldKeep,oldKeepI] = setdiff(this.NodeValue,value);
            
            newValues = [value;oldKeep];
            newColors = [color_num;this.NodeColor(oldKeepI,:)];
            
            [this.NodeValue,sI] = sort(newValues);
            this.NodeColor = newColors(sI,:);

        end
        
        function set.Mode(this,mode)
            assert(ischar(mode),'Mode must be a string');
            mode = upper(mode);
            assert(ismember(mode,{'HSV','RGB'}),'Mode must be either ''HSV'' or ''RGB''');
            this.Mode = mode;
            this.updatepchip;
            
        end
    end
        
    properties (SetAccess=protected,SetObservable=true,AbortSet=true)
        cmapspline; %piecewise cubic hermit spline interpolating between Node points
        clim = [0,1]; %color limits corresponding to extent of NodeValue
    end
    
    methods
        function this = colormapspline(values,colors)
        %Create a colormapspline object
        % optionally specify values and colors for the control points
            
            if nargin==1
                error('values and colors must be specified');
            end
            
            if nargin>1
                this.setColor(values,colors);
            end
            
        end
        
        function cmap = colormap(this,nSteps,clim)
        % cmap = obj.colormap() - returns colormap as numeric array
        %  obj.colormap(nSteps,clim)
        %       nSteps (default=255), number of different levels to include
        %       in the resulting numeric colormap array
        %       clim (default=this.clim), range over which to generate cmap
        
            if isempty(this.cmapspline)
                error('Colormap Spline has not be updated. NodeValue or NodeColor is probably empty.');
                %cmap = zeros(0,3);
                %return;
            end
        
            if nargin<2
                nSteps = 255;
            end
            
            if nargin<3
                clim = this.clim;
            end
            
            vv = linspace(clim(1),clim(2),nSteps);
            
            cmap = ppval(this.cmapspline,vv)';
            
            if strcmpi(this.Mode,'HSV')
                cmap = hsv2rgb(cmap);
            end
            
            
 
        end
    end
    
    %% internal usage only
    methods (Access=protected)
        function updatepchip(this)
            %update the spline after changing node or color values
            
            if numel(this.NodeValue)<2|| size(this.NodeColor,1)<2
                warning('Nodes are empty, will not update spline');
                return;
            end
            
            if size(this.NodeValue,1)==size(this.NodeColor,1)
                
                [NV,idx] = sort(this.NodeValue);
                CV = this.NodeColor(idx,:);
                NV = [NV(1)-1;NV;NV(end)+1];
                CV = [CV(1,:);CV;CV(end,:)];
                
                if strcmpi(this.Mode,'HSV')
                    this.cmapspline = pchip(NV',rgb2hsv(CV)');
                else
                    this.cmapspline = pchip(NV',CV');
                end
                
                this.clim = [min(this.NodeValue),max(this.NodeValue)];
            end
            
        end
    end
end

