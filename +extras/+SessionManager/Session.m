classdef Session < handle
% extras.SessionManager.Session
% Class interface for communicating with C++ objects that outlive calls to
% a mex function built using the SessionManager framework.
% 
% Mex function should be written using the framework included in
% mexDispatch.h and ObjectManager.h
%
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

    
    properties(SetAccess=immutable,Hidden)
        intPointer = 0;
        MEX_function;
    end
    
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
    end
end