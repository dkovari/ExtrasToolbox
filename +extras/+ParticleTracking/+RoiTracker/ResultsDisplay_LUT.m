classdef ResultsDisplay_LUT <  extras.roi.lutViewer
% Class for Displaying LUT Tracking Results produced by RoiTracker3D
%
% 
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved

    %% Lines
    properties(Access=private)
        hMeas = gobjects(1); %handle to measured profile line
        hFit = gobjects(1); %handle to z fit line
        hFitErr = gobjects(1); %handle to z err rect
        TrackerResultsQueue = extras.CallbackQueue.empty(); % callback queue listening to Tracker results
    end
    
    %% 
    properties(AbortSet,SetObservable)
        MinimumUpdatePeriod (1,1) double = 0.3;
    end
    
    %% Internal Handler
    properties(Access=protected)
        Tracker
    end
    
    %% Create
    methods
        function this = ResultsDisplay_LUT(varargin)
        % Create Results Display
        % Syntax:
        %   ResultsDisplay_LUT(Tracker,LUT)
        %   ResultsDisplay_LUT(Parent,Tracker,LUT)
        %   ResultsDisplay_LUT('Parent',par,'Tracker',tr,'LUT',lut);
        %
        % Inputs:
        %   Tracker: Handle to RoiTracker3D generating results
        %   LUT: handle to LUTobject used by Tracker and specifying lut
        %        parameters
        %   Parent: graphical container in which display is created
            
            %% Parse Inputs
            iH = extras.inputHandler();
            iH.addOptionalVariable('Parent',[],@(x) isempty(x)||numel(x)==1&&isgraphics(x),true);
            iH.addRequiredVariable('Tracker',@(x) numel(x)==1&&isa(x,'extras.ParticleTracking.RoiTracker.RoiTracker'),true);
            iH.addRequiredVariable('LUT',@(x) isa(x,'extras.roi.LUTobject'),true);
            
            iH.parse(varargin{:});
            
            Parent = iH.Results.Parent;
            Tracker = iH.Results.Tracker;
            LUT = iH.Results.LUT;
            
            %% Setup lutViewer
            this@extras.roi.lutViewer('LUT',LUT,'Parent',Parent);
            
            %% Setup other props
            for n=numel(this):-1:1
                %% Tracker
                this(n).linkObjectLifetime(Tracker);
                this(n).Tracker = Tracker;
                
                %% line showing measured radial profile
                this(n).hMeas = line('Parent',this(n).hAx_R_Profile,...
                    'XData',NaN,'YData',NaN,...
                    'Marker','none',...
                    'LineStyle',':',...
                    'Color','r',...
                    'LineWidth',1.5,...
                    'DisplayName','Measured Profile');

                this(n).hFitErr = patch('Parent',this(n).hAx_Spline,...
                    'XData',NaN(1,4),...
                    'YData',NaN(1,4),...
                    'EdgeColor','r',...
                    'LineWidth',1,...
                    'LineStyle',':',...
                    'FaceColor','r',...
                    'FaceAlpha',0.5,...
                    'DisplayName','\pm2\sigma_z',...
                    'PickableParts','none');
                this(n).hFit = line('Parent',this(n).hAx_Spline,...
                    'XData',[NaN,NaN],'YData',[NaN,NaN],...
                    'Marker','none',...
                    'LineStyle','--',...
                    'Color','r',...
                    'LineWidth',1.5,...
                    'DisplayName','Z_{fit}',...
                    'PickableParts','none');
                set(this(n).hFit,'XData',this(n).hAx_Spline.XLim);
                set(this(n).hFitErr,'XData',[this(n).hAx_Spline.XLim,flip(this(n).hAx_Spline.XLim)]);

                %listener for xlim changes, expands/contracts fit line
                addlistener(this(n).hAx_Spline,'XLim','PostSet',@(~,~) set(this(n).hFit,'XData',this(n).hAx_Spline.XLim));
                addlistener(this(n).hAx_Spline,'XLim','PostSet',@(~,~) set(this(n).hFitErr,'XData',[this(n).hAx_Spline.XLim,flip(this(n).hAx_Spline.XLim)]));

                %% Subscribe to tracker results
                this(n).TrackerResultsQueue = extras.CallbackQueue();
                this(n).TrackerResultsQueue.afterEach(@(d) this(n).ProcessNewResult(d));
                this(n).Tracker.registerQueue(this(n).TrackerResultsQueue);
                
                %% display draggable lines on top
                uistack(this(n).hLn_Z_Profile.LineHandle,'top');
                uistack(this(n).hLn_R_Profile.LineHandle,'top');
            end
 
            
        end
    end
    
    %% callbacks
    methods(Access=private)
        function ProcessNewResult(this,data)
            
            %% make sure still valid
            if(~isvalid(this))
                return;
            end
            
            %% setup lastTic
            persistent lastTic;
            firstTic = false;
            if isempty(lastTic)
                lastTic = tic;
                firstTic = true;
            end
            
            %% update plots if needed
            if firstTic || toc(lastTic)>this.MinimumUpdatePeriod
                if ~iscell(data)
                    res=data;
                else
                    res=data{1};
                end
                
                %% Find this LUT in LUT Results List
                if isempty(this.LUT.ParentROI)
                    warning('LUT associated with ResultsDisplay does not have ParentROI specified');
                    return;
                end
                
                if isempty(res) || ~isfield(res,'roiList') || isempty(res.roiList)
                    return;
                end
                
                % find roi
                roi_ind = find(strcmpi(this.LUT.ParentROI.UUID,{res.roiList.UUID}),1);
                if isempty(roi_ind)
                    warning('could not find ROI Parent UUID=%s in results list',this.LUT.ParentROI.UUID);
                    return;
                end
                
                % find LUT
                if ~isfield(res.roiList(roi_ind),'LUT')
                    return;
                end
                if isempty(this.LUT.UUID)
                    warning('LUT associated with results display does not have UUID');
                    return;
                end
                lut_ind = find(strcmpi(this.LUT.UUID,{res.roiList(roi_ind).LUT.UUID}),1);
                if isempty(lut_ind)
                    warning('could not find LUT UUID=%s in result LUT list',this.LUT.UUID);
                    return;
                end
                
                %% Update Radial Average Plot
                if ~isfield(res.roiList(roi_ind),'RadialAverage')
                    set(this.hMeas,'XData',[],...
                    'YData',[]);
                    return;
                end
                RP = this.LUT.normalizeProfile(res.roiList(roi_ind).RadialAverage,res.roiList(roi_ind).RadialAverage_rloc);
                set(this.hMeas,'XData',res.roiList(roi_ind).RadialAverage_rloc,...
                    'YData',RP);
                
                %% Update ZFit               
                if ~isfield(res.roiList(roi_ind).LUT(lut_ind),'DepthResult')
                    zz = NaN;
                    sZ = NaN;
                else
                    zz = res.roiList(roi_ind).LUT(lut_ind).DepthResult.Z;
                    sZ = 2*sqrt(res.roiList(roi_ind).LUT(lut_ind).DepthResult.varZ);
                end
                
                set(this.hFit,'YData',[zz,zz]);
                set(this.hFitErr,'YData',...
                    [zz+sZ,zz+sZ,zz-sZ,zz-sZ]);
                
                
                %% update tic
                lastTic = tic;
                
            end
            
            
        end
    end
end