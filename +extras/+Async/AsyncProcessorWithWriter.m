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
%% Copyright 2019 Daniel T. Kovari, Emory University
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
            
            %% prompt to cancel tasks (called here so that user is prompted to let tasks process before being asked if data should be written
            this.uiPromptToClearTasks(); %super-class method
            
            %% prompt to cancel writing
            this.uiPromptToCancelWriting();
            
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
    
    %% protected methods
    methods (Access=protected)
        function uiPromptToCancelWriting(this)
            %% Check if there are results waiting
            this.resultsWaitingToBeWritten = this.runMethod('resultsWaitingToBeWritten');
            this.isResultsFileOpen = this.runMethod('isResultsFileOpen');
            
            if this.isResultsFileOpen
                addlistener(this,'ResultsWriterErrorOccured',@(~,err) warndlg(err.message,'Error Writing Results'));
                this.resumeResultsWriter();
                pause(0.75); %wait a little bit to allow any remaining tasks to start processing
                
                %% try to write tasks
                hWB = [];
                nRem = this.runMethod('resultsWaitingToBeWritten');
                last_comp = 0;
                while this.runMethod('resultsWaitingToBeWritten')>0 || this.RemainingTasks>0

                    RT = this.runMethod('resultsWaitingToBeWritten');
                    comp = nRem-RT;
                    if comp<0 %more tasks were added
                        comp = last_comp;
                        nRem = RT;
                    end

                    if isempty(hWB)
                        hWB = waitbar(comp/nRem,sprintf('%s: Writing Results (%d/%d)\nPress Cancel to skip remaining.',this.Name,comp,nRem),'CreateCancelBtn',@(~,~) this.runMethod('stopWritingAndClearUnsaved'));
                    elseif ishghandle(hWB)
                        waitbar(comp/nRem,hWB,sprintf('%s: Writing Results (%d/%d)\nPress Cancel to skip remaining.',this.Name,comp,nRem));
                    end
                    pause(0.5);
                    
                    if ~this.isResultsWriterRunning
                        warning('Results Writer has stopped writing data');
                        break;
                    end
                end
                try
                    delete(hWB)
                catch
                end
                
            elseif this.resultsWaitingToBeWritten
                warndlg('There are results waiting to be written, but the file is not open. They will be discarded.','Results Waiting','modal');
            end
            
            
            %% cancel remaining write events
            this.clearUnsavedResults()
        end
    end
    
    %% 
    properties(SetObservable=true,AbortSet=true,SetAccess=private)
        isResultsFileOpen = false;
        isResultsWriterRunning = false;
        ResultsFilepath = '';
        resultsWaitingToBeWritten = 0;
    end
    
    %% SaveResults
    properties(SetObservable=true,AbortSet=true)
        SaveResults (1,1) logical = false;
    end
    properties(Access=private)
        internal_SetSaveResults = false;
    end
    methods
        function set.SaveResults(this,val)
            if ~this.internal_SetSaveResults
                this.runMethod('SaveResults',val);
                this.SaveResults = this.runMethod('SaveResults');
            else
                this.SaveResults = val;
            end
            
        end
    end
        
    
    %% Error Related Items
    events
        ResultsWriterErrorOccured %Event thrown when error has occured while writing results
    end
    
    properties(Access=private)
        ResultsWriterErrorTimerPeriod = 0.75;
        ResultsWriterErrorTimer %timer used to check for error while writing results
        
        ResultsWriterCheckTimer %tiemr used to check results writer progress
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
        
        function resumeResultsWriter(this)
            this.isResultsFileOpen = this.runMethod('isResultsFileOpen');
            this.resultsWaitingToBeWritten = this.runMethod('resultsWaitingToBeWritten');
            this.ResultsFilepath = this.runMethod('ResultsFilepath');
            
            if ~this.isResultsFileOpen
                error('Cannot resumeResultsWriter because the file is not open');
            end
            
            this.runMethod('resumeResultsWriter');
            this.isResultsWriterRunning = this.runMethod('isResultsWriterRunning');
            
            %% restart timers if needed
            this.restartResultsWriterErrorTimer();
            this.restartResultsWriterCheckTimer();
        end
        
    end
    
    
    %% Callbacks
    methods(Access=private)
        
        function DispatchWriterErrorMessage(this)
        % internal use - check and dispatch error messages        
            errMsg = this.runMethod('getResultsWriterError');
            if ~isempty(errMsg)
                this.LastErrorMessage = errMsg;
                notify(this,'ResultsWriterErrorOccured',extras.Async.AsyncProcessorError(errMsg,func2str(this.MEX_function)));
            end
            this.clearError();
        end
        
        function ResultsWriterErrorTimerCallback(this)
            if ~isvalid(this)
                return;
            end
            
            if this.runMethod('wasResultsWriterErrorThrown')
                this.DispatchWriterErrorMessage();
            end
            
            this.isResultsWriterRunning = this.runMethod('isResultsWriterRunning');
            if ~this.isResultsWriterRunning
                stop(this.ResultsWriterErrorTimer);
            end
        end
        
        function ResultsWriterCheckTimerCallback(this)
            if ~isvalid(this)
                return;
            end
            
            %% check SaveResults
            this.internal_SetSaveResults = true;
            this.SaveResults = this.runMethod('SaveResults');
            this.internal_SetSaveResults = false;
            
            %% Check other properties
            this.isResultsFileOpen = this.runMethod('isResultsFileOpen');
            this.resultsWaitingToBeWritten = this.runMethod('resultsWaitingToBeWritten');
            this.isResultsWriterRunning = this.runMethod('isResultsWriterRunning');
            
            if ~this.isResultsWriterRunning
                stop(this.ResultsWriterCheckTimer);
            end
        end
        
        function restartResultsWriterErrorTimer(this)
            if ~isvalid(this)
                return;
            end
            
            try
                start(this.ResultsWriterErrorTimer);
            catch
            end
        end
        
        function restartResultsWriterCheckTimer(this)
            if ~isvalid(this)
                return;
            end
            
            this.isResultsFileOpen = this.runMethod('isResultsFileOpen');
            this.isResultsWriterRunning = this.runMethod('isResultsWriterRunning');
            this.resultsWaitingToBeWritten = this.runMethod('resultsWaitingToBeWritten');
            
            if this.isResultsFileOpen && this.isResultsWriterRunning
                try
                    start(this.ResultsWriterCheckTimer);
                catch
                end
            else
                warning('Cannot start ResultsWriterCheckTimer because file is not open or writing is not enabled');
            end
            
        end
    end
end