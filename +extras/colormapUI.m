classdef colormapUI < extras.GraphicsChild & extras.RequireGuiLayoutToolbox & extras.RequireWidgetsToolbox
% GUI for interactively building a colormap
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.

    %% Graphics Containers
    properties (Access=protected)
        OuterHBox;
        PlotVBox;
        CtrlVBox
        %CtrlAboveTableVBox;
        
        Table
        
        SpacerAboveAxis
        CmapAxes
        CAxisHeight = 125;
        CmapImage
        SpacerBelowAxis
        
        ImageHistogram;
        ImageHistXlimListener;
        
    end
    
    properties
        TableContext
        ColorbarContext
    end
    
    %% Linked image
    properties (SetObservable=true,AbortSet=true)
        Image = gobjects(1);
    end
    properties (SetAccess=protected)
        ImageDeleteListener;
        ImageCDataChangeListener;
    end
    methods
        function set.Image(this,val)
            
            %% setup image and image histogram
            assert(isgraphics(val)&&strcmpi(val.Type,'image'),'Image must be a valid image graphics object');
            
            delete(this.ImageDeleteListener);
            
            this.Image = val;
            
            %% delete listener
            try
                delete(this.ImageDeleteListener)
            catch
            end
            this.ImageDeleteListener = addlistener(this.Image,'ObjectBeingDestroyed',@(~,~) this.AssociatedImageDeleted);
            
            %% cdata listener
            try
                delete(this.ImageCDataChangeListener)
            catch
            end
            this.ImageCDataChangeListener = addlistener(this.Image,'CData','PostSet',@(~,~) this.updateColorbar());
              
            %% create imageHist
            if isempty(this.Image)
                this.AssociatedImageDeleted();
            else
                if isempty(this.ImageHistogram) || ~isvalid(this.ImageHistogram) %create ImageHist
                    this.ImageHistogram = extras.ImageHistogram(this.SpacerBelowAxis);
                    this.PlotVBox.Heights = [0,this.CAxisHeight,-1];
                    this.ImageHistogram.Image = this.Image;
                    
                    this.ImageHistXlimListener = addlistener(this.ImageHistogram.Axes,'XLim','PostSet',@(~,~) this.updateColorbar);
                    
                    xlabel(this.ImageHistogram.Axes,this.ValueLabelString);
                    
                    
                else
                    this.ImageHistogram.Image = this.Image;
                end
            end
            
            %% set cmap if needed
            if isempty(this.cmap.NodeValue) %hasn't be updated
                %% create cmap from current image colormap
                cm = this.Image.Parent.Colormap;
                NV = 1:size(cm,1);
                this.cmap.setColor(NV,cm);
            end

            %% Update
            this.cmapChanged();
        end
    end
    
    %% other
    properties(Dependent)
        clim
        cextent
    end
    methods
        function CEXT = get.cextent(this)
            if isgraphics(this.Image) %valid image
                
                CEXT = [min(this.cmap.clim(1),double(min(this.Image.CData(:)))),max(this.cmap.clim(2),double(max(this.Image.CData(:))))];
                %CLIM = [min(this.cmap.clim(1),this.Image.Parent.CLim(1)),max(this.cmap.clim(2),this.Image.Parent.CLim(2))];
            else
                CEXT = this.cmap.clim;
            end
        end
        function CLIM = get.clim(this)
           % if isgraphics(this.Image) %valid image
                
            %    CLIM = [min(this.cmap.clim(1),min(this.Image.CData(:))),max(this.cmap.clim(2),max(this.Image.CData(:)))];
                %CLIM = [min(this.cmap.clim(1),this.Image.Parent.CLim(1)),max(this.cmap.clim(2),this.Image.Parent.CLim(2))];
            %else
                CLIM = this.cmap.clim;
            %end
        end
    end
    
    %% public
    properties (SetObservable=true,AbortSet=true)
        cmap %= extras.colormapspline; %colormapspline object
        
        ValueLabel = 'Value'; %string specifying label for values
        ValueUnit = ''; %string specifying unit for value (empty means no unit)
        
        Title
    end
    methods (Access=protected)
        function updateValueLabel(this)
            try
                this.ImageHistogram.Axes.XLabel.String = this.ValueLabelString;
            catch %ME
                %disp(ME.getReport)
            end
            
            try
                this.Table.ColumnName{2} = this.ValueLabelString;
            catch %ME
                %disp(ME.getReport)
            end
            
            try
                this.CmapAxes.XLabel.String = this.ValueLabelString;
            catch %ME
                %disp(ME.getReport)
            end
        end
    end
    methods
        function set.Title(this,str)
            set(this.ParentFigure,'Name',str);
            this.Title=str;
        end
        function set.ValueLabel(this,str)
            assert(ischar(str),'ValueLabel must be a char array');
            this.ValueLabel = str;
            this.updateValueLabel;
        end
        function set.ValueUnit(this,str)
            assert(isempty(str)||ischar(str),'ValueUnit must be char array');
            this.ValueUnit = str;
            this.updateValueLabel();
        end
        function set.cmap(this,cmp)
            assert(isa(cmp,'extras.colormapspline'),'cmap must be a colormapspline');
            this.cmap = cmp;
            
            delete(this.cmapValuesListener);
            delete(this.cmapColorsListener);
            
            %% Create Listeners for changes to the colormap data
            this.cmapValuesListener = addlistener(this.cmap,'NodeValue','PostSet',@(~,~) this.cmapChanged);
            this.cmapColorsListener = addlistener(this.cmap,'NodeColor','PostSet',@(~,~) this.cmapChanged);
            
            %% Update
            this.cmapChanged();
            
        end 
    end
    
    properties(Dependent)
        ValueLabelString
    end
    methods
        function vs = get.ValueLabelString(this)
            vs = this.ValueLabel;
            if ~isempty(this.ValueUnit)
                vs = [vs,' [',this.ValueUnit,']'];
            end
        end
    end
    
    %% internal properties
    properties(Access=protected)
        cmapValuesListener;
        cmapColorsListener;
        
        DragLines = extras.DraggableLine.empty;
    end

    %% Create
    properties(Access=private)
        colormapUI_BeingContstructed = true;
    end
    methods
        function this = colormapUI(varargin)
        % create colormapUI interface
        %   colormapUI()
        %   colormapUI(parent,_): specify the parent graphics container
            
            %% initiate graphics parent related variables
            this@extras.GraphicsChild(@() figure('Name','ColormapUI',...
                    'NumberTitle','off',...
                    'MenuBar','none',...
                    'ToolBar','none',...
                    'HandleVisibility','callback'));
                
            %% Validate Parent
            %look for parent specified in arguments
            if nargin>0&&isnumeric(varargin{1}) %skip number as first input
                v2 = this.CheckParentInput(varargin{2:end});
                varargin = [varargin{1},v2];
            else
                varargin=this.CheckParentInput(varargin{:});
            end
            
            
            %% Build GUI
            this.OuterHBox = uix.HBox('parent',this.Parent);
            this.PlotVBox = uix.VBox('parent',this.OuterHBox);
            
            this.CtrlVBox = uix.VBox('parent',this.OuterHBox);
            
            hb = uix.HBox('parent',this.CtrlVBox);
            uicontrol(hb,...
                'style','pushbutton',...
                'string','Add Value',...
                'Callback',@(~,~) this.TableAdd);
            uicontrol(hb,...
                'style','pushbutton',...
                'string','Delete Selected',...
                'Callback',@(~,~) this.TableDelete);
            %this.CtrlAboveTableVBox = uix.VBox('parent',this.CtrlVBox); %panel above jtable
            %jtable
            
            this.OuterHBox.Widths = [-1,200];
            
            hFig = ancestor(this.Parent,'figure');
            
            %% Add Menu
            m = uimenu(hFig,'Text','Colorbar');
            uimenu(m,'Text','Load ValueMap','Callback',@(~,~) this.LoadValueMap);
            uimenu(m,'Text','Save ValueMap','Callback',@(~,~) this.SaveValueMap);
            %-----------------------------
            uimenu(m,'Text','Export ValueMap','Separator','on','Callback',@(~,~) this.ExportValueMap);
            uimenu(m,'Text','Export Colormap','Callback',@(~,~) this.ExportColormap);

            %% Build Table
            
            this.TableContext = uicontextmenu('Parent',hFig);
            uimenu(this.TableContext,'Label','Add Value','Callback',@(~,~) this.TableAdd);
            uimenu(this.TableContext','Label','Delete Selected','Callback',@(~,~) this.TableDelete);
            
            this.Table = uiw.widget.Table('Parent',this.CtrlVBox,...
                'ColumnName',{'',this.ValueLabelString,'Color'},...
                'ColumnPreferredWidth',[25,100,100],...
                'ColumnFormat',{'integer','numeric','color'},...
                'ColumnFormatData',{{},{},{' '}},...
                'ColumnEditable',logical([0,1,1]),...
                'CellEditCallback',@(h,e) this.tableEdit(h,e),...
                'SelectionMode','discontiguous',...
                'UIContextMenu',this.TableContext,...
                'Sortable',true);
            this.Table.ColumnMaxWidth(1)=25;
            
            this.CtrlVBox.Heights = [35,-1];
            
            
            %% colormap axes
            this.SpacerAboveAxis = uix.VBox('parent',this.PlotVBox);
            
            this.CmapAxes = axes(this.PlotVBox,'NextPlot','replacechildren','tickdir','both');
            this.CmapAxes.YAxis.Visible = 'off';
            xlabel(this.CmapAxes,this.ValueLabelString);
            
            this.CmapImage = image(this.CmapAxes,reshape([0,0,0],1,1,3));
            
            set(this.CmapImage,...
                'PickableParts','none',...
                'HitTest','off');
            
            
            
            %context menu
            this.ColorbarContext = uicontextmenu('parent',hFig);
            uimenu(this.ColorbarContext,'Label','Add Value','Callback',@(~,~) this.ColorbarAdd)
            
            this.CmapAxes.UIContextMenu = this.ColorbarContext;
            
            
            this.SpacerBelowAxis = uix.VBox('parent',this.PlotVBox);
            
            this.PlotVBox.Heights = [-1,this.CAxisHeight,-1];
                        
            %% create cmap
            this.cmap = extras.colormapspline(); %colormapspline object
                      
            %% colormapUI(__,[#,#,#...],Colors,...) --> pass num and colors to setColor
            if numel(varargin)>=2 && isnumeric(varargin{1})
                this.cmap.setColor(varargin{1:2});
                varargin(1:2) = [];
            end
            
            %% Set other properties
            set(this,varargin{:});
            
            %% construction flag
            this.colormapUI_BeingContstructed = false;
            
            %% final update
            this.cmapChanged();
        end
    end
    
    %% delete
    methods
        function delete(this)
            try
                delete(this.ContextMenu)
            catch
            end
            
            try
            delete(this.OuterHBox);
            catch
            end
            
            try
                delete(this.ImageHistXlimListener)
            catch
            end
            
            try
                delete(this.ImageDeleteListener)
            catch
            end
            
            %delete(this.ImageListener);
            %delete(this.ImageDeleteListener);
            
            try
                delete(this.ImageCDataChangeListener)
            catch
            end
            
            try
            delete(this.cmapValuesListener);
            delete(this.cmapColorsListener);
            catch
            end
        end
    end
    
    %% internal callback methods
    methods (Access=protected)
        function TableAdd(this)
            if this.colormapUI_BeingContstructed
                return;
            end
            
            this.cmap.NodeValue(end+1)=NaN;
            this.cmap.NodeColor(end+1,1:3)=[0,0,0];
        end
        
        function TableDelete(this)
            if this.colormapUI_BeingContstructed
                return;
            end
            
            idx = this.Table.SelectedRows;
            
            this.cmap.NodeValue(idx)=[];
            this.cmap.NodeColor(idx,:) = [];
            this.Table.SelectedRows = [];
        end
        
        function ColorbarAdd(this)
            if this.colormapUI_BeingContstructed
                return;
            end
            
            CP = this.CmapAxes.CurrentPoint;
            this.cmap.setColor(CP(1),[0,0,0]);
            %this.cmap.NodeValue(end+1) = CP(1);
            %this.cmap.NodeColor(end+1,1:3) = [0,0,0];
            
            idx = find(this.cmap.NodeValue==CP(1));
            this.Table.SelectedRows = idx;
        end
        
        function updateColorbar(this)
            %% Update Colorbar
            
            if this.colormapUI_BeingContstructed
                return;
            end
            
            %need to determine clim range
            CLIM = this.cextent;
            
            CB = zeros(1,3);
            try
               CB =  this.cmap.colormap(512,CLIM);
            catch %ME
                %disp(ME.getReport)
            end
            CB = repmat(CB,1,1,2);
            CB = permute(CB,[3,1,2]);
            this.CmapImage.CData = CB;
            this.CmapImage.XData = CLIM;
            this.CmapImage.YData = [0,1];
            set(this.CmapAxes,'XLim',CLIM,'YLim',[0,1]);
            
            %% Update DragLines
            for n=1:size(this.cmap.NodeValue,1)
                if isnan(this.cmap.NodeValue(n))
                    try
                        delete(this.DragLines(n))
                    catch
                    end
                elseif numel(this.DragLines)>= n && isvalid(this.DragLines(n)) %just update the line
                    this.DragLines(n).UserData.NodeIndex = n;
                    this.DragLines(n).X = [this.cmap.NodeValue(n),this.cmap.NodeValue(n)];
                else %need to create
                    this.DragLines(n) = extras.DraggableLine(this.CmapAxes,...
                        [this.cmap.NodeValue(n),this.cmap.NodeValue(n)],[NaN,NaN],...
                        'Color','k',...
                        'LineWidth',1.5,...
                        'Marker','d',...
                        'MarkerSize',8,...
                        'MarkerEdgeColor','k',...
                        'MarkerFaceColor','w',...
                        'UserData',struct('NodeIndex',n),...
                        'UIeditCallback',@(h,e) this.dragLineCallback(h,e));
                end
            end
            
            if numel(this.DragLines)>numel(this.cmap.NodeValue)
                % pause
                %'here'
                delete(this.DragLines(numel(this.cmap.NodeValue):end));
                this.DragLines(numel(this.cmap.NodeValue):end) = [];
            end
        end
        
        function cmapChanged(this)
        %called when cmap value or colors are changed
        
            if this.colormapUI_BeingContstructed
                return;
            end
            
            try
            %% Update the table
            nRows = max(size(this.cmap.NodeValue,1),size(this.cmap.NodeColor,1));
            DATA = cell(nRows,3);
            for n=1:nRows
                DATA{n,1} = n;
                DATA{n,3} = [0,0,0]; %default to black
            end

            for n = 1:size(this.cmap.NodeValue,1)
                DATA{n,2} = this.cmap.NodeValue(n);
            end
            
            for n=1:size(this.cmap.NodeColor,1)
                DATA{n,3} = this.cmap.NodeColor(n,:);
            end
            this.Table.Data = DATA;
            catch
            end
                   
            %% Update colorbar
            this.updateColorbar();
            
            %% Update Colormap for associated image
            if isgraphics(this.Image) %valid image
                
                %% update image display
                
                %xlim
                try
                this.ImageHistogram.Axes.XLim = this.cextent;
                catch
                end
                
                %image colormap
                try
                    CB = this.cmap.colormap(512,this.clim);
                    colormap(this.Image.Parent,CB);
                catch
                end
                
                % image clim
                try
                    set(this.Image.Parent,'CLim',this.clim);
                catch
                end
                
            end
            
            
        end
        function tableEdit(this,~,evt)
            editRow = evt.Indices(1);
            editCol = evt.Indices(2);
            switch editCol
                case 2 % value
                    try
                    this.cmap.NodeValue(editRow) = evt.NewValue;
                    
                    [NV,idx] = sort(this.cmap.NodeValue);
                    this.cmap.NodeValue = NV;
                    this.cmap.NodeColor = this.cmap.NodeColor(idx,:);
                    
                    id = find(this.cmap.NodeValue==evt.NewValue);
                    this.Table.SelectedRows = id;
                    
                    catch
                    end
                case 3 % color
                    this.cmap.NodeColor(editRow,:) = evt.NewValue/255;
                otherwise
                    warning('wrong col edit');
            end
            
        end
        
        function dragLineCallback(this,hl,~)
            idx = hl.UserData.NodeIndex;
            val = hl.X(1);
            
            try
                this.cmap.NodeValue(idx) = val;
            catch ME
                disp(ME.getReport);
            end
            
            this.Table.SelectedRows = idx;
        end
        
        function AssociatedImageDeleted(this)
            delete(this.ImageHistXlimListener);
            delete(this.ImageHistogram);
            delete(this.ImageCDataChangeListener);
            
            this.PlotVBox.Heights = [-1,this.CAxisHeight,-1];
        end
    end
    
    %% User Accessible
    methods
        function LoadValueMap(this,filepath,VarNames)
            persistent lastDir;
            persistent lastFile;
            
            if exist('filepath','var') && iscellstr(filepath) && ~exist('VarNames','var')
                VarNames = filepath;
                clear filepath;
            end
            
            if ~exist('filepath','var') || isempty(filepath)
                if isempty(lastDir)&&isempty(lastFile)
                    [f,p] = uigetfile('*.mat','Load Colormap Values From File');
                else
                    [f,p] = uigetfile('*.mat','Load Colormap Values From File',fullfile(lastDir,lastFile));
                end
                if f==0
                    return;
                end
                filepath = fullfile(p,f);
            end
            
            if ~exist('VarNames','var')
                VarNames{1} = 'Values';
                VarNames{2} = 'Colors';
            else
                assert(numel(VarNames)==2,'VarNames must contain two chars arrays');
                assert(iscellstr(VarNames),'VarNames must be a cellstr');
                VarNames{1} = genvarname(VarNames{1});
                VarNames{2} = genvarname(VarNames{2});
            end
            
            try
                S = load(filepath,VarNames{:});
            catch ME
                error('Could not load variables: %s, %s from file',VarNames{:});
            end
            
            this.cmap.NodeValue = S.(VarNames{1});
            this.cmap.NodeColor = S.(VarNames{2});
  
        end
        function outfile = SaveValueMap(this,filepath,VarNames)
            persistent lastDir;
            persistent lastFile;
            
            if exist('filepath','var') && iscellstr(filepath) && ~exist('VarNames','var')
                VarNames = filepath;
                clear filepath;
            end
            
            if ~exist('filepath','var') || isempty(filepath)
                if isempty(lastDir)&&isempty(lastFile)
                    [f,p] = uiputfile('*.mat','Save Colormap Values as...');
                else
                    [f,p] = uiputfile('*.mat','Save Colormap Values as...',fullfile(lastDir,lastFile));
                end
                if f==0
                    outfile = [];
                    if nargout<1
                        clear outfile;
                    end
                    return;
                end
                filepath = fullfile(p,f);
                lastFile = f;
                lastDir = p;
            end
            
            if ~exist('VarNames','var')
                VarNames{1} = 'Values';
                VarNames{2} = 'Colors';
            else
                VarNames{1} = genvarname(VarNames{1});
                VarNames{2} = genvarname(VarNames{2});
            end
            
            %% put values and colors into variable specified by names
            S.(VarNames{1}) = this.cmap.NodeValue;
            S.(VarNames{2}) = this.cmap.NodeColor;
            
            %% save to file
            if ~exist(filepath,'file')
                save(filepath,'-struct','S');
            else
                save(filepath,'-struct','S','-append');
            end
            
            outfile = filepath;
            if nargout<1
                clear outfile;
            end
            
        end
        function [Values,Colors] = ExportValueMap(this,VarNames)
            Values = this.cmap.NodeValue;
            Colors = this.cmap.NodeColor;
            
            if nargout < 1
                if ~exist('VarNames','var')
                    VarNames = extras.inputdlg({'Values Name:','Colors Name:'},'Variable Names');
                    if isempty(VarNames) || isempty(VarNames{1}) || isempty(VarNames{2})
                        clear Values Colors
                        return;
                    end   
                else
                    assert(iscellstr(VarNames),'VarNames must be cellstr with output variable names');
                end
            
                VarNames{1} = genvarname(VarNames{1});
                VarNames{2} = genvarname(VarNames{2});

                %% check for conflicts
                basevars = evalin('base','who');
                for n=1:numel(VarNames)
                    if ismember(VarNames{n},basevars)
                      warning('ExportColormap:overwrite', ...
                        [varname, ...
                        ' already exists in the base workspace. It will be overwritten.']);
                    end

                end
                assignin('base',VarNames{1},Values);
                assignin('base',VarNames{2},Colors);
                clear Values Colors;
            end
        end
        function CMAP = ExportColormap(this,len,VarName)
            
            if ~exist('len','var')
                len = 512;
            end
            
            CMAP = this.cmap.colormap(len);
            
            if nargout < 1
                if ~exist('VarName','var')
                    VarName = extras.inputdlg('Export Colormap as:','Variable Name',1);
                    if isempty(VarName) ||isempty(VarName{1})
                        clear CMAP;
                        return;
                    end
                    VarName = VarName{1};
                else
                    assert(ischar(VarName),'VarName must be a char array');
                end
                varname=genvarname(VarName);
                
                %% check for conflicts
                basevars = evalin('base','who');
                if ismember(varname,basevars)
                  warning('ExportColormap:overwrite', ...
                    [varname, ...
                    ' already exists in the base workspace. It will be overwritten.']);
                end
                assignin('base',varname,CMAP);
                clear CMAP;
            end
        end        
    end

end
