classdef AsyncMxFileWriter < extras.Async.AsyncProcessor 
%% extras.mxfile.AsyncMxFileWriter
% Interface to Mex-based system for asynchronously writing data to mxf.gz
% type files.
% The .mxf.gz filetype is essentially a gzip compressed stream of MATLAB
% arrays.
% The arrays are serialized such that cell and struct with nested data are
% written as a flat sequence of fundamental (numeric or char) arrays.
%
% The data from mxf.gz files can be loaded with the accompanying
% MxFileReader.

    %% Constructor
    methods
        function this = AsyncMxFileWriter(filepath)
            % Create MxFileWriter
            % optionally specify destination file path and open file
            % automatically adds mxf.gz extension if not present
            
            this@extras.Async.AsyncProcessor (@extras.mxfile.AsyncMxFileWriter_mex); 
            if nargin>0
                this.openFile(filepath);
            end
        end
    end
    
    %% private set
    properties(SetObservable=true,AbortSet=true,SetAccess=private)
        IsFileOpen
        Filepath
    end
    
    %% public methods
    methods
        function openFile(this,fpth)
            % open a file for writing.
            % automatically adds mxf.gz extension if not present
            
            this.runMethod('openFile',fpth);
            this.IsFileOpen = this.runMethod('isFileOpen');
            this.Filepath = this.runMethod('filepath');
        end
        function closeFile(this)
            % close file
            
            this.runMethod('closeFile');
            this.IsFileOpen = this.runMethod('isFileOpen');
        end
        function writeArrays(this,varargin)
            % write data to file
            
            if(nargin>1)
                this.pushTask(varargin{:});
            end
        end
    end
end