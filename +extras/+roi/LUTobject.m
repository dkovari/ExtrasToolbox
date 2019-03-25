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
            
            %% create spline
            
            %% simplify knots
            
            %% compute derivatives
            
            %% 
            this.MinR = MinR;
            this.MaxR = MaxR;
        end
    end
end