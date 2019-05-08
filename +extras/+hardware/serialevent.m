classdef (ConstructOnLoad) serialevent < event.EventData
% Event object passed by extras.SerialDevice when data is recieved
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.       
    properties
        SerialObject
        SerialEvent
    end
    
    methods
        function this = serialevent(SerialObject,SerialEvent)
            this.SerialObject = SerialObject;
            this.SerialEvent = SerialEvent;
        end
    end
end