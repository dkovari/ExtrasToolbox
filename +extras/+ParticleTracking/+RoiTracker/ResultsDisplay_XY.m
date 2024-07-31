classdef ResultsDisplay_XY < handle & extras.GraphicsChild & extras.widgets.mixin.ObjectDependentLifetime
%Display scatter plot of points on axes showing realtime results for XY
%coordinates
%
%   NOTE: This class requires the CameraToolbox be installed (see
%   github.com/dkovari/CameraToolbox)
    
    %% Props
    properties(SetAccess=private,SetObservable,AbortSet)
        Tracker; %handle to tracker object
        CameraDisplay = CameraToolbox.Display.empty();
    end
    
    %% internal
    properties(Access=private)
        DataPlot;
        CallbackQueue
    end
    
    %% 
    properties(SetObservable,AbortSet)
        MinimumUpdatePeriod = 0.5;
    end
    %% Create
    methods
        function this = ResultsDisplay_XY(varargin)
            % Create Display
            % Syntax:
            %   ResultsDisplay_XY(Tracker)
            %   ResultsDisplay_XY(Parent,Tracker)
            %   ResultsDisplay_XY('Parent',par,'Tracker',tr,'CameraDisplay',CamDisp)
            %
            % Inputs:
            %   Tracker: RoiTracker object generating Tracker Results
            %   Parent: axes handle or CameraToolbox.Display
            %   CameraDisplay: CameraToolbox.Display to synchronize with
            %
            %   If Parent is a CamDisplay then CameraDisplay should not be
            %   specified.
            %   If Parent is axes different than CamDisplay.Parent then
            %   point will still be drawn on Parent, but will only be
            %   updated in sync with CamDisplay
            
            %% Parse Inputs
            iH = extras.inputHandler;
            iH.addOptionalVariable('Parent',[],@(x) isgraphics(x)&&strcmpi(x.Type,'axes') || isa(x,'CameraToolbox.Display')||isempty(x),true);
            iH.addRequiredVariable('Tracker',@(x) isa(x,'extras.ParticleTracking.RoiTracker.RoiTracker'),true);
            iH.addParameter('CameraDisplay',CameraToolbox.Display.empty(),@(x) isa(x,'CameraToolbox.Display'));
            
            iH.parse(varargin{:});
            
            if ~isempty(iH.Results.Parent)&&isa(iH.Results.Parent,'CameraToolbox.Display')
                assert(isempty(iH.Results.CameraDisplay),'Cannot specify CameraToolbox.Display as both Parent and CameraDisplay');
                Parent = iH.Results.Parent.Parent;
                CameraDisplay = iH.Results.Parent;
            elseif isempty(iH.Results.Parent)&&~isempty(iH.Results.CameraDisplay)
                Parent = iH.Results.CameraDisplay;
                CameraDisplay = iH.Results.CameraDisplay;
            else
                Parent = iH.Results.Parent;
                CameraDisplay = iH.Results.CameraDisplay;
            end
            Tracker = iH.Results.Tracker;
            
            %% Obj Dep
            assert(isa(Tracker,'extras.ParticleTracking.RoiTracker.RoiTracker'),'Tracker must be valid RoiTracker');
            this@extras.widgets.mixin.ObjectDependentLifetime(Tracker);
            %% Setup GraphicsParent type
            this@extras.GraphicsChild(@() gca(figure()) );
            this.AllowableParentType = 'axes'; %set allowed parent type so that current axes are captured if user specifies figure
            if isempty(Parent)
                this.CheckParentInput();
            else
                this.CheckParentInput(Parent);
            end
            
            %% set internal props
            this.Tracker = Tracker;
            this.CameraDisplay = CameraDisplay;
            
            %% create scatter plot
            this.DataPlot = line('XData',NaN,...
                'YData',NaN,...
                'Parent',this.Parent,...
                'LineStyle','none',...
                'Marker','+',...
                'Color','r',...
                'MarkerSize',14);
            
            %% associate callback queue
            this.CallbackQueue = extras.CallbackQueue(); %make queue
            afterEach(this.CallbackQueue,@(data) this.updatePlot(data)); %set callback for queue
            this.Tracker.registerQueue(this.CallbackQueue); %associate queue with tracker results
            
        end
    end
    
    %% delete
    methods
        function delete(this)
            delete(this.CallbackQueue)
            delete(this.DataPlot);
        end
    end
    
    %% Update Callback
    methods(Access=private)
        function updatePlot(this,data)
            
            %% check still valid
            if(~isvalid(this))
                return;
            end
            
            %% setup tic
            persistent lastTic;
            firstTic = false;
            if isempty(lastTic)
                lastTic = tic;
                firstTic = true;
            end
            
            %% update plots if enought time elapsed, or using CameraDisplay timestamp
            if firstTic || toc(lastTic)>this.MinimumUpdatePeriod ||~isempty(this.CameraDisplay)
                
                if ~iscell(data)
                    res=data;
                else
                    res=data{1};
                end
                
                if isempty(res) || ~isfield(res,'roiList') || ~isfield(res.roiList,'CentroidResult')
                    set(this.DataPlot,'XData',[],'YData',[]);
                else
                    if ~isempty(this.CameraDisplay) && ... %only update if matching cameradisplay image timestamp
                            res.Time~=this.CameraDisplay.CurrentImageTimestamp
                        return;
                    end
                        
                    cnt = [res.roiList.CentroidResult];
                    set(this.DataPlot,'XData',[cnt.X],'YData',[cnt.Y]);
                end
            end
            
        end
    end
end