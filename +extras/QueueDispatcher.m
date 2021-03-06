classdef QueueDispatcher < handle
%% Manage and distribute data to a list of extras.Queues
%
% When queues are deleted, they are automatically removed from the list.
%
% List Management
%========================
% To add a queue to the distribution list
%   registerQueue(QD, queuesToAdd)
%       This will look through the array queuesToAdd and add queues that
%       are not already in the list.
%
% To remove queues
%   deregisterQueue(QD,queuesToRemove)
%       Look through array queuesToRemove and remove any that are present
%       in QueueList
%
% DispatchData
%==================
%   send(QD,data)
%   Sends data to the queues in the list
%       alias of send(QD.QueueList,data)

    properties (SetAccess=protected,SetObservable=true,AbortSet=true)
        QueueList = extras.Queue.empty;
    end
    properties (Access=protected,SetObservable=true,AbortSet=true)
        DeleteListeners
    end
    
    methods
        function send(this,data)
        %   send(QD,data)
        %   Sends data to the queues in the list
        %       alias of send(QD.QueueList,data)
        % will use the native Send command for each object in the queue list 

            send(this.QueueList,data);
        end
        
        function delete(this)
            delete(this.DeleteListeners)
        end
        function registerQueue(this,queues)
        %   deregisterQueue(QD,queuesToRemove)
        %       Look through array queuesToRemove and remove any that are present
        %       in QueueList
            this.QueueList = union(this.QueueList,queues);
            this.QueueList(~isvalid(this.QueueList)) = [];
            
            %% setup listeners for when queues are deleted
            delete(this.DeleteListeners);
            this.DeleteListeners = addlistener(this.QueueList,'ObjectBeingDestroyed',@(q,~) this.QueDelete(q));
            
        end
        function deregisterQueue(this,queues)
        %   registerQueue(QD, queuesToAdd)
        %       This will look through the array queuesToAdd and add queues that
        %       are not already in the list.
            this.QueueList = setdiff(this.QueueList,queues);
            this.QueueList(~isvalid(this.QueueList)) = [];
            
            %% setup listeners for when queues are deleted
            delete(this.DeleteListeners);
            this.DeleteListeners = addlistener(this.QueueList,'ObjectBeingDestroyed',@(q,~) this.QueDelete(q));
            
        end
    end
    methods (Hidden)
        function QueDelete(this,queue)
            [~,ind] = ismember(queue,this.QueueList);
            ind(ind==0)=[];
            this.QueueList(ind) = [];
        end
    end

end