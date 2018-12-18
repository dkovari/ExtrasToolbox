classdef AsyncProcessorError < event.EventData
% Error Event generated by AsyncProcessor when a task in the queue thros an error

    properties
        identifier
        message
    end
    methods
        function this = AsyncProcessorError(errMsg)
            this.identifier = errMsg.identifier;
            this.message = errMsg.message;
        end
    end

end
