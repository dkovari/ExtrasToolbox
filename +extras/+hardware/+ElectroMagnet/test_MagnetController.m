% Test Electro magnet controller

%% Setup
MagCon = extras.hardware.ElectroMagnet.MagnetController;
TurnMon = extras.hardware.ElectroMagnet.TurnMonitor(MagCon);
TurnCon = extras.hardware.ElectroMagnet.TurnController(MagCon,TurnMon);
TurnConUI = extras.hardware.ElectroMagnet.TurnControlUI(TurnCon);