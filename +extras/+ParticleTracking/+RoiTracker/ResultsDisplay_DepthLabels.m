classdef ResultsDisplay_DepthLabels < extras.GraphicsChild & ...
        extras.widgets.mixin.ObjectDependentLifetime
% Display labels showing LUT depth for default LUT
    
    %% Props
    properties(SetAccess=private,SetObservable,AbortSet)
        Tracker; %handle to tracker object
        CameraDisplay = CameraToolbox.Display.empty(); %optional CameraDisplay to synchronized with
        Labels = gobjects(0); %handle to labels
    end
    
    %% private
    properties(Access=private)
        CallbackQueue
    end
    
    %% 
    properties(SetObservable,AbortSet)
        MinimumUpdatePeriod = 0.5;
    end
    
    %% create
    methods
        function this = ResultsDisplay_DepthLabels(varargin)
            % Create label display
            % Syntax:
            %   ResultsDisplay_DepthLabels(Tracker)
            %   ResultsDisplay_DepthLabels(Parent,Tracker)
            %   ResultsDisplay_DepthLabels('Parent',par,'Tracker',tr,'CameraDisplay',CamDisp)
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
            
            %% associate callback queue
            this.CallbackQueue = extras.CallbackQueue(); %make queue
            afterEach(this.CallbackQueue,@(data) this.updateLabels(data)); %set callback for queue
            this.Tracker.registerQueue(this.CallbackQueue); %associate queue with tracker results
        end
    end
    
    %% callbacks
    methods(Access=private)
        function updateLabels(this,data)
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
                %% check result
                if ~iscell(data)
                    res=data;
                else
                    res=data{1};
                end
                if isempty(res) || ~isfield(res,'roiList')
                    return;
                end
                
                if ~isempty(this.CameraDisplay) && ... %only update if matching cameradisplay image timestamp
                        res.Time~=this.CameraDisplay.CurrentImageTimestamp
                    return;
                end
                
                %% empty list
                if isempty(res.roiList)
                    delete(this.Labels);
                    this.Labels = gobjects(0);
                end
                
                %% Set text for each roi
                for n=numel(res.roiList):-1:1
                    %% get label
                    if numel(this.Labels)>=n && isgraphics(this.Labels(n)) %valid label
                        label = this.Labels(n);
                    else %must create label
                        label = text('Parent',this.Parent,...
                            'Position',res.roiList(n).Window(1:2),...
                            'VerticalAlignment','top',...
                            'HandleVisibility','callback',...
                            'PickableParts','none',...
                            'Color','r',...
                            'FontSize',9);
                        this.Labels(n) = label;
                    end
                    
                    %% update label position
                    set(label,'Position',res.roiList(n).Window(1:2));
                    
                    %this_roi = res.roiList(n)
                    %DL = res.roiList(n).DefaultLUT
                    
                    %% Check for default LUT
                    if ~isfield(res.roiList(n),'DefaultLUT') || isempty(res.roiList(n).DefaultLUT)
                        label.String = 'Z: ---';
                        continue;
                    end
                    
                    %% find default LUT
                    if ~isfield(res.roiList(n),'LUT')||isempty(res.roiList(n).LUT)
                        label.String = 'Z: No LUT';
                        continue;
                    end
                    
                    lut_ind = find(strcmp(res.roiList(n).DefaultLUT.UUID,{res.roiList(n).LUT.UUID}),1);
                    if isempty(lut_ind)
                        label.String = 'Z: No DefaultLUT';
                        continue;
                    end
                    
                    %% Get Z value
                    if ~isfield(res.roiList(n).LUT(lut_ind),'DepthResult')
                        label.String = 'Z: Not Calibrated';
                        continue;
                    end
                    Z = res.roiList(n).LUT(lut_ind).DepthResult.Z;
                    vZ = res.roiList(n).LUT(lut_ind).DepthResult.varZ;
                    
                    %% determine if referenced against other roi and set string
                    if ~strcmp(res.roiList(n).LUT(lut_ind).UUID,res.roiList(n).UUID) % relative
                        %% find reference roi
                        ref_roi = find(strcmp(res.roiList(n).LUT(lut_ind).UUID,{res.roiList.UUID}),1);
                        if isempty(ref_roi)
                            label.String = '\DeltaZ: Ref. Not Found';
                            continue;
                        end
                        %% find ref result
                        if ~isfield(res.roiList(ref_roi),'LUT')||isempty(res.roiList(ref_roi).LUT)
                            label.String = '\DeltaZ: Ref. Not Computed';
                            continue;
                        end
                        ref_lut = find(strcmp(res.roiList(ref_roi).UUID,{res.roiList(ref_roi).LUT.UUID}),1);
                        if isempty(ref_lut)
                            label.String = '\DeltaZ: Ref. LUT Missing';
                            continue;
                        end
                        if ~isfield(res.roiList(ref_roi).LUT(ref_lut),'DepthResult')
                            label.String = '\DeltaZ: Ref. No Result';
                            continue;
                        end
                        Z_ref = res.roiList(ref_roi).LUT(ref_lut).DepthResult.Z;
                        vZ_ref = res.roiList(ref_roi).LUT(ref_lut).DepthResult.varZ;
                        
                        %% set string
                        label.String = sprintf('\\DeltaZ: %.2g\\pm%.2g %s',Z_ref-Z,2*sqrt(vZ+vZ_ref),res.roiList(n).LUT(lut_ind).Z_Units);

                    else %not relative
                        %% set string
                        label.String = sprintf('Z: %.2g\\pm%.2g %s',Z,2*sqrt(vZ),res.roiList(n).LUT(lut_ind).Z_Units);
                    end
                    
                end
            end
        end
    end
    
    
end