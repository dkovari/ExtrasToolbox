classdef Queue <handle & matlab.mixin.Heterogeneous
%% (Abstract) A simple que, similar to parallel.pool.DataQueue and parallel.pool.Queue
%
% This is an abstract class.
% To create a queue use extras.CallbackQueue or extras.PollQueue
%
% This is the parent of both of those classes, and is derived from
% mixins.Heterogeneous so that lists of PollQueues and CallbackQueues can
% be mixed together in a single array.
%
% This class implements the send(queue,data) method, which allows a user to
% distribute arbitrary data (any type/structure) to be added to a queue
%
% Derived classes must implement the internalSend Method, which actually
% handles distributing the data.

    methods (Static, Sealed, Access = protected)
       function default_object = getDefaultScalarElement
           default_object = extras.PollQueue;
       end
    end
    methods (Abstract, Access=protected)
      % Not implemented by Queue Class
      internalSend(obj,data)
    end
    
    properties(SetObservable,AbortSet)
        Suspended (1,1) logical = false; %Flag indicating that Queue is suspended and therefore ignoring data
    end
    
    methods (Sealed)
        function send(queueArray,data)
        %Send data to the queues
        
            for n=1:numel(queueArray)
                if isvalid(queueArray(n)) && ~queueArray(n).Suspended
                    queueArray(n).internalSend(data);
                end
            end
        end
    end
    
    methods (Sealed)
        function tf = eq(A,B)
            tf = eq@handle(A,B);
        end
        function tf = ne(A,B)
            tf = ne@handle(A,B);
        end
    end
    
end