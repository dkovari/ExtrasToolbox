classdef ProgressBar < extras.RequireGuiLayoutToolbox &...
        extras.RequireWidgetsToolbox & ...
        extras.widgets.LabelPanel & ...
        extras.widgets.mixin.AssignNV & ...
        extras.widgets.mixin.HasTooltip
% Progress Bar inside extras.widgets.LabelPanel
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.

    %% ProgressBar Public Properties
    properties (SetObservable, AbortSet)
        Progress (1,1) double {mustBeGreaterThanOrEqual(Progress,0),mustBeLessThanOrEqual(Progress,1)}= 0; %fractional progress [0,1]
        Message char = ''; %char array with message to display on progress bar
        MessageInterpreter = 'tex';
        Color = [0,118,168]/255;
    end
    methods
        function set.MessageInterpreter(this,val)
            this.MessageInterpreter = validatestring(val,{'tex','latex','none'});
        end
        function set.Color(this,val)
            if ~isnumeric(val)
                val = extras.colorname2rgb(val);
            end
            assert(isnmeric(val)&&numel(val)==3&&all(~isnan(val))&&all(val>=0&val<=1),'ForegroundColor was not a valid RGB array');
            this.Color = reshape(val,1,3);        
        end
    end
    
    %% Create Method
    methods
        function this = ProgressBar(varargin)
        % create progress bar
        
            %% listener for first time parent is set
            this.ProgressBar_ParentInitializedListener = addlistener(this,'Parent','PostSet',@(~,~) this.firstinitializeparent);
            
            %% set public args
            this.setPublicProperties(varargin{:});
        end
    end
    
    %% Graphics Elements
    properties(Access=protected)
        Progress_Axes
        Progress_Rectangle
        Progress_MessageText
    end
    
    %% firstinitializeparent
    properties(Access=private)
        ProgressBar_ParentInitialized (1,1) logical = false; %T/F if Parent has been initialized
        ProgressBar_ParentInitializedListener
    end
    methods(Access=private)
        function firstinitializeparent(this)
        % Builds gui elements in parent
            
            %% check if initialized already
            if this.ProgressBar_ParentInitialized
                return;
            end
            
            %% Construct GUI
            this.Progress_Axes = axes(...
                'Parent',this,...
                'NextPlot','add',...
                'Color','none',...
                'PickableParts','none',...
                'HandleVisibility','callback',...
                'Box','on',...
                'XLim',[0,1],...
                'XTick',[],...
                'XMinorTick','off',...
                'XColor','k',...
                'YLim',[0,1],...
                'YTick',[],...
                'YMinorTick','off',...
                'YColor','k');
            extras.expandAxes(this.Progress_Axes);
            
            this.Progress_Rectangle = rectangle(...
                this.Progress_Axes,...
                'Position',[0,0,this.Progress,1],...
                'FaceColor',this.Color,...
                'SelectionHighlight','off',...
                'EdgeColor','k');
            addlistener(this,'Progress','PostSet',@(~,~) set(this.Progress_Rectangle,'Position',[0,0,this.Progress,1]));
            addlistener(this,'Color','PostSet',@(~,~) set(this.Progress_Rectangle,'FaceColor',this.Color));
            xlim(this.Progress_Axes,[0,1]);
            
            this.Progress_MessageText = text(...
                'parent',this.Progress_Axes,...
                'String',this.Message,...
                'Color','k',...
                'Position',[0.5,0.5],...
                'HorizontalAlignment','center',...
                'VerticalAlignment','middle',...
                'SelectionHighlight','off',...
                'Interpreter',this.MessageInterpreter,...
                'PickableParts','none');
            uistack(this.Progress_MessageText,'top');
            
            addlistener(this,'Message','PostSet',@(~,~) set(this.Progress_MessageText,'String',this.Message));
            addlistener(this,'MessageInterpreter','PostSet',@(~,~) set(this.Progress_MessageText,'Interpreter',this.MessageInterpreter));    
                
            %% Construction complete, set flag
            delete(this.ProgressBar_ParentInitializedListener);
            this.ProgressBar_ParentInitialized = true;
        end
    end
    
    %% Display Customization
    methods (Access=protected)
        
        function propGroup = getPropertyGroups(this)
            propGroup = getPropertyGroups@extras.widgets.LabelPanel(this);
            
            %% ProgressBar
            titleTxt = sprintf(['\n\textras.widgets.ProgressBar Properties:',...
                                '\n\t--------------------------------------------']);
            
            thisProps = struct(...
                'Progress',this.Progress,...
                'Message',this.Message);
            
            propGroup = [propGroup,matlab.mixin.util.PropertyGroup(thisProps,titleTxt)];

        end %function
      
    end %Display Customization methods  
end