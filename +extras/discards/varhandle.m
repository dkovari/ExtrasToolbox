classdef varhandle < matlab.mixin.SetGet & matlab.mixin.Copyable
% Handle class wrapper for arbitraty variable
%   Construct a handle object around a variable and overloads the
%   sub-assign operators so that you can access the data from the handle
%   name directly. In essences this gives you a "pass-by-reference" wrapper
%   for any data type
%
%   Unfortunately, due to MATLAB limitations the (non-substript) assigment 
%   operator cannot be overloaded. Consequently the following will clear 
%   the handle instead of reseting the internal data
%   >> vh = varhandle([1,2,3]); %create varhandle holding [1,2,3]
%   >> vh(3) = 8; %now holding [1,2,8]
%   >> vh = [5,6,7]; %vh is nolonger a handle object, just a flat array
%
%   If you want to reassign the data in vh you can use the assign() method
%   >> vh = varhandle([1,2,3]); %create varhandle holding [1,2,3]
%   >> assign(vh,[9,10,11]); %vh now contains [9,10,11]
%
% NOTE: this is a work in progress
%% Copyright 2018 Daniel Kovari, Emory University
%   All rights reserved
    
    properties (Access=protected)
        varhandle_InternalData
    end
    
    %% constructor
    methods
        function this = varhandle(that)
        % Contstruct handle around data
        %   passed variable (that) will be wrapped in a handle class
            if isobject(that)&&isa(that,'handle')
                warning('variable is already a handle, will not convert');
                this = that;
                return;
            end
            set(this,'varhandle_InternalData',that);
        end
    end
    
    %% assignment
    methods
        function assign(vh,data)
        % Assign data to the varhandle
            set(vh,'varhandle_InternalData',data);
        end
    end
    
    %% overloads
    methods
        function n = numel(this,varargin)
            n = numel(this.varhandle_InternalData,varargin{:});
        end
        function varargout = size(this,varargin)
            [varargout{1:nargout}] = size(this.varhandle_InternalData,varargin{:});
        end
        function c = dataclass(this)
            c = class(this.varhandle_InternalData);
        end
        function disp(this)
            if ~isvalid(this)
                builtin('disp',this);
                return;
            end
            fprintf(' %s containing %s:\n',mfilename('class'),class(this.varhandle_InternalData));
            disp(this.varhandle_InternalData);
        end
        
        %% is*
        function tf = iscell(this)
            tf = iscell(this.varhandle_InternalData);
        end
        function tf = isstruct(this)
            tf = isstruct(this.varhandle_InternalData);
        end
        function tf = isscalar(this)
            tf = isscalar(this.varhandle_InternalData);
        end
        function tf = isnumeric(this)
            tf = isnumeric(this.varhandle_InternalData);
        end
        function tf = isa(this,dataType)
            tf = builtin('isa',this,dataType)||isa(this.varhandle_InternalData,dataType);
        end
        function tf = iscellstr(this)
            tf = iscellstr(this.varhandle_InternalData);
        end
        function tf = ischar(this)
            tf = ischar(this.varhandle_InternalData);
        end
        function tf = isdatetime(this)
            tf = isdatetime(this.varhandle_InternalData);
        end
        function tf = isduration(this)
            tf = isduration(this.varhandle_InternalData);
        end
        function tf = isenum(this)
            tf = isenum(this.varhandle_InternalData);
        end
        function tf = isfloat(this)
            tf = isfloat(this.varhandle_InternalData);
        end
        function tf = isgraphics(this)
            tf = isgraphics(this.varhandle_InternalData);
        end
        function tf = isinteger(this)
            tf = isinteger(this.varhandle_InternalData);
        end
        function tf = isjava(this)
            tf = isjava(this.varhandle_InternalData);
        end
        function tf = islogical(this)
            tf = islogical(this.varhandle_InternalData);
        end
        function tf = isobject(this)
            tf = isobject(this.varhandle_InternalData);
        end
        function tf = isreal(this)
            tf = isreal(this.varhandle_InternalData);
        end
        function tf = isstring(this)
            tf = isstring(this.varhandle_InternalData);
        end
        function tf = istable(this)
            tf = istable(this.varhandle_InternalData);
        end
        function tf = istimetable(this)
            tf = istimetable(this.varhandle_InternalData);
        end
        function tf = iscalendarduration(this)
            tf = iscalendarduration(this.varhandle_InternalData);
        end
        function tf = iscategorical(this)
            tf = iscategorical(this.varhandle_InternalData);
        end
        function tf = isappdata(this)
            tf = isappdata(this.varhandle_InternalData);
        end
        function tf = isbetween(this,varargin)
            tf = isbetween(this.varhandle_InternalData,varargin{:});
        end
        function tf = iscolumn(this,varargin)
            tf = iscolumn(this.varhandle_InternalData,varargin{:});
        end
        function tf = iscom(this,varargin)
            tf = iscom(this.varhandle_InternalData,varargin{:});
        end
        function tf = isdiag(this,varargin)
            tf = isdiag(this.varhandle_InternalData,varargin{:});
        end
        function tf = isdst(this,varargin)
            tf = isdst(this.varhandle_InternalData,varargin{:});
        end
        function tf = isempty(this,varargin)
            tf = isempty(this.varhandle_InternalData,varargin{:});
        end
        function tf = isequal(this,varargin)
            tf = isequal(this.varhandle_InternalData,varargin{:});
        end
        function tf = isequaln(this,varargin)
            tf = isequaln(this.varhandle_InternalData,varargin{:});
        end
        function tf = isevent(this,varargin)
            tf = isevent(this.varhandle_InternalData,varargin{:});
        end
        function tf = isfile(this,varargin)
            tf = isfile(this.varhandle_InternalData,varargin{:});
        end
        function tf = isfinite(this,varargin)
            tf = isfinite(this.varhandle_InternalData,varargin{:});
        end
        function tf = isfolder(this,varargin)
            tf = isfolder(this.varhandle_InternalData,varargin{:});
        end
        function tf = ishermitian(this,varargin)
            tf = ishermitian(this.varhandle_InternalData,varargin{:});
        end
        function tf = ishold(this,varargin)
            tf = ishold(this.varhandle_InternalData,varargin{:});
        end
        function tf = isinf(this,varargin)
            tf = isinf(this.varhandle_InternalData,varargin{:});
        end
        function tf = iskeyword(this,varargin)
            tf = iskeyword(this.varhandle_InternalData,varargin{:});
        end
        function tf = isleter(this,varargin)
            tf = isleter(this.varhandle_InternalData,varargin{:});
        end
        function tf = ismatrix(this,varargin)
            tf = ismatrix(this.varhandle_InternalData,varargin{:});
        end
        function tf = ismember(this,varargin)
            tf = ismember(this.varhandle_InternalData,varargin{:});
        end
        function tf = ismethod(this,varargin)
            tf = ismethod(this.varhandle_InternalData,varargin{:});
        end
        function tf = ismissing(this,varargin)
            tf = ismissing(this.varhandle_InternalData,varargin{:});
        end
        function tf = isnan(this,varargin)
            tf = isnan(this.varhandle_InternalData,varargin{:});
        end
        function tf = isnat(this,varargin)
            tf = isnat(this.varhandle_InternalData,varargin{:});
        end
        function tf = isordinal(this,varargin)
            tf = isordinal(this.varhandle_InternalData,varargin{:});
        end
        function tf = isprime(this,varargin)
            tf = isprime(this.varhandle_InternalData,varargin{:});
        end
        function tf = isprop(this,varargin)
            tf = isporp(this.varhandle_InternalData,varargin{:});
        end
        function tf = isStringScalar(this,varargin)
            tf = isStringScalar(this.varhandle_InternalData,varargin{:});
        end
        function tf = isstrprop(this,varargin)
            tf = isstrprop(this.varhandle_InternalData,varargin{:});
        end
        function tf = isstudent(this,varargin)
            tf = isstudent(this.varhandle_InternalData,varargin{:});
        end
        function tf = issymmetric(this,varargin)
            tf = issgmmetric(this.varhandle_InternalData,varargin{:});
        end
        function tf = istril(this,varargin)
            tf = istril(this.varhandle_InternalData,varargin{:});
        end
        function tf = istriu(this,varargin)
            tf = istriu(this.varhandle_InternalData,varargin{:});
        end
        function tf = isundefined(this,varargin)
            tf = isundefined(this.varhandle_InternalData,varargin{:});
        end
        function tf = isvarname(this,varargin)
            tf = isvarname(this.varhandle_InternalData,varargin{:});
        end
        function tf = isweekend(this,varargin)
            tf = isweekend(this.varhandle_InternalData,varargin{:});
        end
        
        %% Numeric Conversion
        function out = double(this,varargin)
            out = double(this.varhandle_InternalData,varargin{:});
        end
        function out = single(this,varargin)
            out = single(this.varhandle_InternalData,varargin{:});
        end
        function out = int8(this,varargin)
            out = int8(this.varhandle_InternalData,varargin{:});
        end
        function out = int16(this,varargin)
            out = int16(this.varhandle_InternalData,varargin{:});
        end
        function out = int32(this,varargin)
            out = int32(this.varhandle_InternalData,varargin{:});
        end
        function out = int64(this,varargin)
            out = int64(this.varhandle_InternalData,varargin{:});
        end
        function out = uint8(this,varargin)
            out = uint8(this.varhandle_InternalData,varargin{:});
        end
        function out = uint16(this,varargin)
            out = uint16(this.varhandle_InternalData,varargin{:});
        end
        function out = uint32(this,varargin)
            out = uint32(this.varhandle_InternalData,varargin{:});
        end
        function out = uint64(this,varargin)
            out = uint64(this.varhandle_InternalData,varargin{:});
        end
        function out = cast(this,varargin)
            out = cast(this.varhandle_InternalData,varargin{:});
        end
        function out = typecast(this,varargin)
            out = typecast(this.varhandle_InternalData,varargin{:});
        end
        function out = logical(this,varargin)
            out = logical(this,varargin{:});
        end
    end
    
    
    %% subsref & subsassgn
    methods
        function varargout = subsref(this,s)
            if strcmp(s(1).type,'.') && strcmp(s(1).subs,'varhandle_InternalData')
                [varargout{1:nargout}] = builtin('subsref',this,s);
            else
                [varargout{1:nargout}] = builtin('subsref',this.varhandle_InternalData,s);
            end
        end
        
        function this = subsasgn(this,s,varargin)
            if strcmp(s(1).type,'.') && strcmp(s(1).subs,'varhandle_InternalData')
                builtin('subsasgn',this,s,varargin{:});
            else
                set(this,'varhandle_InternalData',builtin('subsasgn',this.varhandle_InternalData,s,varargin{:}));
            end
        end
    end
end