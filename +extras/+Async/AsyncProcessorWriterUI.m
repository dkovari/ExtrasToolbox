classdef AsyncProcessorWriterUI < extras.GraphicsChild & extras.widgets.mixin.ObjectDependentLifetime & extras.RequireGuiLayoutToolbox
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.

    %% 
    properties(SetAccess=private)
        Processor; %Handle to linked AsyncProcessorWithWriter
    end
    
    %% GUI elements
    properties(Access=private)
        FileSelector
        SaveResultsCheckbox
        ResultsFileOpenField
    end
    
    %% listeners
	properties(Access=private)
        FileChangeListener
        isResultsFileOpenListener
        isResultsWriterRunningListener
        resultsWaitingToBeWrittenListener
        SaveResultsListener
        isResultsWriterRunningField
        resultsWaitingToBeWrittenField
    end

    %% create
    methods
        function this = AsyncProcessorWriterUI(varargin)
        % Inputs:
        %   AsyncProcessorWithWriterUI(AsyncProcessorWithWriter_Object)
        %   AsyncProcessorWithWriterUI(GraphicsParent,__)
        %   AsyncProcessorWithWriterUI('Parent',GraphicsParent,...
        %                              'Processor',AsyncProcessor_Object)

            %% check inputs
            iH = extras.inputHandler;
            iH.addOptionalVariable('Parent',gobjects(0),@isgraphics,true);
            iH.addRequiredVariable('Processor',@(x) isa(x,'extras.Async.AsyncProcessorWriter'),true);
            
            iH.parse(varargin{:});
            
            Parent = iH.Results.Parent;
            Processor =iH.Results.Processor;
            
            %% Setup ObjectDependentLifetime
            this@extras.widgets.mixin.ObjectDependentLifetime(Processor);
            
            %% setup Graphics
            this@extras.GraphicsChild(@() ...
                figure(...
                    'Name','AsyncProcessorWithWriter UI',...
                    'NumberTitle','off',...
                    'ToolBar','none',...
                    'MenuBar','none',...
                    'HandleVisibility','Callback')...
                );
            
            %create parent fig
            if ~isempty(Parent)
                 this.CheckParentInput(Parent);
            else
                this.CheckParentInput();
            end
            
            %% change name of figure if needed
            this.Processor = Processor;
            if this.CreatedParent
                this.ParentFigure.Name = this.Processor.Name;
            end
            
            %% Build GUI
            
            Pnl = uix.Panel('Parent',this.Parent,'Title','Results Writer');
            vbox = uix.VBox('Parent',Pnl);
            
            % Save Results checkbox
            this.SaveResultsCheckbox = extras.widgets.UIControl(...
                'Parent',vbox,...
                'Style','checkbox',...
                'Label','Save Results:',...
                'LabelWidth',-0.25,...
                'Spacing',2,...
                'LabelHorizontalAlignment','right',...
                'Value',this.Processor.SaveResults,...
                'Callback',@(h,~) set(this.Processor,'SaveResults',~this.Processor.SaveResults));
            
            this.SaveResultsListener = addlistener(this.Processor,'SaveResults','PostSet',@(~,~) extras.inline_try(@() set(this.SaveResultsCheckbox,'Value',this.Processor.SaveResults)));
            
            %File selector
            this.FileSelector = extras.widgets.FileSelector('Parent',vbox,...
                'SelectionBehavior','putfile',...
                'Filter','*.mxf.gz',...
                'SelectionTitle',sprintf('%s Results File',this.Processor.Name),...
                'Tooltip','Results File',...
                'Label','Results File:',...
                'LabelWidth',-0.25,...
                'Spacing',2,...
                'LabelHorizontalAlignment','right',...
                'Filepath',this.Processor.ResultsFilepath,...
                'Callback',@(~,~) this.UIchangeFile);
            
            this.FileChangeListener = addlistener(this.Processor,'ResultsFilepath','PostSet',@(~,~) extras.inline_try(@() set(this.FileSelector,'Filepath',this.Processor.ResultsFilepath)));
            
            %ResultsFileOpen
            this.ResultsFileOpenField = extras.widgets.UIControl(...
                'Parent',vbox,...
                'Label','Results File Open:',...
                'LabelWidth',-0.25,...
                'Spacing',2,...
                'LabelHorizontalAlignment','right',...
                'Style','text',...
                'String',num2str(this.Processor.isResultsFileOpen));
            
            this.isResultsFileOpenListener = addlistener(this.Processor,'isResultsFileOpen','PostSet',@(~,~) extras.inline_try(@() set(this.ResultsFileOpenField,'string',num2str(this.Processor.isResultsFileOpen))));
            
            %writer running
            this.isResultsWriterRunningField = extras.widgets.UIControl(...
                'Parent',vbox,...
                'Label','Results Writer Running:',...
                'LabelWidth',-0.25,...
                'Spacing',2,...
                'LabelHorizontalAlignment','right',...
                'Style','text',...
                'String',num2str(this.Processor.isResultsWriterRunning));
            
            this.isResultsWriterRunningListener = addlistener(this.Processor,'isResultsWriterRunning','PostSet',@(~,~) extras.inline_try(@() set(this.isResultsWriterRunningField,'string',num2str(this.Processor.isResultsWriterRunning))));
            
            %resultsWaitingToBeWritten
            %writer running
            this.resultsWaitingToBeWrittenField = extras.widgets.UIControl(...
                'Parent',vbox,...
                'Label','Unsaved Results:',...
                'LabelWidth',-0.25,...
                'Spacing',2,...
                'LabelHorizontalAlignment','right',...
                'Style','text',...
                'String',num2str(this.Processor.resultsWaitingToBeWritten));
            
            this.resultsWaitingToBeWrittenListener = addlistener(this.Processor,'resultsWaitingToBeWritten','PostSet',@(~,~) extras.inline_try(@() set(this.resultsWaitingToBeWrittenField,'string',num2str(this.Processor.resultsWaitingToBeWritten))));
            
            vbox.Heights = repmat(35,size(vbox.Heights));
        end
    end
    
    %% delete
    methods
        function delete(this)
            delete(this.FileSelector);
            delete(this.FileChangeListener);
            
            delete(this.isResultsFileOpenListener);
            delete(this.isResultsWriterRunningListener);
            delete(this.resultsWaitingToBeWrittenListener);
            delete(this.SaveResultsListener);
        end
    end
    
    %% Callbacks
    methods(Access=private)
        function UIchangeFile(this)
            this.Processor.openResultsFile(this.FileSelector.Filepath);
        end
    end
end