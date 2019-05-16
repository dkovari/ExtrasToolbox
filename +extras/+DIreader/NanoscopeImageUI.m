classdef NanoscopeImageUI < extras.GraphicsChild
    
    %% Graphics Related
    properties (SetAccess=protected)
        TabGroup
        Axes = gobjects(0);
        Image = gobjects(0);
        
        UndoMenu
        RedoMenu
        
        Scalebar = extras.scalebar.empty;
        ColormapList = {};
    end
    properties (Dependent)
        ActiveImageID
    end
    methods
        function idx = get.ActiveImageID(this)
            idx = this.TabGroup.SelectedTab.UserData.ImageIndex;
        end
    end
    
    %% File Related
    properties (SetObservable=true,AbortSet=true)
        NSpathname;
        NSfilename;
        NSfilepath;
        
        LastExportImagePath;
        
    end
    properties(SetAccess=protected,SetObservable=true,AbortSet=true)
        NS_data;
    end
    
    %% create
    methods
        function this = NanoscopeImageUI(varargin)
            %% initiate graphics parent related variables
            this@extras.GraphicsChild(@() figure('Name','NanoscopeUI',...
                    'NumberTitle','off',...
                    'MenuBar','none',...
                    'ToolBar','figure',...
                    'HandleVisibility','callback'));
                
            %% Validate Parent
            %look for parent specified in arguments
            varargin = this.CheckParentInput(varargin{:});
            
            %% disable some of the toolbar controls
            if this.CreatedParent
                %New Figure
                delete(findall(this.ParentFigure,'ToolTipString','Save Figure'));
                %New Figure
                delete(findall(this.ParentFigure,'ToolTipString','New Figure'));
                %Open File
                delete(findall(this.ParentFigure,'ToolTipString','Open File'));
                %Print Figure
                delete(findall(this.ParentFigure,'ToolTipString','Print Figure'));
                %Edit Plot
                delete(findall(this.ParentFigure,'ToolTipString','Edit Plot'));
                %Rotate 3D
                delete(findall(this.ParentFigure,'ToolTipString','Rotate 3D'));
                %Data Cursor
                delete(findall(this.ParentFigure,'ToolTipString','Data Cursor'));
                %Brush/Select Data
                delete(findall(this.ParentFigure,'ToolTipString','Brush/Select Data'));
                %Link Plot
                delete(findall(this.ParentFigure,'ToolTipString','Link Plot'));
                %Insert Legend
                delete(findall(this.ParentFigure,'ToolTipString','Insert Legend'));
                %Hide Plot Tools
                delete(findall(this.ParentFigure,'ToolTipString','Hide Plot Tools'));
                %Show Plot Tools and Dock Figure
                delete(findall(this.ParentFigure,'ToolTipString','Show Plot Tools and Dock Figure'));
            end
            
            %% Create TabPanel
            this.TabGroup = uitabgroup(this.Parent);
            %% Create menus
            
            % File
            fm = findall(this.ParentFigure,'Text','&File');
            fm = [fm,findall(this.ParentFigure,'Text','File')];
            if isempty(fm)
                fm = uimenu(this.ParentFigure,'Text','&File','Tag','figMenuFile');
            end
            fm=fm(1);
            uimenu(fm,'Text','Open Nanoscope File',...
                'Separator','on',...
                'Callback',@(~,~) this.openNSFile);
            %--------------------
            uimenu(fm,'Text','Export Image File',...
                'Separator','on',...
                'Callback',@(~,~) this.exportImage);
%             % Edit
%             em = findall(this.ParentFigure,'Text','&Edit');
%             em = [em,findall(this.ParentFigure,'Text','Edit')];
%             if isempty(em)
%                 em = uimenu(this.ParentFigure,'Text','&Edit','Tag','figMenuEdit');
%             end
%             this.UndoMenu = uimenu(em,'Text','Undo',...
%                 'Callback',@(~,~) this.undoLastOp);
%             this.RedoMenu = uimenu(em,'Text','Redo',...
%                 'Callback',@(~,~) this.redoLastOp);
            
            % Data
            dm = findall(this.ParentFigure,'Text','Data');
            if isempty(dm)
                dm = uimenu(this.ParentFigure,'Text','Data','Tag','figMenuData');
            end
            dm=dm(1);
            uimenu(dm,'Text','Export to WS','Callback',@(~,~) this.exportData)
            
            % view
            vm = findall(this.ParentFigure,'Text','&View');
            vm = [vm,findall(this.ParentFigure,'Text','View')];
            if isempty(vm)
                vm = uimenu(this.ParentFigure,'Text','&Tools','Tag','figMenuTools');
            end
