classdef AsyncProcessorWithWriterUI < extras.GraphicsChild & extras.widgets.mixin.ObjectDependentLifetime & extras.RequireGuiLayoutToolbox
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.

    %% 
    properties(SetAccess=private)
        Processor; %Handle to linked AsyncProcessorWithWriter
    end

    %% create
    methods
        function this = AsyncProcessorWithWriterUI(varargin)
        % Inputs:
        %   AsyncProcessorWithWriterUI(AsyncProcessorWithWriter_Object)
        %   AsyncProcessorWithWriterUI(GraphicsParent,__)
        %   AsyncProcessorWithWriterUI('Parent',GraphicsParent,...
        %                              'Processor',AsyncProcessor_Object)

            %% check inputs
            iH = extras.inputHandler;
            iH.addOptional('Parent',gobjects(0),@isgraphics,true);
            iH.addRequired('Processor',@(x) isa(x,'extras.Async.AsyncProcessorWithWriter'));
            
            iH.parse(varargin);
            
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
            if this.CreateParent
                this.ParentFigure.Name = this.Processor.Name;
            end
            
            %% Build GUI
            
            
            
            
        end
    end
    
end