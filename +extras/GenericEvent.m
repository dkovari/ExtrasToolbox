classdef GenericEvent < event.EventData & dynamicprops
    % Event Class for handeling arbirary event data
    
    %% create
    methods
        function this = GenericEvent(varargin)
        %% Construct generic event
        % Syntax:
        %   GenericEvent(): create event without properties
        %   GenericEvent(STRUCT_SCALAR): construct event with properties
        %       detrmined by fields of STRUCT_SCALAR
        %   GenericEvent(__,CELL_STR): construct event with properties
        %       specified by names in char arrays
        %   GenericEvent(__,'Name',Value,...): constuct event with
        %       propertie names set by 'Name' fields and Values for those
        %       properties set by the repective Value
        
            %% Handle no input
            if nargin<1
                return;
            end
            
            %% handle struct
            if ~isempty(varargin)
                if isstruct(varargin{1})
                    s = varargin{1};
                    if numel(s)>1
                        error('GenericEvent cannot be constructed from struct with numel>1');
                    end
                    fn = fieldnames(s);
                    for n=1:numel(fn)
                        mp = findprop(this,fn{n});
                        if isempty(mp)
                            mp = addprop(this,fn{n});
                        end
                        this.(fn{n}) = s.(fn{n});
                    end
                    varargin(1) = [];
                end
            end
            
            %% handle cell_str
            if ~isempty(varargin)
                if iscellstr(varargin{1})
                    fn = varargin{1};
                    for n=1:numel(fn)
                        mp = findprop(this,fn{n});
                        if isempty(mp)
                            mp = addprop(this,fn{n});
                        end
                    end
                    varargin(1) = [];
                end
            end
            
            %% handle name,value
            if ~isempty(varargin)
                assert(mod(numel(varargin),2)==0,'GenericEvent requires even number of inputs to evaluate name,value pairs');
                
                for n=1:2:numel(varargin)
                    fn = varargin{n};
                    assert(ischar(fn),'Error parsing Name,Value pairs');
                    mp = findprop(this,fn);
                    if isempty(mp)
                        mp = addprop(this,fn);
                    end
                    this.(fn) = varargin{n+1};
                end
            end
        end
    end
end