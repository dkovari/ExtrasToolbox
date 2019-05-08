classdef LUTobject < handle
% class for managing a spline look-up-table
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.

    properties (AbortSet,SetObservable,SetAccess=protected)
        UUID %unique identifier indicating the particle being tracked
        pp %spline of defining lut
        dpp %first derivative of spline
        rr %radial coordinates of each spline in pp
        zlim %z limits of the splines
        Z_Units = 'µm';
        R_Units = 'px';
        
    end
    properties(AbortSet,SetObservable)
        MinR %minimum Radius of the particle pattern
        MaxR %maximum radius of the particle pattern
        
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
        IsCalibrated = false;
    end
    
    events
        PropertyChanged
    end
    
    %% ctor
    methods
        function this = LUTobject(varargin)
            
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
            addlistener(this,'UUID','PostSet',@(~,~) notify(this,'PropertyChanged'));
            addlistener(this,'pp','PostSet',@(~,~) notify(this,'PropertyChanged'));
            addlistener(this,'dpp','PostSet',@(~,~) notify(this,'PropertyChanged'));
            addlistener(this,'MinR','PostSet',@(~,~) notify(this,'PropertyChanged'));
            addlistener(this,'MaxR','PostSet',@(~,~) notify(this,'PropertyChanged'));
            
        end
    end
    
    %%
    methods
        function s = toStruct(this)
            s = struct('UUID',{this.UUID},'pp',{this.pp},'dpp',{this.dpp},'MinR',{this.MinR},'MaxR',{this.MaxR});
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
            
            %%
            assert(numel(r_coords)==size(profiles,2),'columns of profiles must match size of r_coords');
            assert(numel(Z)==size(profiles,1),'rows of profiles must match size of Z');
            
            this.MinR = min(r_coords);
            this.MaxR = max(r_coords);
            this.rr = r_coords;
            this.zlim = [min(Z),max(Z)];
        
            %% compute regularized & simplified spline 
            this.pp = extras.ParticleTracking.splineroot_helpers.smoothpchip(reshape(Z,[],1),profiles);
            
            %% compute derivatives
            this.dpp = fnder(this.pp,1);
            
            %% done, set calibration flag
            this.IsCalibrated = true;
            
        end
    end
end