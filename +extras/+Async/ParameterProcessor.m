classdef ParameterProcessor < extras.Async.AsyncProcessorWriter & dynamicprops
% Base class for all ParameterProcess type AsyncProcessors
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.

    %% create
    methods
        function this = ParameterProcessor(MEX_FUNCTION)
            this@extras.Async.AsyncProcessorWriter(MEX_FUNCTION)
            this.Name = 'ParameterProcessor'; %Change Name
            
            %% update params
            this.internal_updateParameters();
        end
    end
    %% persistentArgs methods
    properties (AbortSet=true,SetObservable=true)
        Parameters %user-accessible 
    end
    properties(Access=protected)
        internal_setParameters = false; %lock specifying if using internal call to set.PersistentArgs
    end
    properties(Access=private)
        DynamicParameterMap = containers.Map('KeyType','char','ValueType','any');
    end
    methods(Access=private)
        function internal_updateParameters(this)
            %% Get new value of Persistent args
            this.internal_setParameters = true;
            try
                this.Parameters = this.runMethod('getParameters');
            catch ME
                this.internal_setParameters = false;
                rethrow(ME)
            end
            this.internal_setParameters = false;
            
            %% Update Dynamic Props
            if isempty(this.Parameters)
                prop_names = {};
            else
                prop_names = reshape(fieldnames(this.Parameters),1,[]);
            end
            
            %delete dynamic properties not currrent property list
            bad_props = setdiff(keys(this.DynamicParameterMap),prop_names);
            for n=1:numel(bad_props)
                bp = bad_props{n};
                mp = findprop(this,bp);
                if ~isempty(mp)
                    delete(mp);
                end
            end
            remove(this.DynamicParameterMap,bad_props);
            
            % add/update properties
            for n=1:numel(prop_names)%prop_name = prop_names
                prop_name = prop_names{n};
                mp = findprop(this,prop_name);
                if isempty(mp) %need to create
                    mp = this.addprop(prop_name);
                    mp.SetMethod = @(h,value) setParameters(h,prop_name,value);
                    mp.SetObservable = true;
                    mp.AbortSet = true;
                    this.DynamicParameterMap(prop_name) = mp;
                end
                
                %% update value
                mp.SetMethod = []; %change to default set because we don't want recursive bug
                this.(prop_name) = this.Parameters.(prop_name);
                mp.SetMethod = @(h,value) setParameters(h,prop_name,value);
            end
            
        end
    end
    methods
        function set.Parameters(this,Args)
        % change the persistent args
        % external calls are forwarded to setPersistentArgs() or
        % clearPersistentArgs()
        
            if this.internal_setParameters %setting internally, just change values
                this.Parameters = Args;
            else
                if iscell(Args)
                    if isempty(Args)
                        this.clearParameters();
                    else
                        this.setParameters(Args{:});
                    end
                else
                    assert(isstruct(Args)&&numel(Args)==1,'PersistentArgs must be set using a struct of size [1,1]');
                     this.setParameters(Args);
                end
                
            end
        end
        
        function setParameters(this,varargin)
        % set persistent arguments
        % those arguments will be appended to the arguments passed by pushTask
        
            this.runMethod('setParameters',varargin{:});
            
            %% Get new value of Persistent args
            this.internal_updateParameters();
        end

        function clearParameters(this)
        % clear persistent arguments
        
            %% clear
            this.runMethod('clearParameters');
            
            %% Get new value of Persistent args
            this.internal_updateParameters();
        end
    end
end