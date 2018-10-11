classdef CallbackQueue < extras.Queue
%% A Queue class, similar to parallel.pool.DataQueue
%
% This class is derived from extras.Queue and implements the sendData
% functionality, which allows arbitraty data to be added to a queue.
%
% When data is added every function registered with afterEach is called,
% with the most recent data supplied as the argument.
% Data in the queue cannot be handled directly.
%
% Functions:
%=============================
%   afterEach(CQ,fcn)
%       registeres the function_handle to be executed when
%       data is added to the queue.

    properties(Access=protected)
        Callbacks = {}
    end
    
    methods
        function afterEach(this,fcn)
            assert(isa(fcn,'function_handle'),'fcn must be a function handle');
            this.Callbacks = cat(1,this.Callbacks,{fcn});
        end
        function clearCallbacks(this)
            this.Callbacks = {};
        end
    end
    methods (Access=protected)
        function internalSend(this,D)
            for n=1:numel(this.Callbacks)
                this.Callbacks{n}(D);
            end
        end
    end
end