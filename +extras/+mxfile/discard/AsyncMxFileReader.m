classdef AsyncMxFileReader < extras.SessionManager.Session
%% extras.mxfile.AsyncMxFileReader
% Interface to Mex-based system for asynchronouslyreading data from mxf.gz 
% type files.
% The .mxf.gz filetype is essentially a gzip compressed stream of MATLAB
% arrays.
% The arrays are serialized such that cell and struct with nested data are
% written as a flat sequence of fundamental (numeric or char) arrays.
%
% mxf.gz files can be created using MxFileWriter or AsyncMxFileWriter

    %% Constructor
    methods
        function this = AsyncMxFileReader(filepath)
            % Create MxFileWriter
            % optionally specify destination file path and open file
            % automatically adds mxf.gz extension if not present
            
            this@extras.SessionManager.Session(@extras.mxfile.AsyncMxFileReader_mex);
                        
            %% setup error timer
            try
                delete(this.ErrorCheckTimer);
            catch
            end
            this.ErrorCheckTimer = timer('ObjectVisibility','off',...
                                     'BusyMode','drop',...
                                     'ExecutionMode','fixedSpacing',...
                                     'Tag','AsyncProcessor-ErrorCheckTimer',...
                                     'Period',this.ErrorCheckTimerPeriod,...
                                     'ErrorFcn',@(~,err) disp(err),...
                                     'TimerFcn',@(~,~) this.ErrorCheckTimerCallback);
             
             %% Setup the Results TImer
             try
                 delete(this.ResultsCheckTimer)
             catch
             end
             this.ResultsCheckTimer = timer('ObjectVisibility','off',...
                                        'BusyMode','drop',...
                                        'ExecutionMode','fixedSpacing',...
                                        'Tag','AsyncProcessor-ResultsCheckTimer',...
                                        'Period',this.ResultsCheckTimerPeriod,...
                                        'ErrorFcn',@(~,err) disp(err),...
                                        'TimerFcn',@(~,~) this.ResultsCheckTimerCallback);
           
            if nargin>0
                this.openFile(filepath);
            end
        end
    end
    
    %%Destructor
    methods
        function delete(this)
            
            %% stop and delete timers
            try
            stop(this.ResultsCheckTimer)
            catch
            end
            delete(this.ResultsCheckTimer);
            try
            stop(this.ErrorCheckTimer)
            catch
            end
            delete(this.ErrorCheckTimer);
        end
    end
    
    %% File Info
    properties(SetObservable=true,AbortSet=true,SetAccess=private)
        IsFileOpen = false;
        Filepath = '';
        AtEndOfFile = false;
        FileSize = 0; %returns the (compressed) size of the file, in bytes
        CurrentByte = NaN; %current position in the (compressed) file
        LoadProgress = 0; %fraction of the (compressed) data that has been read
        LoadingData = false;
    end
    
    %% Error Related Items
    events
        ErrorOccured %Event thrown when error has occured in the processor
    end
    properties (SetObservable=true,AbortSet=true)
        ErrorCheckTimerPeriod = 0.75; %period at which to check for errors
    end
    methods %set period
        function set.ErrorCheckTimerPeriod(this,val)
            assert(isscalar(val)&&isnumeric(val)&&val>0,'ErrorCheckTimerPeriod must be numeric scalar with value>0');
            stop(this.ErrorCheckTimer)
            this.ErrorCheckTimer.Period = val;
            this.ErrorCheckTimerPeriod = val;
            this.restartErrorCheckTimer();
        end
    end
    properties(SetAccess=protected,SetObservable=true,AbortSet=true)
        LastErrorMessage %struct containing last error message
    end
    properties (Access=protected)
        ErrorCheckTimer = timer('ObjectVisibility','off',...
         'BusyMode','drop',...
         'ExecutionMode','fixedSpacing',...
         'Tag','AsyncProcessor-ErrorCheckTimer');
    end
    methods (Access=protected)
        function restartErrorCheckTimer(this)
        %internal use - Restart the error check timer
            
            if strcmpi(this.ErrorCheckTimer.Running,'on') %timer is already running
                return;
            end
            
            if this.wasErrorThrown()
                this.DispatchErrorMessage();
            end

            try
                start(this.ErrorCheckTimer);
            catch ME
                disp(ME.getReport);
                warning('could not restard ErrorCheckTimer');
            end
        end
        function DispatchErrorMessage(this)
        % internal use - check and dispatch error messages        
            errMsg = this.getError();
            if ~isempty(errMsg)
                this.LastErrorMessage = errMsg;
                notify(this,'ErrorOccured',extras.Async.AsyncProcessorError(errMsg));
            end
            this.clearError();
        end
    end
    methods (Hidden) %ErrorCheckTimer callback
        function ErrorCheckTimerCallback(this)
            if ~isvalid(this) %cancel if object has been deleted
                return;
            end

            if this.wasErrorThrown() %check for pending errors and dispatch
                this.DispatchErrorMessage();
            end

            %% Stop timer if task thread has stopped or there are no more tasks
            if ~this.LoadingData
                stop(this.ErrorCheckTimer);
            end
        end
    end

    %% Results Related properties
    properties(SetAccess=private,SetObservable=true,AbortSet=true)
        Data = {};
    end
    events
        FinishedLoadingData
    end
    properties(Access=protected)
        ResultsCheckTimer = timer('ObjectVisibility','off',...
            'BusyMode','drop',...
            'ExecutionMode','fixedSpacing',...
            'Tag','AsyncProcessor-ResultsCheckTimer');
    end
    properties (SetObservable=true,AbortSet=true)
        ResultsCheckTimerPeriod = 0.75; %period at which to check for new results
    end
    methods %set period
        function set.ResultsCheckTimerPeriod(this,val)
            assert(isscalar(val)&&isnumeric(val)&&val>0,'ResultsCheckTimerPeriod must be numeric scalar with value>0');
            stop(this.ResultsCheckTimer)
            this.ResultsCheckTimer.Period = val;
            this.ResultsCheckTimerPeriod = val;
            this.restartResultsCheckTimer();
        end
    end
    methods (Access=protected)
        function restartResultsCheckTimer(this)
            if ~isvalid(this)
                return
            end
        %internal use - restart results timer
            if strcmpi(this.ResultsCheckTimer.Running,'on') %timer is already running
                return;
            end
            try
                start(this.ResultsCheckTimer);
            catch ME
                disp(ME.getReport);
                warning('could not restard ResultsCheckTimer');
            end
        end
    end
    methods (Hidden) %Timer callback
        function ResultsCheckTimerCallback(this)
            if ~isvalid(this) %cancel if object has been deleted
                return;
            end
            
            %% copy loaded data
            nRes = this.runMethod('numberOfArraysLoaded'); %get number of arrays remaining in bufer
            if nRes > 0 %there is data waiting for us
                this.Data = cat(1,this.Data,this.runMethod('getLoadedDataAndClear'));
            end
            
            this.CurrentByte = this.runMethod('getPositionInFile');
            this.LoadProgress = this.runMethod('loadProgress');
            
            %% check if at end of file
            this.AtEndOfFile = this.runMethod('isEOF');
            
            if this.AtEndOfFile
                notify(this,'FinishedLoadingData');
            end
            
            %% check if thread is running
            this.LoadingData = this.runMethod('threadRunning');
            
            if ~this.LoadingData %thread stopped, stop timer
                stop(this.ResultsCheckTimer)
            end
            
        end
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
            
            this.Data = {};
            this.restartErrorCheckTimer();
            this.LoadingData = this.runMethod('threadRunning');
        end
        function loadData(this)
            this.IsFileOpen = this.runMethod('isFileOpen');
            this.runMethod('loadData');
            this.restartResultsCheckTimer();
            this.restartErrorCheckTimer();
            this.LoadingData = this.runMethod('threadRunning');
        end
        function resume(this)
            this.loadData();
        end
        function cancel(this)
            this.runMethod('cancel');
            this.IsFileOpen = this.runMethod('isFileOpen');
            this.AtEndOfFile = this.runMethod('isEOF');
            this.FileSize = this.runMethod('getCompressedSize');
            this.CurrentByte = this.runMethod('getPositionInFile');
            this.LoadProgress = this.runMethod('loadProgress');
            stop(this.ResultsCheckTimer);
            
            nRes = this.runMethod('numberOfArraysLoaded'); %get number of arrays remaining in bufer
            if nRes > 0 %there is data waiting for us
                this.Data = cat(1,this.Data,this.runMethod('getLoadedDataAndClear'));
            end
            this.LoadingData = this.runMethod('threadRunning');
        end
        function closeFile(this)
            % close file
            this.runMethod('closeFile');
            this.IsFileOpen = this.runMethod('isFileOpen');
            this.LoadingData = this.runMethod('threadRunning');
        end
    end
    
    %% Error related functions
    methods(Hidden)
        function tf = wasErrorThrown(this)
        % T/F is error was thrown
            tf = this.runMethod('wasErrorThrown');
        end
        function err = getError(this)
        % Get error from processor, if no error was thrown err is empty
        % clears the error buffer after calling
            err = this.runMethod('getError');
        end
        function clearError(this)
        % clear error flag and error struct from buffer
            this.runMethod('clearError');
        end
    end
end