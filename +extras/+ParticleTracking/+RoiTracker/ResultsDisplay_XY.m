classdef ResultsDisplay_XY < handle & extras.GraphicsChild & extras.widgets.mixin.ObjectDependentLifetime
%Display scatter plot of points on axes showing realtime results for XY
%coordinates
    
    %% Props
    properties(SetAccess=private,SetObservable,AbortSet)
        Tracker; %handle to tracker object
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
        function this = ResultsDisplay_XY(Parent,Tracker)
            % Create Display
            % Inputs:
            %   Parent Axes
            %   Tracker Object
            
            %% Obj Dep
            assert(isa(Tracker,'extras.ParticleTracking.RoiTracker.RoiTracker'),'Tracker must be valid RoiTracker');
            this@extras.widgets.mixin.ObjectDependentLifetime(Tracker);
            %% Setup GraphicsParent type
            this@extras.GraphicsChild(@() gca(figure()) );
            this.AllowableParentType = 'axes'; %set allowed parent type so that current axes are captured if user specifies figure
            this.CheckParentInput(Parent);
            
            %% set internal props
            this.Tracker = Tracker;
            
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
            if(~isvalid(this))
                return;
            end
            
            persistent lastTic;
            firstTic = false;
            if isempty(lastTic)
                lastTic = tic;
                firstTic = true;
            end
            if firstTic || toc(lastTic)>this.MinimumUpdatePeriod
                if ~iscell(data)
                    res=data;
                else
                    res=data{1};
                end

                if isempty(res) || ~isfield(res,'roiList') || ~isfield(res.roiList,'CentroidResult')
                    set(this.DataPlot,'XData',[],'YData',[]);
                else
                    cnt = [res.roiList.CentroidResult];
                    set(this.DataPlot,'XData',[cnt.X],'YData',[cnt.Y]);
                end
            end
            
        end
    end
end