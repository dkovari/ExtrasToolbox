classdef (HandleCompatible) AssignNV < matlab.mixin.SetGet
    % Mixin class providing easy set access of properties
    %
    % Typically this will be used for classes that have constructors that
    % accept name-value pairs to initialize their properties.
    %
    % unlike just using matlab.mixin.setget, you can force restrictions of
    % setting private or protected members.
    
    properties(Access=protected)
        AssignNV_CaseSensitive (1,1) logical = false;
        %AssignNV_KeepUnmatched (1,1) logical = true;
        AssignNV_PartialMatching (1,1) logical = true;
        AssignNV_StructExpand (1,1) logical = true;
    end
    
    methods(Hidden) %don't advertise setPublicProperties method
        function [Unmatched,Results] = setPublicProperties(this,varargin)
            % accepts Name,Value syntax and sets public properties
            % of the derived class
            %
            % Return:
            %   Unmatched: struct of unmatched name-value pairs
            %   Resutls: struct of matched name-value pairs
            
            props = extras.getproperties(this,'SetAccess','public');
            p = inputParser;
            p.CaseSensitive = this.AssignNV_CaseSensitive;
            p.KeepUnmatched = true;
            p.PartialMatching = this.AssignNV_PartialMatching;
            p.StructExpand = this.AssignNV_StructExpand;
            
            for n=1:numel(props)
                addParameter(p,props{n},[]);
            end
            
            parse(p,varargin{:});
            Unmatched = p.Unmatched;
            Results = p.Results;
            
            % remove unspecified
            UseDef = p.UsingDefaults;
            Results = rmfield(Results,UseDef); 
            
            %% set values
            set(this,Results);
            
        end
        
    end
    
    methods (Access=protected)
        function [Unmatched,Results] = setProtectedProperties(this,varargin)
            % accepts Name,Value syntax and sets public  and protected properties
            % of the derived class
            %
            % Return:
            %   Unmatched: struct of unmatched name-value pairs
            %   Resutls: struct of matched name-value pairs
            
            props = extras.getproperties(this,'SetAccess',{'public','protected'});
            p = inputParser;
            p.CaseSensitive = this.AssignNV_CaseSensitive;
            p.KeepUnmatched = true;
            p.PartialMatching = this.AssignNV_PartialMatching;
            p.StructExpand = this.AssignNV_StructExpand;
            
            for n=1:numel(props)
                addParameter(p,props{n},[]);
            end
            
            parse(p,varargin{:});
            Unmatched = p.Unmatched;
            Results = p.Results;
            
            % remove unspecified
            UseDef = p.UsingDefaults;
            Results = rmfield(Results,UseDef); 
            
            %% set values
            set(this,Results);
            
        end
        
        function [Unmatched,Results] = setAllProperties(this,varargin)
            % accepts Name,Value syntax and sets all properties
            % including: public, protected, and private
            % of the derived class
            %
            % Return:
            %   Unmatched: struct of unmatched name-value pairs
            %   Resutls: struct of matched name-value pairs
            
            props = extras.getproperties(this,'SetAccess');
            p = inputParser;
            p.CaseSensitive = this.AssignNV_CaseSensitive;
            p.KeepUnmatched = true;
            p.PartialMatching = this.AssignNV_PartialMatching;
            p.StructExpand = this.AssignNV_StructExpand;
            
            for n=1:numel(props)
                addParameter(p,props{n},[]);
            end
            
            parse(p,varargin{:});
            Unmatched = p.Unmatched;
            Results = p.Results;
            
            % remove unspecified
            UseDef = p.UsingDefaults;
            Results = rmfield(Results,UseDef); 
            
            %% set values
            set(this,Results);
            
        end
    end
end