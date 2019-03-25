classdef Session < handle
%% extras.SessionManager.Session
% Class interface for communicating with C++ objects that outlive calls to
% a mex function built using the SessionManager framework.
% 
% Mex function should be written using the framework included in
% mexDispatch.h and ObjectManager.h
%
% NOTE: Do not use Session objects in a diamon hierarchy.
%       The destruct for every initialized Session object must be called,
%       otherwise the c++ objects created by the mex file will not be
%       destroyed and the mex file cannot be cleared.
%       If a diamond is used, then multiple Session objects will
%       be created, but only one may be deleted, meaning there will be an
%       unreferenceable c++ object floating around.
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
        intPointer = 0;
        MEX_function;
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
            assert(isa(MEX_NAME,'function_handle')||...
                exist(MEX_NAME,'file')==3,...
                'MEX_NAME must be a function handle to a mex function or the name of a mex function');
            
            if ischar(MEX_NAME)
                this.MEX_function = str2func(MEX_NAME);
            else
                this.MEX_function = MEX_NAME;
            end
            
            this.intPointer = this.MEX_function('new',varargin{:});
        end
        
        function delete(this)
            this.MEX_function('delete',this.intPointer);
        end
    end
    
    %% Run Method
    methods(Hidden)
        function varargout = runMethod(this,METHOD_NAME,varargin)
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
        end
    end
end