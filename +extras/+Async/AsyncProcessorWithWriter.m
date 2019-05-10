classdef (Abstract) AsyncProcessorWithWriter < extras.Async.AsyncProcessor
%% extras.SessionManager.AsyncProcessorWithWriter
% Interface to Mex-based Asynchronous data processor with integrated File
% writer
%   This class creates the interface to a MEX SessionManager object
%   providing the thread processor and results writer.
%   
%  The associated MEX SessionManager object should be derived from
%   mex::AsyncProcessorWithWriter defined in 
%       ...\include\extras\async\AsyncProcessorWithWriter.hpp
%
%   Communication from MATLAB to the processor object is handed by
%   background timers.
%
%   Users can add tasks to the processor queue using pushTask()
%
%   Results can be retrieved via the extras.QueueDispatcher system.
%       Users should create an extras.Queue type object and subscribe to
%       events using the method
%           AsyncProcessorWithWriter.registerQueue(YOUR_QUEUE)
%       See extras.QueueDispatcher for more details
%% Copyright 2018 Daniel T. Kovari, Emory University
%   All rights reserved.  

    %% create
    methods
        function this = AsyncProcessorWithWriter(MEX_FUNCTION)
            this@extras.Async.AsyncProcessor(MEX_FUNCTION)
            this.Name = 'AsyncProcessorWithWriter'; %Change Name
            
            %% setup writer error timer
            try
                delete(this.ResultsWriterErrorTimer);
            catch
            end
            this.ResultsWriterErrorTimer = timer('ObjectVisibility','off',...
                                     'BusyMode','drop',...
                                     'ExecutionMode','fixedSpacing',...
                                     'Tag','AsyncProcessorWithWriter-ResultsWriterErrorTimer',...
                                     'Period',this.ResultsWriterErrorTimerPeriod,...
                                     'ErrorFcn',@(~,err) disp(err),...
                                     'TimerFcn',@(~,~) this.ResultsWriterErrorTimerCallback);
             
             %% Setup the check writer timer
             try
                 delete(this.ResultsWriterCheckTimer)
             catch
             end
             this.ResultsWriterCheckTimer = timer('ObjectVisibility','off',...
                                        'BusyMode','drop',...
                                        'ExecutionMode','fixedSpacing',...
                                        'Tag','AsyncProcessorWithWriter-ResultsWriterCheckTimer',...
                                        'Period',this.ResultsWriterCheckTimerPeriod,...
                                        'ErrorFcn',@(~,err) disp(err),...
                                        'TimerFcn',@(~,~) this.ResultsWriterCheckTimerCallback);
        end
    end
    
    %% delete
    methods
        function delete(this)
            
            %% stop and delete timers
            try
            stop(this.ResultsWriterErrorTimer);
            catch
            end
            delete(this.ResultsWriterErrorTimer);
            try
            stop(this.ResultsWriterCheckTimer);
            catch
            end
            delete(this.ResultsWriterCheckTimer);
        end
    end
    
    %% 
    properties(SetObservable=true,AbortSet=true,SetAccess=private)
        isResultsFileOpen = false;
        isResultsWriterRunning = false;
        ResultsFilepath = '';
        resultsWaitingToBeWritten = 0;
    end
    
    properties(SetObservable=true,AbortSet=true)
        SaveResults = false;
    end
    
    %% Error Related Items
    events
        ResultsWriterErrorOccured %Event thrown when error has occured while writing results
    end
    
    properties(Access=private)
        ResultsWriterErrorTimerPeriod = 0.75;
        ResultsWriterErrorTimer
        
        ResultsWriterCheckTimer
        ResultsWriterCheckTimerPeriod = 0.75;
    end
    
    %% public methods
    methods
        function openResultsFile(this,fpth)
            % open a file for writing.
            % automatically adds mxf.gz extension if not present
            
            this.runMethod('openResultsFile',fpth);
            this.isResultsFileOpen = this.runMethod('isResultsFileOpen');
            this.ResultsFilepath = this.runMethod('ResultsFilepath');
            
            this.resultsWaitingToBeWritten = this.runMethod('resultsWaitingToBeWritten');
            this.isResultsWriterRunning = this.runMethod('isResultsWriterRunning');
            
            %% restart timers if needed
            this.restartResultsWriterErrorTimer();
            this.restartResultsWriterCheckTimer();
        end
        function closeResultsFile(this)
            % close file
            
            this.runMethod('closeResultsFile');
            this.isResultsFileOpen = this.runMethod('isResultsFileOpen');
        end
        
        function clearUnsavedResults(this)
            this.runMethod('clearUnsavedResults');
            this.resultsWaitingToBeWritten = this.runMethod('resultsWaitingToBeWritten');
        end
        
        function pauseResultsWriter(this)
            this.runMethod('pauseResultsWriter');
            
            this.resultsWaitingToBeWritten = this.runMethod('resultsWaitingToBeWritten');
            this.isResultsFileOpen = this.runMethod('isResultsFileOpen');
            this.isResultsWriterRunning = this.runMethod('isResultsWriterRunning');
            this.ResultsFilepath = this.runMethod('ResultsFilepath');
            
            %% stop timers
            stop(this.ResultsWriterErrorTimer);
            stop(this.ResultsWriterCheckTimer);
        end
    end
end