classdef HasDeviceName < handle
    properties(SetAccess=protected,AbortSet=true,SetObservable=true)
        DeviceName = 'SerialDevice'; %User readable device name, can be changed by derived classes.
    end
end