%             uimenu(vm,'Text','Show Scalebar',...
%                 'Separator','on',...
%                 'Checked','off',...
%                 'Callback',@(~,~) this.showScalebar);
            
            % Tools
            tm = findall(this.ParentFigure,'Text','&Tools');
            tm = [tm,findall(this.ParentFigure,'Text','Tools')];
            if isempty(tm)
                tm = uimenu(this.ParentFigure,'Text','&Tools','Tag','figMenuTools');
            end
            uimenu(tm,'Text','Colormap Editor',...
                'Separator','on',...
                'Callback',@(~,~) this.showColormapUI);
     
            %% Look for filepath, load file
            fileidx = find(strcmpi('File',varargin));
            if ~isempty(fileidx)
                assert(numel(fileidx)==1,'''File'' specified more than once');
                assert(numel(varargin)>fileidx,'''File'' must be accompanied by a string specifying filepath');
                this.openNSfile(varargin{fileidx+1})
                varargin(fileidx:fileidx+1) = [];
            else
                this.openNSFile();
            end
                        
        end
    end
    
    methods (Access=protected)
        function buildPanels(this)
            %% delete any existing plots
            try
                delete(this.Scalebar)
                this.Scalebar = extras.scalebar.empty;
            catch
            end
            
            try
                for n =1:numel(this.ColormapList)
                    delete(this.ColormapList{n});
                end
                this.ColormapList = cell(1,numel(this.NS_data));
            catch
            end
            
            try
                delete(this.Image)
            catch
            end
            try
                delete(this.Axes)
            catch
            end
            
            try
                delete(this.TabGroup.Children)
            catch
            end
            
            %% create axes and images for each image in the ns file
            for n=1:numel(this.NS_data)
                uit = uitab(this.TabGroup,'Title',this.NS_data(n).type,'UserData',struct('ImageIndex',n));
                this.Axes(n) = axes('Parent',uit,...
                    'NextPlot','add',...
                    'HandleVisibility','callback');
                this.Image(n) = image('Parent',this.Axes(n),...
                    'CDataMapping','scaled',...
                    'CData',this.NS_data(n).ImageData,...
                    'XData',[0,this.NS_data(n).width],...
                    'YData',[0,this.NS_data(n).height]);
                axis(this.Axes(n),'image');
                this.Axes(n).XAxis.Visible = ' off';
                this.Axes(n).YAxis.Visible = ' off';
                try
                set(this.Axes(n),'LooseInset',get(this.Axes(n),'TightInset'));
                catch
                end
                
                %% Create Scalebar
                this.Scalebar(n) = extras.scalebar(this.Axes(n),...
                    'Scale',1,...
                    'Length',this.NS_data(n).width/10,...
                    'Unit',this.NS_data(n).unit,...
                    'Color','w',...
                    'location','southeast');
                this.Scalebar(n).Text.FontSize = 12;
            end
        end
    end
    
    %% Callbacks
    methods
        function showColormapUI(this)
            n = this.ActiveImageID;
            
            %Check for colormapspline
            if ~isfield(this.NS_data,'cmap') || isempty(this.NS_data(n).cmap) || ~isvalid(this.NS_data(n).cmap)
                this.NS_data(n).cmap = extras.colormapspline(...
                    [min(this.NS_data(n).ImageData(:)),median(this.NS_data(n).ImageData(:)),max(this.NS_data(n).ImageData(:))],...
                    {'b','r','w'});
            end
            
            if numel(this.ColormapList)<n || isempty(this.ColormapList{n}) || ~isvalid(this.ColormapList{n}) %create colormapUI
                this.ColormapList{n} = ...
                    extras.colormapUI(...
                    'cmap',this.NS_data(n).cmap,...
                    'Image',this.Image(n),...
                    'Title',this.NS_data(n).type);
            else
                figure(this.ColormapList{n}.ParentFigure);
            end
        end
        
        function NSdata = exportData(this,VarName)
            
            NSdata = this.NS_data;
            
            if nargout<1
                if ~exist('VarName','var')
                    VarName = extras.inputdlg('Export Data as:','Variable Name',1);
                    if isempty(VarName) ||isempty(VarName{1})
                        clear NSdata;
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
                  warning('exportData:overwrite', ...
                    [varname, ...
                    ' already exists in the base workspace. It will be overwritten.']);
                end
                assignin('base',varname,NSdata);
                %% clear output var
                clear NSdata;
            end
            
        end
        
        function openNSFile(this,filepath)
            if ~exist('filepath','var')
                filepath = [];
            end
            [NSdata,filepath] = extras.DIreader.NanoscopeLoader('FilePath',filepath,'scale_nm',true);
            if ~isempty(NSdata)
                this.NS_data = NSdata;
                [this.NSpathname,f,e] = fileparts(filepath);
                this.NSfilename = [f,e];
                this.NSfilepath = filepath;
                
                %% rebuild gui
                this.buildPanels
            end
        end
        exportImage(this)
    end
end