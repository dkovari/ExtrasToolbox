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
%% Copyright 2018-2019 Daniel T. Kovari -----------------------------------
% Permission is hereby granted, free of charge, to any person obtaining
% a copy of this software and associated documentation files (the
% "Software"), to deal in the Software without restriction, including
% without limitation the rights to use, copy, modify, merge, publish,
% distribute, sublicense, and/or sell copies of the Software, and to
% permit persons to whom the Software is furnished to do so, subject to
% the following conditions:
% 
% The above copyright notice and this permission notice shall be
% included in all copies or substantial portions of the Software.
% 
% THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
% EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
% MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
% IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
% CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
% TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
% SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
%--------------------------------------------------------------------------

    %%
    properties(SetAccess=protected)
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
        function clearCallbacks(this,indicies)
        % afterEach Callbacks
        % Optional Inputs:
        %   indicies: nummeric array specifying which elements to remove
        %       if indicies is not specified, all callbacks are removed
        
            if nargin<2
                this.Callbacks = {};
                return;
            end
            
            mustBeInteger(indicies)
            mustBePositive(indicies);
            
            this.Callback(indicies) = [];
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