classdef LUTobject < handle
% Class for managing a spline look-up-table
% Used in conjunction with roiObject3D and
% extras.ParticleTracking.RoiTracker
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.

    %% Properties
    properties (AbortSet,SetObservable,SetAccess=protected)
        UUID %unique identifier indicating the particle being tracked
        pp %spline of defining lut
        dpp %first derivative of spline
        rr %radial coordinates of each spline in pp
        zlim %z limits of the splines
        Z_Units = 'µm'; %char array specifying units of the z-dimension
        R_Units = 'px'; %char array specifying units of the r-dimension
    end
    properties(AbortSet,SetObservable)
        MinR (1,1) double = NaN %minimum Radius of the particle pattern
        MaxR (1,1) double = NaN%maximum radius of the particle pattern
        NormalizeProfileData (1,1) logical = true; %T/F indicating if ProfileData should be normalized
        NormalizeProfileStart char = 'end-5'; %starting range of average used to compute normalization factor applied to profile data
        NormalizeProfileEnd char = 'end';%ending range of average used to compute normalization factor applied to profile data
    end
    properties(Dependent)
        NormalizeProfileStartIndex %returns numeric stargin index used to determine normalization value
        NormalizeProfileEndIndex %returns numeric stargin index used to determine normalization value
    end
    methods
        function val = get.NormalizeProfileStartIndex(this)
            dat_len = this.MaxR-this.MinR+1;
            if isnan(dat_len)
                val = NaN;
                return;
            end
            ind = 1:dat_len;
            val = eval(['ind(',this.NormalizeProfileStart,')']);
        end
        function val = get.NormalizeProfileEndIndex(this)
            dat_len = this.MaxR-this.MinR+1;
            if isnan(dat_len)
                val = NaN;
                return;
            end
            ind = 1:dat_len;
            val = eval(['ind(',this.NormalizeProfileEnd,')']);
        end
    end
    
    
    methods
        function set.MinR(this,val)
            assert(isscalar(val),'MinR must be scalar numeric');
            this.MinR = val;
            this.IsCalibrated = false;
        end
        function set.MaxR(this,val)
            assert(isscalar(val),'MaxR must be scalar numeric');
            this.MaxR = val;
            this.IsCalibrated = false;
        end
    end
    
    properties(SetAccess=protected,AbortSet,SetObservable)
        IsCalibrated (1,1) logical = false; %flag indicating if LUT is calibrated (has valid spline table)
    end
    methods
        function set.IsCalibrated(this,val)
            this.IsCalibrated = val;
            notify(this,'PropertyChanged');
        end
    end
    
    events
        PropertyChanged %event indicating one of the properties has changed
    end
    
    %% ctor
    methods
        function this = LUTobject(varargin)
        % Syntax
        %   LUTobject(): create LUT object with none of the fields set
        %   LUTobject(roiObject): create LUT with UUID equal to the UUID
        %                         for the roiObject(s)
        %   LUTobject(__,'UUID',uuid,'MinR',mnr,'MaxR',mxr,...)           

            %% setup UUID MaxR and MinR 
            if nargin>0
                p = inputParser;
                addParameter(p,'MinR',[]);
                addParameter(p,'MaxR',[]);
                need_uuid = true;
                if isa(varargin{1},'extras.roi.roiObject3D') %use specfied roiObjects for uuid
                    UUID = {varargin{1}.UUID};
                    varargin(1) = [];
                    need_uuid=false;
                else
                    addParameter(p,'UUID',{});
                end
                
                parse(p,varargin{:});
                
                if need_uuid
                    UUID = p.Results.UUID;
                end
                
                assert(~isempty(UUID),'Must specify roiObjects or UUID');
                
                MinR = p.Results.MinR;
                
                if isempty(MinR)
                    MinR = NaN;
                end
                
                MaxR = p.Results.MaxR;
                if isempty(MaxR)
                    MaxR = NaN;
                end
                
                assert(numel(MinR)==numel(UUID)||numel(MinR)==1);
                assert(numel(MaxR)==numel(UUID)||numel(MaxR)==1);
                
                if numel(MinR) == 1
                    MinR = repmat(MinR,size(UUID));
                end 
                
                if numel(MaxR) == 1
                    MaxR = repmat(MaxR,size(UUID));
                end
                
                % set the parameters
                for n=numel(UUID):-1:1
                    this(n).UUID = UUID{n};
                    this(n).MinR = MinR(n);
                    this(n).MaxR = MaxR(n);
                end

            end
            
            %% setup listeners
            %addlistener(this,'UUID','PostSet',@(~,~) notify(this,'PropertyChanged'));
            %addlistener(this,'pp','PostSet',@(~,~) notify(this,'PropertyChanged'));
            %addlistener(this,'dpp','PostSet',@(~,~) notify(this,'PropertyChanged'));
            %addlistener(this,'MinR','PostSet',@(~,~) notify(this,'PropertyChanged'));
            %addlistener(this,'MaxR','PostSet',@(~,~) notify(this,'PropertyChanged'));
            
        end
    end
    
    %% Public Methods
    methods
        function s = toStruct(this)
        % Return struct holding essential properties
        %   Used to pass LUT to MEX Functions
        %   (e.g. see extras.ParticleTracking.RoiTracker)
            s = struct('UUID',{this.UUID},...
                'IsCalibrated',{this.IsCalibrated},...
                'pp',{this.pp},...
                'dpp',{this.dpp},...
                'MinR',{this.MinR},...
                'MaxR',{this.MaxR},...
                'zlim',{this.zlim},...
                'Z_Units',{this.Z_Units},...
                'rr',{this.rr},...
                'R_Units',{this.R_Units},...
                'NormalizeProfileData',{this.NormalizeProfileData},...
                'NormalizeProfileStartIndex',{this.NormalizeProfileStartIndex},...
                'NormalizeProfileEndIndex',{this.NormalizeProfileEndIndex});
        end
        
        function createLUT(this,Z,profiles,r_coords)
        % makes spline data out of Z and profiles
        % 
        % Input:
        %   Z: Coordinate (abscissa) values for each row in profiles
        %       numel(Z)==size(profiles,1)
        %   profiles: Data to fit
        %        =[ I(Z[1],r_min), I(Z[1],r_2), ... I(Z[1],r_max);
        %                           ... 
        %           I(Z[m],r_min), I(Z[m],r_2), ... I(Z[m],r_max);]
        %  r_coords: radial coordinates (mantissa) valuse for columns
        %       numel(r_coords)==size(profiles,2)
            
            %% check inputs
            assert(numel(r_coords)==size(profiles,2),'columns of profiles must match size of r_coords');
            assert(numel(Z)==size(profiles,1),'rows of profiles must match size of Z');
            
            this.IsCalibrated = false;
            
            this.MinR = min(r_coords);
            this.MaxR = max(r_coords);
            this.rr = r_coords;
            this.zlim = [min(Z),max(Z)];
            
            %% normalize profile data if needed
            if this.NormalizeProfileData
                for n=1:size(profiles,1)
                    avg = nanmean(profiles(n,this.NormalizeProfileStartIndex:this.NormalizeProfileEndIndex));
                    profiles(n,:) =  profiles(n,:)/avg;
                end
            end
        
            %% compute regularized & simplified spline 
            this.pp = extras.ParticleTracking.splineroot_helpers.smoothpchip(reshape(Z,[],1),profiles);
            
            %% compute derivatives
            this.dpp = fnder(this.pp,1);
            
            %% done, set calibration flag
            this.IsCalibrated = true;
            
            %% send final notification
            notify(this,'PropertyChanged');
            
        end
    end
end