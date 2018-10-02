classdef ObjectMap < matlab.mixin.SetGet
% extras.ObjectMap Class
%
% Key-Value Map where the Values can be any type of object
% {} and () are overloaded to accept multiple indices
% 
% Usage:
%   ObjectMap can be used to create a map between any object and a key
%   defined by a standard type 
% Key Types include:
%    'char', 'double', 'single', 'int32', 'uint32', 'int64', or 'uint64'
%
% ValueType is used to define the object restrictions
%   If ValueType='any' the the class essentially reproduces the
%   standard container.Map class.
%
% If other ValueType is specified, set operations will attempt to
% perform automatic type conversion using the specified classes
% constructor.
%
% Accessing Values
%====================================
% Multiple keys may be specified by passing a cell array of keys
%
% Return type matches the cell array convention
% ()  --> returns cell array
%   OM({'one','two','three'})
%       >>> ans = 
%       >>>     1x3 cell array
%       >>>     {1x1 obj} {1x1 obj} {1x1 obj}
%
% {} --> returns comma separated list
%   OM{{'one','two'}}
%       >> ans = 
%           obj:...
%       >> ans = 
%           obj:...
%
% You can wrap comma separated list into an object array using square
% brackets
%   [OM{{'one','two'}}]
%       >> ans = 
%           1x2 obj array with properties:...
%
%
% Setting Values
%=======================================
% Values can be assigned using the same conventions as cell arrays
%
%   [OM({'one','two'})] = deal(Val1,Val2)
%           or
%   [OM({'one','two'})] = deal(ValForBoth)
%
%   OM{'one'} = Val
%       or
%   OM('one') = Val
%
%   OM({'one','two'}) = ValForBoth
%   OM({'one','two'}) = {Val1, Val2};
%       Note: to set multiple entries equal to the same cell array use
%       extra bracket
%   OM({'one','two'}) = {{CellForBoth}};
%
% For objects that require multiple construction arguments, You can 
% also construct individual key-value pairs inplace
%   OM.setValue('newKey',Arg1,Arg2,...);
%
% Deleting the ObjectMap Contents
%=============================
%   Calling delete on an ObjectMap will also forward the delete
%   operation to the objects contained in the map. Therefore if the map
%   contains handle objects, they will be deleted.
%
% ScalarValues
%==============================
% You can restrict the map to hold scalar elements, such that only a single
% instance of type ValueType can be stored in each mapped element.
% By setting ScalarValues=1 the subreference notation parallels traditional
% arrays
%Example
%   >> OM = extras.ObjectMap('ValueType','double','ScalarValues',true);
%   >> OM('one')=1
%   >> OM('two')=2
%   >> vals = OM(:)
%       vals = 
%           1   2
%   >> vals = OM({'two','one'})
%       vals = 
%           2   1
%
% NOTE: ScalarValues requires ValueType not be 'any' or 'cell'
%
% hasmember(ObjMap,vals)
%====================================
% ObjectMap has a member function that is similar to ismember
%   you can determing if a value is held within the map, and the keys that
%   it corresponds to
%   
%   [inMap,keyMatch] = hasmember(ObjMap,valSet)
%       If the ObjectMap has ScalarValues=true
%           valSet should be specified using an array of ValueType
%       If ObjectMap is not scalar (ScalarValues=false) then valSet should
%       be specified as a cell array, otherwise the entire set of values
%       will be interpreted as the value against which each element of the
%       map is tested.

    properties(SetAccess=protected)
        KeyType = 'char';
        ValueType = 'any'; 
        ScalarValues = false;
    end
    methods
        function set.ScalarValues(this,val)
            this.ScalarValues = logical(val);
        end
    end
    
    properties (Access=protected)
        Map
    end
    
    events
        KeyAdded
        KeyRemoved
        KeysChanged
    end
    
    properties(Dependent)
        Count
    end
    methods
        function val = get.Count(this)
            val = this.Map.Count;
        end
    end
    
    %% Create/Delete
    methods
        function this = ObjectMap(varargin)
            %% set parameters by argument Name-value pairs
            p = inputParser;
            addParameter(p,'KeyType','char',@(x) ischar(x)&&ismember(x,{'char','double','single','int32','uint32','int64','uint64'}));
            addParameter(p,'ValueType','double',@ischar);
            addParameter(p,'ScalarValues',false,@isscalar);
            parse(p,varargin{:});
            set(this,p.Results);
            
            if ismember(this.ValueType,{'any','cell'})
                this.ScalarValues = false;
            end
            
            %% Create Map
            this.Map = containers.Map('KeyType',this.KeyType,'ValueType','any');
        end
        function delete(this)
            vals = values(this.Map);
            for n=1:numel(vals)
                try
                    delete(vals{n});
                catch ME
                   %disp(ME.getReport)
                   warning('could not delete object in map, contained object may not be handle objects');
                end
            end
            delete(this.Map);
        end
    end
    
    %% Forwarded Map Methods
    methods
        function tf = isKey(this,keySet)
            tf = this.Map.isKey(keySet);
        end
        function keySet = keys(this)
            keySet = keys(this.Map);
        end
        function mapLength = length(this)
            mapLength = length(this.Map);
        end
        function remove(this,keySet)
        % ketSet should be scalar key or cell array of keys
            try
                remove(this.Map,keySet);
                notify(this,'KeyRemoved');
                notify(this,'KeysChanged');
            catch
            end
        end
        function varargout = size(this,varargin)
            [varargout{1:nargout}] = size(this.Map,varargin{:});
        end
        
        function v = values(this)
        %Return cell array contain all the values in the map
            v = values(this.Map);
        end
        
        function n = numel(this)
            n = this.length;
        end
    end
    
    %% Additional Get/Set Public Methods
    methods
        function setValue(this,key,varargin)
            
            notKey = ~this.Map.isKey(key);
            % set value of key
            if strcmp(this.ValueType,'any')
                assert(numel(varargin)==1,'When setting ObjectMap with ValueType=''any'' only one setValue argument is needed.')
                this.Map(key) = varargin{1};
            elseif numel(varargin)==1 && isa(varargin{1},this.ValueType)
                if this.ScalarValues
                    assert(isscalar(varargin{1}),'ObjectMap has ScalarValues=true, set Value must be a single object');
                end
                this.Map(key) = varargin{1};
            else
                this.Map(key) = feval(this.ValueType,varargin{:});
            end
            
            if notKey
                notify(this,'KeyAdded');
                notify(this,'KeysChanged');
            end
        end
        function val = getValue(this,key)
            assert(~iscell(key)&& isa(key,this.KeyType) , sprintf('key must be a single variable or type: %s',this.KeyType));
            
            assert(this.isKey(key),'specified key is not valid');
            
            val = this.Map(key);
        end
    end
    
    %% Other Public Methods
    methods
        function [inMap,keyMatch] = hasmember(this,ValueSet)
            if this.ScalarValues
                assert(~iscell(ValueSet),'Cannot test ObjectMap.hasmember() with cell array if ObjectMap.ScalarValues==true');
                inMap = false(size(ValueSet));
                keyMatch = cell(size(ValueSet));
                keySet = this.keys;
                val = this.values;
                inM = false(1,this.length);
                for m=1:numel(ValueSet)
                    for n=1:this.length
                        inM(n) = val{n}==ValueSet(m);
                    end
                    inMap(m)=any(inM);
                    keyMatch{m} = keySet(inM);
                end
            else
                if ~iscell(ValueSet)
                    inM = false(1,this.length);
                    val = this.values;
                    for n=1:this.length
                        inM(n) = isequal(val{n},ValueSet);
                    end
                    inMap = any(inM);
                    keySet = this.keys;
                    keyMatch = keySet(inM);
                    
                else
                    inMap = false(size(ValueSet));
                    keyMatch = cell(size(ValueSet));
                    keySet = this.keys;
                    val = this.values;
                    inM = false(1,this.length);
                    for m=1:numel(ValueSet)
                        for n=1:this.length
                            inM(n) = isequal(val{n},ValueSet{m});
                        end
                        inMap(m) = any(inM);
                        keyMatch{m} = keySet(inM);
                    end
                end
            end
            
        end
    end
    
    %% Overload subs methods
    methods
        %% Number of Args overload
        function n = numArgumentsFromSubscript(this,s,indexingContext)
            if ~strcmp(s(1).type,'{}')
                n = builtin('numArgumentsFromSubscript',this,s,indexingContext);
                return
            end
            assert( isempty(s(1).subs) || numel(s(1).subs)==1,'Multi-dimensional indexing is not supported');

            if isempty(s(1).subs) || ischar(s(1).subs{1})&&strcmp(s(1).subs{1},':') % obj{} or obj{:}
                n = this.length;
            else
                if iscell(s(1).subs{1})
                    n = numel(s(1).subs{1});
                else
                    n=1;
                end
            end
        end

        %% Subs Ref
        function varargout = subsref(this,s)
            
            %% Forward non bracket subreferences
            if ~ismember(s(1).type,{'{}','()'})
                [varargout{1:nargout}] = builtin('subsref',this,s);
                return;
            end
            
            assert(numel(s(1).subs)<=1,'Multi-dimension indexing is not allowed');
            
            %% Get list of values to operate on
            if isempty(s(1).subs) || ischar(s(1).subs{1})&&strcmp(s(1).subs{1},':') % Handle no arguments {} and {:} --> returns all
                outvals = values(this.Map);
            elseif isempty(s(1).subs{1}) %return nothing
                outvals = {};
            else% indicies specified
                if any(~this.isKey(s(1).subs{1}))
                    error('One of the specified indices is not a valid key. Remember, multiple indices should be specified as a cell array (e.g. ObjMap{{''key1'',''key2'',''key3''}} or ObjMap(num2cell(1:3)))');
                end
                if ~iscell(s(1).subs{1}) %single index
                    outvals{1} = this.Map(s(1).subs{1});
                else
                    outvals = cell(size(s(1).subs{1}));
                    for n=1:numel(s(1).subs{1})
                        outvals{n} = this.Map(s(1).subs{1}{n});
                    end
                end
            end
            
            if length(s)>1
                try
                    for n=1:numel(outvals)
                        %n_out = numArgumentsFromSubscript(outvals{n},s(2:end),matlab.mixin.util.IndexingContext.Statement)
                        o = {builtin('subsref',outvals{n},s(2:end))}
                        if isempty(o)
                            outvals{n} = [];
                        else
                            outvals{n} = o{1};
                        end
                        %outvals{n} = builtin('subsref',outvals{n},s(2:end));
                    end
                catch ME
                    disp(ME.getReport)
                    error('Error while applying sub-ref operation to Map Values');
                end
            end
            
            if strcmp(s(1).type,'{}') %each index goes to separate output variable
                varargout = outvals;
            else %'()' %idecies are grouped in a cell array
                if length(s)==1 && this.ScalarValues
                    varargout{1} = [outvals{:}];
                else
                    varargout{1} = outvals;
                end
                
            end

        end
        
        %% subs assign
        function this = subsasgn(this,s,varargin)
            %% Forward non bracket subreferences
            if ~ismember(s(1).type,{'{}','()'})
                this = builtin('subsasgn',this,s,varargin{:});
                return;
            end
            
            assert(numel(s(1).subs)<=1,'Multi-dimension indexing is not allowed');
            
            %% Get List of indecies
            %class(s(1).subs)
            %disp(s(1).subs)
            if isempty(s(1).subs) || ischar(s(1).subs{1})&&strcmp(s(1).subs{1},':')
                keySet = this.keys;
            elseif iscell(s(1).subs{1})
                keySet = s(1).subs{1};
            elseif isempty(s(1).subs{1})
                keySet = {};
            else
                %if ischar(
                %assert(isa(s(1).subs{1},this.KeyType),sprintf('subscript is not of type %s',this.KeyType));
                keySet = {s(1).subs{1}};
            end
            
            if length(s)>1
                assert(all(this.isKey(keySet)),'Keys not defined before set operation. All keys must be previously defined when setting fields/properties of linked valued');
            end
            
            %% Remove values if Obj(...) = []
            if length(s)==1 && strcmp(s(1).type,'()') && numel(varargin)==1 && isnumeric(varargin{1})&& isempty(varargin{1}) %Obj(...)=[]
                this.remove(keySet);
                return;
            end
            
            %% Set Scalar List with Obj(...) = ValueTypeArray
            if strcmp(s(1).type,'()') && length(s)==1 && this.ScalarValues && isa(varargin{1},this.ValueType) %behave like regular array, required Obj(args)=ArrayWithSizeofArgs
                if ~isscalar(varargin{1}) && numel(keySet)~=numel(varargin{1})
                    error('Setting ObjectMap for type %s with ScalarValue=true to an array of %s must have same number of LHS and RHS elements',this.ValueType,this.ValueType);
                end
                for n=1:numel(keySet)
                    if numel(varargin{1})==1
                        this.setValue(keySet{n},varargin{1})
                    else
                        this.setValue(keySet{n},varargin{1}(n));
                    end
                end
                return;
            end
            
            %% Format for obj(...)={} or obj(...).__...={}
            if strcmp(s(1).type,'()')
                if iscell(varargin{1})
                    assert(numel(varargin{1})==numel(keySet),'Setting: obj(...)={} requires the number of cells in RHS match the number of keys in LHS');
                    varargin = varargin{1};
                end
            end
            
            %% Set Values
            for n=1:numel(keySet)
                if numel(varargin)==1
                    val = varargin{1};
                else
                    val = varargin{n};
                end

                if length(s)>1
                    this.Map(keySet{n}) = builtin('subsasgn',this.Map(keySet{n}),s(2:end),val);
                else
                    this.setValue(keySet{n},val);
                end
            end

        end
    end
    
    
end