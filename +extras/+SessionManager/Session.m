classdef Session < handle
%% extras.SessionManager.Session
% Class interface for communicating with C++ objects that outlive calls to
% a mex function built using the SessionManager framework.
% 
% Mex function should be written using the framework included in
% mexDispatch.h and ObjectManager.h
%
% NOTE: If using Session in a diamond inheritance structure, be sure to
%  pass the constructor arguments to ONLY ONE of the parent classes
%  inheriting from extras.SessionManager.Session.
%  For example:
%      classdef B < extras.SessionManager.Session
%           ...
%
%       classdef C < extras.SessionManager.Session
%           ...
%
%       classdef D < B & C
%           methods
%               function this = D()
%                   this@C(); 
%                   this@B(@YOUR_MEX_FUNCTION,...args...);
%                   ...
%               end
%           end
%       end
%
% Example C++ Code
%=================================
% struct myClass{
% int myVal = 0;
% int retOne(){ return 1;}
% };
% 
% #include "mexDispatch.h"
% #include "ObjectManager.h"
% 
% ObjectManager<myClass> manager;
% 
% // Create new instance
% MEX_DEFINE(new) (int nlhs, mxArray* plhs[],
% 	int nrhs, const mxArray* prhs[]) {
% 	int64_t val = manager.create(new myClass);
% 	plhs[0] = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
% 	*(((int64_t*)mxGetData(plhs[0]))) = val;
% }
% 
% //delete instance
% MEX_DEFINE(delete) (int nlhs, mxArray* plhs[],
% 	int nrhs, const mxArray* prhs[])
% {
% 	if (nrhs < 1) {
% 		throw(std::runtime_error("requires intptr argument specifying object to destruct"));
% 	}
% 	manager.destroy(prhs[0]);
% }
% 
% //define retOne method
% MEX_DEFINE(retOne) (int nlhs, mxArray* plhs[],
% 	int nrhs, const mxArray* prhs[])
% {
%     if (nrhs < 1) {
% 		throw(std::runtime_error("requires intptr argument specifying object"));
% 	}
%     plhs[0] = mxCreateDoubleScalar(manager.get(prhs[0])->retOne());
% }
% 
% // Implement mexFunction using dispatch
% MEX_DISPATCH

    
    properties(SetAccess=private,Hidden)
        intPointer = 0; %UINT66_T value storing address (uint64_ptr) of the associated mex object
        MEX_function; %function handle for mexFunction managing the mex object
    end
    properties(SetAccess=private)
         SESSION_OBJECT_FULLY_CONSTRUCTED (1,1) logical = false; %flag indicating if mex object has been fully constructed.
    end
    
    %% Change MEX_function
%     methods(Access=protected)
%         function change_MEX_function(this,MEX_NAME,varargin)
%             % use this method to change the underlying MEX_function
%             % provides the ability for a subclass to redefine the
%             % MEX_function instead of using the one inherited
%             
%             this.MEX_function('delete',this.intPointer);
%             assert(isa(MEX_NAME,'function_handle')||...
%                 exist(MEX_NAME,'file')==3,...
%                 'MEX_NAME must be a function handle to a mex function or the name of a mex function');
%             
%             if ischar(MEX_NAME)
%                 this.MEX_function = str2func(MEX_NAME);
%             else
%                 this.MEX_function = MEX_NAME;
%             end
%             
%             this.intPointer = this.MEX_function('new',varargin{:});
%         end
%     end
    
    %% Create/Delete
    methods
        function this = Session(MEX_NAME,varargin)
        % SESSION Construct instance of this class, and creates
        % corresponding mex object using the mexInterface function
        % specified by MEX_NAME
        % Input Arguments:
        %   MEX_NAME - Funciton handle or char array
        %   [...] - Additional arguments are passed to mex object
        %   constructor
        % NOTE:
        %   DO NOT SPECIFY ARGUMENTS if inheriting in a diamond.
        %   If you have a "diamond" inheritance structure (e.g. D<B,C; B<A; C<A), MATLAB creates
        %   two instances of the initial base class (A in example);
        %   however, it does not delete the initially created extra object.
        %   To avoid problems with extra mex objects floating around,
        %   constructing Session without any arguments does not set the
        %   MEX_FUNCTION and DOES NOT create the mex object.
        
            if nargin<1
                return;
            end
            
            assert(isa(MEX_NAME,'function_handle')||...
                exist(MEX_NAME,'file')==3,...
                'MEX_NAME must be a function handle to a mex function or the name of a mex function');
            
            if ischar(MEX_NAME)
                this.MEX_function = str2func(MEX_NAME);
            else
                this.MEX_function = MEX_NAME;
            end
            
            this.intPointer = this.MEX_function('new',varargin{:});
            
            this.SESSION_OBJECT_FULLY_CONSTRUCTED = true;
        end
        
        function delete(this)
            if ~this.SESSION_OBJECT_FULLY_CONSTRUCTED
                return;
            end
            this.MEX_function('delete',this.intPointer);
        end
    end
    
    %% Run Method
    methods(Hidden)
        function varargout = runMethod(this,METHOD_NAME,varargin)
            if(~this.SESSION_OBJECT_FULLY_CONSTRUCTED)
                warning('Session Object has not been fully constructed. Cannot run method');
                return;
            end
            assert(ischar(METHOD_NAME),'METHOD_NAME should be a char array specifying method name');
            try
                [varargout{1:nargout}] = this.MEX_function(METHOD_NAME,this.intPointer,varargin{:});
            catch ME
                METHOD_NAME
                varargin{:}
                disp(ME.getReport);
                rethrow(ME);
            end
        end
        function methNames = getMethodNames(this)
            %hidden method which returns a list of the valid method
            %names
            methNames = runMethod(this,'getMethodNames');
        end
    end
    methods(Access=protected,Hidden)
        function change_MEX_FUNCTION(this,MEX_NAME,varargin)
            assert(isa(MEX_NAME,'function_handle')||...
                exist(MEX_NAME,'file')==3,...
                'MEX_NAME must be a function handle to a mex function or the name of a mex function');
            
            this.MEX_function('delete',this.intPointer);
            
            if ischar(MEX_NAME)
                this.MEX_function = str2func(MEX_NAME);
            else
                this.MEX_function = MEX_NAME;
            end
            
            this.intPointer = this.MEX_function('new',varargin{:});
            this.SESSION_FULLY_CONSTRUCTED = true;
        end
    end
end