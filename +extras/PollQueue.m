classdef PollQueue < extras.Queue
%% A pollable Queue class, similar to parallel.pool.PollableDataQueue
%
% This class is derived from extras.Queue and implements the sendData
% functionality, which allows arbitraty data to be added to a queue.
%
% When data is added, a notification is broadcast to listeners of NewData.
% Data can be retrieved from the queue using:
%   popFront()
%   popBack()
%   popAll()
%
% The queue can be cleared using:
%   Clear()
%
% Info about the queue:
%   Length(): Length of the queue
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.   

    properties(SetAccess=protected)
        Data;
    end
    
    events
        NewData
    end
    
    methods(Access=protected)
        function internalSend(this,D)
            this.Data = cat(1,this.Data,{D});
            
            notify(this,'NewData');
        end
    end
    
    methods
        function this = PollQueue()
            this.Data = {};
        end

        function L = Length(this)
            L = numel(this.Data);
        end
        
        function Clear(this)
            this.Data = cell(0,1);
        end
        
        function D = popFront(this)
            if isempty(this.Data)
                D = {};
                return
            end
            
            D = this.Data{end};
            this.Data{end}=[];
        end
        
        function D = popBack(this)
            if isempty(this.Data)
                D = {};
                return
            end
            
            D = this.Data{1};
            this.Data{1}=[];
        end
        
        function Data = popAll(this)
            if isempty(this.Data)
                Data = {};
                return;
            end
            Data = this.Data;
            this.Data = {};
        end
        
        
    end
end