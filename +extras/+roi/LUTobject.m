classdef LUTobject < handle
    
    properties
        UUID
        pp
        dpp
        MinR
        MaxR
    end
    
    methods
        function s = toStruct(this)
            s = struct('UUID',{this.UUID},'pp',{this.pp},'dpp',{this.dpp},'MinR',{this.MinR},'MaxR',{this.MaxR});
        end
        
        function createLUT(this,Z,profiles,MinR,MaxR)
        % makes spline data out of Z and profiles
        % 
        % Input:
        %   Z: Coordinate (abscissa) values for each row in profiles
        %       numel(Z)==size(profiles,1)
        %   profiles: Data to fit
        %        =[ I(Z[1],r_min), I(Z[1],r_2), ... I(Z[1],r_max);
        %                           ... 
        %           I(Z[m],r_min), I(Z[m],r_2), ... I(Z[m],r_max);]
        %  MinR: minimum radius, corresponds to first column of profiles
        %  MaxR: maximum radius, corresponds to last column of profiles
            
            %% compute regularized & simplified spline 
            this.pp = extras.ParticleTracking.splineroot_helpers.smoothpchip(Z,profiles);
            
            %% compute derivatives
            this.dpp = fnder(this.pp,1);
            
            %% 
            this.MinR = MinR;
            this.MaxR = MaxR;
        end
    end
end