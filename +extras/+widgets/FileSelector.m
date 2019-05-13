classdef FileSelector < extras.RequireGuiLayoutToolbox &...
        extras.RequireWidgetsToolbox & ...
        extras.widgets.LabelPanel & ...
        extras.widgets.mixin.HasCallback & ...
        extras.widgets.mixin.AssignNV & ...
        extras.widgets.mixin.HasTooltip
% Widget for creating a text field and file selection button.

    %% Create
    methods
        function this = FileSelector(varargin)
            %create extras.widgets.FileSelector
            % hCtrl = extras.widgets.FileSelector(Name,Value)
            
            %% listener for first time parent is set
            this.parentlistener = addlistener(this,'Parent','PostSet',@(~,~) this.create_graphics);
            
            %% Other Listeners
            addlistener(this,'Tooltip','PostSet',@(~,~) extras.inline_try(@() set(this.FilepathField,'Tooltip',this.Tooltip)));

            addlistener(this,'UIFileChanged',@(h,e) this.callCallback(e));
            
            addlistener(this,'Filepath','PostSet',@(~,~) extras.inline_try(@() set(this.FilepathField,'Value',this.Filepath)));
            
            %% set public args
            this.setPublicProperties(varargin{:});
        end
    end
    
    %% delete
    methods
        function delete(this)
            delete(this.FilepathField);
            delete(this.SelectionButton);
            delete(this.VBox);
        end
    end
    

    %% File Properties
    properties(SetObservable,AbortSet)
        Filepath char = ''; %char array specifying file path selected
        FileName char = ''; %char array specifying name (w/o extension) of file
        FileExtension char = ''; %file extension of specified file
        FileDir char = ''; %directory of specified file
        Filter = '*.*'; %File filter options used by uigetfile when user pressed [...] button
        SelectionTitle = 'Choose File'; %string displayed at top of uigetfile window
        DefaultFilepath = ''; %default name used by uigetfile
        AutoChangeDefaultName (1,1) logical = true; %T/F specifying if DefaultName should automatical be changed to last selection
        SelectionBehavior = 'putfile'; %'putfile' or 'getfile' indicating if uiputfile or uigetfile are used
    end
    properties(SetAccess=protected,SetObservable,AbortSet)
        FilterIndex %index of the selected filter
    end
    properties(Access=private)
        internal_set_Filepath = false;
        internal_set_FileName = false;
        internal_set_FileExtension = false;
        internal_set_FileDir = false;
    end
    methods
        function set.SelectionBehavior(this,value)
            this.SelectionBehavior = char(validatestring(value,{'putfile','getfile'}));
        end
        function set.Filepath(this,value)
            if this.internal_set_Filepath
                this.Filepath = value;
                return;
            end
            
            [P,F,E] = fileparts(value);
            
            this.Filepath = value;
            
            this.internal_set_FileName = true;
            this.internal_set_FileExtension = true;
            this.internal_set_FileDir = true;
            
            this.FileName = F;
            this.FileExtension = E;
            this.FileDir = P;
            
            if this.AutoChangeDefaultName
                this.DefaultFilepath = value;
            end
            
            this.internal_set_FileName = false;
            this.internal_set_FileExtension = false;
            this.internal_set_FileDir = false;
        end
        
        function set.FileName(this,value)
            if this.internal_set_FileName
                this.FileName = value;
                return;
            end
            
            this.internal_set_Filepath = true;
            %this.internal_set_FileName = true;
            %this.internal_set_FileExtension = true;
            %this.internal_set_FileDir = true;
            
            this.FileName = value;
            this.Filepath = fullfile(this.FileDir,[this.FileName,this.FileExtension]);
            %this.FileExtension = E;
            %this.FileDir = P;
            
            if this.AutoChangeDefaultName
                this.DefaultFilepath = this.Filepath;
            end
            
            this.internal_set_Filepath = false;
            %this.internal_set_FileName = false;
            %this.internal_set_FileExtension = false;
            %this.internal_set_FileDir = false;
        end
        
        function set.FileExtension(this,value)
            if this.internal_set_FileExtension
                this.FileExtension = value;
                return;
            end
            
            %% append '.' if not included or empty
            if isempty(value)
                value = '';
            else
                if value(1)~='.'
                    value = ['.',value];
                end
            end
            
            this.internal_set_Filepath = true;
            %this.internal_set_FileName = true;
            %this.internal_set_FileExtension = true;
            %this.internal_set_FileDir = true;
            
            this.FileExtension = value;
            this.Filepath = fullfile(this.FileDir,[this.FileName,this.FileExtension]);
            %this.FileExtension = E;
            %this.FileDir = P;
            
            if this.AutoChangeDefaultName
                this.DefaultFilepath = this.Filepath;
            end
            
            this.internal_set_Filepath = false;
            %this.internal_set_FileName = false;
            %this.internal_set_FileExtension = false;
            %this.internal_set_FileDir = false;
        end
        
        function set.FileDir(this,value)
            if this.internal_set_FileDir
                this.FileDir = value;
                return;
            end
            
            
            this.internal_set_Filepath = true;
            %this.internal_set_FileName = true;
            %this.internal_set_FileExtension = true;
            %this.internal_set_FileDir = true;
            
            this.FileDir = value;
            this.Filepath = fullfile(this.FileDir,[this.FileName,this.FileExtension]);
            %this.FileExtension = E;
            %this.FileDir = P;
            
            if this.AutoChangeDefaultName
                this.DefaultFilepath = this.Filepath;
            end
            
            this.internal_set_Filepath = false;
            %this.internal_set_FileName = false;
            %this.internal_set_FileExtension = false;
            %this.internal_set_FileDir = false;
        end
    end
    
    events
        UIFileSelectionStarted
        UIFileSelectionComplete
        UIFileChanged
    end
    
    %% Display Customization
    methods (Access=protected)
        
        function propGroup = getPropertyGroups(this)
            propGroup = getPropertyGroups@extras.widgets.LabelPanel(this);
        
            %% FileSelector
            titleTxt = sprintf(['\n\textras.widgets.FileSelector FileSelector Properties:',...
                                '\n\t-----------------------------------------------------']);
            thisProps = struct(...
                'Filepath',this.Filepath,...
                'FileName',this.FileName,...
                'FileExtension',this.FileExtension,...
                'FileDir',this.FileDir,...
                'Filter',this.Filter,...
                'FilterIndex',this.FilterIndex,...
                'SelectionTitle',this.SelectionTitle,...
                'DefaultFilepath',this.DefaultFilepath,...
                'AutoChangeDefaultName',this.AutoChangeDefaultName,...
                'SelectionBehavior',this.SelectionBehavior,...
                'Tooltip',this.Tooltip);
            propGroup = [propGroup,matlab.mixin.util.PropertyGroup(thisProps,titleTxt)];

        end %function
      
    end %Display Customization methods  
    
    %% Internal Graphics
    properties(Access=private)
        VBox %vbox used to fix vertical spacing
        HBox %hbox holding field and [...] button
        FilepathField
        SelectionButton
    end
    
    %% internal graphics creation method
    %% Internal use only
    properties(Access=private)
        parentlistener
        parentFirstSet = false;
    end
    methods(Access=private)
        function create_graphics(this)
            %called first time parent is set
            % creates gui elements
            if this.parentFirstSet
                return;
            end
            
            %% Create VBox & HBox
            this.VBox = uix.VBox('Parent',this);
            uix.Empty('Parent',this.VBox);
            this.HBox = uix.HBox('Parent',this.VBox);
            uix.Empty('Parent',this.VBox);
            this.VBox.Heights = [-1,25,-1];
            
            %% File Field
            this.FilepathField = extras.widgets.ValueControl(...
                'Parent',this.HBox,...
                'ValueType','string',...
                'RememberValueHistory',true,...
                'Value',this.Filepath,...
                'Tooltip',this.Tooltip,...
                'Callback',@(~,~) this.FieldCallback());
            
            %% Selection button
            this.SelectionButton = uicontrol(...
                'Parent',this.HBox,...
                'Style','pushbutton',...
                'String','...',...
                'TooltipString','Select File',...
                'HandleVisibility','callback',...
                'Callback',@(~,~) this.ButtonCallback());
            
            %% set HBox size
            this.HBox.Widths = [-1,25];
            
            %% get rid of parent listener
            try
               delete(this.parentlistener)
            catch
            end
            %% set flag
            this.parentFirstSet = true;
        end
    end
    
    %% Callbacks
    methods(Access=private)
        function FieldCallback(this)
            orig_file = this.Filepath;
            
            this.Filepath = this.FilepathField.Value;

            %% fire event
            if ~strcmpi(orig_file,this.Filepath)
                notify(this,'UIFileChanged');
            end
        end
        function ButtonCallback(this)
            %% start event
            notify(this,'UIFileSelectionStarted');
            
            orig_file = this.Filepath;
            
            %% Launch file selection gui
            if strcmpi('putfile',this.SelectionBehavior)
                %% PUT FILE
                [file,path,idx] = uiputfile(this.Filter,this.SelectionTitle,this.DefaultFilepath);
                if file~=0
                    this.Filepath = fullfile(path,file);
                    this.FilterIndex = idx;
                end
            else
                %% GET FILE
                [file,path,idx] = uigetfile(this.Filter,this.SelectionTitle,this.DefaultFilepath);
                if file~=0
                    this.Filepath = fullfile(path,file);
                    this.FilterIndex = idx;
                end
            end
            
            %% fire event
            notify(this,'UIFileSelectionComplete');
            if ~strcmpi(orig_file,this.Filepath)
                notify(this,'UIFileChanged');
            end
        end
    end
end