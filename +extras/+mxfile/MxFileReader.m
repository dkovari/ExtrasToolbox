classdef MxFileReader < extras.SessionManager.Session
%% extras.mxfile.MxFileReader
% Interface to Mex-based system for reading data from mxf.gz type files.
% The .mxf.gz filetype is essentially a gzip compressed stream of MATLAB
% arrays.
% The arrays are serialized such that cell and struct with nested data are
% written as a flat sequence of fundamental (numeric or char) arrays.
%
% mxf.gz files can be created using MxFileWriter

    %% Constructor
    methods
        function this = MxFileReader(filepath)
            % Create MxFileWriter
            % optionally specify destination file path and open file
            % automatically adds mxf.gz extension if not present
            
            this@extras.SessionManager.Session(@extras.mxfile.MxFileReader_mex);
            if nargin>0
                this.openFile(filepath);
            end
        end
    end
    
    %% private set
    properties(SetObservable=true,AbortSet=true,SetAccess=private)
        IsFileOpen = false;
        Filepath
        AtEndOfFile = false;
        FileSize = 0; %returns the (compressed) size of the file, in bytes
        CurrentByte = NaN; %current position in the (compressed) file
        LoadProgress = 0; %fraction of the (compressed) data that has been read
    end
    
    %% public methods
    methods
        function openFile(this,fpth)
            % open a file for writing.
            % automatically adds mxf.gz extension if not present
            
            this.runMethod('openFile',fpth);
            this.IsFileOpen = this.runMethod('isFileOpen');
            this.Filepath = this.runMethod('filepath');
            this.AtEndOfFile = this.runMethod('isEOF');
            this.FileSize = this.runMethod('getCompressedSize');
            this.CurrentByte = this.runMethod('getPositionInFile');
            this.LoadProgress = this.runMethod('loadProgress');
        end
        function closeFile(this)
            % close file
            this.runMethod('closeFile');
            this.IsFileOpen = this.runMethod('isFileOpen');
            this.AtEndOfFile = false;
        end
        function data = readNextArray(this,varargin)
            % read next array from file
            try
                this.IsFileOpen = this.runMethod('isFileOpen');
                data = this.runMethod('readNextArray');
                this.AtEndOfFile = this.runMethod('isEOF');
                this.CurrentByte = this.runMethod('getPositionInFile');
                this.LoadProgress = this.runMethod('loadProgress');
                if this.AtEndOfFile
                    warning('Reached the end of File: %s',this.Filepath);
                end
            catch ME
                try
                    this.AtEndOfFile = this.runMethod('isEOF');
                    this.AtEndOfFile = this.runMethod('isEOF');
                    this.CurrentByte = this.runMethod('getPositionInFile');
                    this.LoadProgress = this.runMethod('loadProgress');
                catch
                end
                rethrow(ME);
            end
        end
        
        function data = readAll(this)
            data = {};
            
            t1 = tic();
            hWB = [];
            while ~this.AtEndOfFile
                data{end+1} = this.readNextArray();
                if this.AtEndOfFile
                    data(end) = [];
                end
                if toc(t1)>0.8
                    if isempty(hWB)
                        hWB = waitbar(this.LoadProgress,sprintf('Loading Data From: %s', this.Filepath));
                    else
                        if ~isvalid(hWB)
                            warning('Reading %s canceled by user',this.Filepath);
                            return;
                        else
                            waitbar(this.LoadProgress,hWB);
                        end
                    end
                end
            end
            delete(hWB);
        end
    end

end