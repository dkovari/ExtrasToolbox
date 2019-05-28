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
        MinimumUpdatePeriod (1,1) double = 0.8;
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
            iH.addOptionalVariable('Parent',[],@(x) isgraphics(x)&&isvalid(x)||isempty(x),true);
            iH.addRequiredVariable('Tracker',@(x) isa(x,'extras.ParticleTracking.RoiTracker.RoiTracker'),true);
            iH.addRequiredVariable('LUT',@(x) isa(x,'extras.roi.LUTobject'),true);
            
            iH.parse(varargin{:});
            
            Parent = iH.Results.Parent;
            Tracker = iH.Results.Tracker;
            LUT = iH.Results.LUT;
            
            %% Setup lutViewer
            this@extras.roi.lutViewer('LUT',LUT,'Parent',Parent);
            this.linkObjectLifetime(Tracker);
            
            %% line showing measured radial profile
            this.hMeas = line('Parent',this.hAx_R_Profile,'XData',NaN,'YData',NaN,'Marker','none','LineStyle',':','LineColor','r','LineWidth',1.5,'DisplayName','Measured Profile');
            this.hFitErr = patch('Parent',this.hAx_Spline,...
                'XData',NaN(1,4),...
                'YData',NaN(1,4),...
                'EdgeColor','r',...
                'LineWidth',1,...
                'LineStyle',':',...
                'FaceColor','r',...
                'FaceAlpha',0.5,...
                'DisplayName','Spline Fit Error',...
                'PickableParts','none');
            this.hFit = line('Parent',this.hAx_Spline,...
                'XData',NaN,'YData',NaN,...
                'Marker','none',...
                'LineStyle','-',...
                'LineColor','r',...
                'LineWidth',1.5,...
                'DisplayName','Spline Fit',...
                'PickableParts','none');
            
            %listener for xlim changes, expands/contracts fit line
            addlistener(this.hAx_Spline,'XLim','PostSet',@(~,~) set(this.hFit,'XData',this.hAx_Spline.XLim));
            addlistener(this.hAx_Spline,'XLim','PostSet',@(~,~) set(this.hFitErr,'XData',[this.hAx_Spline.XLim,flip(this.hAx_Spline.XLim)]));
            
            %% Subscribe to tracker results
            this.TrackerResultsQueue = extras.CallbackQueue();
            this.TrackerResultsQueue.afterEach(@(d) this.ProcessNewResult(d));
            this.Tracker.registerQueue(this.TrackerResultsQueue);
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
                set(this.hMeas,'XData',res.roiList(roi_ind).RadialAverage_rloc,...
                    'YData',res.roiList(roi_ind).RadialAverage);
                
                %% Update ZFit
                zz = res.roiList(roi_ind).LUT(lut_ind).Result.Z;
                set(this.hFit,'YData',[zz,zz]);
                
                sZ = sqrt(res.roiList(roi_ind).LUT(lut_ind).Result.varZ);
                set(this.hFitErr,'YData',...
                    [zz+sZ,zz+sZ,zz-sZ,zz-sZ]);
                
                
                %% update tic
                lastTic = tic;
                
            end
            
            
        end
    end
end