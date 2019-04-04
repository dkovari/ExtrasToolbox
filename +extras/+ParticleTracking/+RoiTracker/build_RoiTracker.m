% Build Radial Center

[THIS_PATH,~,~] =  fileparts(mfilename('fullpath'));
OUTNAME = 'RoiTracker_mex'; %output function name
OUTDIR = THIS_PATH;%fullfile(THIS_PATH,'..'); %output to .../+extras/+ParticleTracking

src = fullfile(THIS_PATH,'source','RoiTracker.cpp'); %SOURCE FILE NAME

%% Construct Args
ArgsStruct = extras.mex_builds.MexArgsStruct_zlib();

%% Add particle tracking headers
ArgsStruct.Include = [ArgsStruct.Include,...
    {['-I',fullfile(extras.ToolboxPath,'+ParticleTracking')]}];

%% BUILD
[CA,AS] = extras.mex_builds.ArgStruct2Args(ArgsStruct);

mex('-v',CA{:},...
    '-outdir',OUTDIR,...
    '-output',OUTNAME,...
    AS{:},...
    src);
