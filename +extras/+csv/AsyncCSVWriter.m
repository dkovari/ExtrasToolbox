classdef AsyncCSVWriter < extras.Async.ParameterProcessor_nowriter 
%% extras.csv.AsyncCSVWriter
% Interface to Mex-based system for asynchronously writing data as csv file


    properties(Access=protected)
        AsyncCSVWriter_initialized (1,1) logical= false;
    end
    %% Constructor
    methods
        function this = AsyncCSVWriter(filepath,file_mode)
            % Create MxFileWriter
            % optionally specify destination file path and open file
            % automatically adds mxf.gz extension if not present
            
            this@extras.Async.ParameterProcessor_nowriter (@extras.csv.AsyncCSVWriter_mex); 
            this.Name = 'AsyncCSVWriter';
            if nargin==1
                this.openFile(filepath);
            elseif nargin==2
                this.openFile(filepath,file_mode);
            end
            
            %% Setup Default Parameters
            this.setParameters(struct('NumericFormat','%G','StringFormat','"%s"'));
            this.AsyncCSVWriter_initialized = true;
        end
    end
    
    %% private set
    properties(SetObservable=true,AbortSet=true,SetAccess=private)
        IsFileOpen
        Filepath
        FileAccessMode
    end
    
    %% public methods
    methods
        function openFile(this,fpth,fileAccessMode)
            % open a file for writing.
            if nargin>2
                this.runMethod('openFile',fpth,fileAccessMode)
            else
                this.runMethod('openFile',fpth);
            end
            
            this.IsFileOpen = this.runMethod('isFileOpen');
            this.Filepath = this.runMethod('filepath');
            this.FileAccessMode = this.runMethod('fileAccessMode');
        end
        function closeFile(this)
            % close file
            
            this.runMethod('closeFile');
            this.IsFileOpen = this.runMethod('isFileOpen');
        end
        function writeRow(this,varargin)
            % write data to file
            
            if(nargin>1)
                this.pushTask(varargin{:});
            end
        end
    end
    
    %% Override inherited
    methods
        
    end
    
     %% Hide Inherited Methods
     methods(Hidden)
        function setParameters(this,varargin)
        % Hide setParameters
        
            if ~this.AsyncCSVWriter_initialized
                %% call inherited setParameters
                setParameters@extras.Async.ParameterProcessor_nowriter(this,varargin{:});
            else
                p=inputParser;
                addParameter(p,'NumericFormat',this.Parameters.NumericFormat);
                addParameter(p,'StringFormat',this.Parameters.StringFormat);
                parse(p,varargin{:});
                setParameters@extras.Async.ParameterProcessor_nowriter(this,p.Results);
            end
        end
     end
end