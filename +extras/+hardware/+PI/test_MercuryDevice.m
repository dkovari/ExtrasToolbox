% Script for testing PI Mercury Motor Devices

%% Port Settings
COM_PORT = 'COM7';
BOARD_ID = 0;
DEV_TYPE = 'M-126.PD2';

%% Create Device
try
    delete(MD)
catch
end

MD = extras.hardware.PI.MercuryDevice(COM_PORT,BOARD_ID,DEV_TYPE);
hub = MD.Hub;
SL = extras.hardware.TargetValueDeviceUI(MD);
del_list = addlistener(SL,'ObjectBeingDestroyed',@(~,~) delete(MD));