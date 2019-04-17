% Script for testing PI Mercury Motor Devices

%% Port Settings
COM_PORT = 'COM7';
BOARD_ID = 0;
DEV_TYPE = 'M-126.PD2';

%% Create Device
try
    delete(MD)
    delete(MHUB);
catch
end

MHUB = extras.hardware.PI.MercuryHub.create();
COMSEL = extras.hardware.ComSelectorUI(MHUB);



MD = extras.hardware.PI.MercuryDevice(MHUB,BOARD_ID,DEV_TYPE);
SL = extras.hardware.TargetValueDeviceUI(MD);
del_list = addlistener(SL,'ObjectBeingDestroyed',@(~,~) delete(MD));