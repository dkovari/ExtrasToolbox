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
%% Copyright 2018-2019 Daniel T. Kovari, Emory University
% All rights reserved.

    properties(Access=protected)
        Callbacks = {};% cell array containing function handles executed when obj.send(data) is called
    end
    
    %% Public
    methods
        function afterEach(this,fcn)
            %Add Callback function which is executed after new data is sent
            %to the queue
            % Input:
            %   fcn: valid function handle accepting one input
            assert(isa(fcn,'function_handle'),'fcn must be a function handle');
            this.Callbacks = cat(1,this.Callbacks,{fcn});
        end
        function clearCallbacks(this)
            % Remove all afterEach Callbacks
            
            this.Callbacks = {};
        end
    end
    
    
    %% Specialized internalSend
    methods (Access=protected)
        function internalSend(this,D)
            % Overloaded internalSend Method, responsible for executing
            % callbacks. Method is called by extras.Queue.Send(...)
            for n=1:numel(this.Callbacks)
                this.Callbacks{n}(D);
            end
        end
    end
end