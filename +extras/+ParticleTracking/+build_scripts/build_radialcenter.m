% Build Radial Center

[THIS_PATH,~,~] =  fileparts(mfilename('fullpath'));
OUTNAME = 'radialcenter'; %output function name
OUTDIR = fullfile(THIS_PATH,'..'); %output to .../+extras/+ParticleTracking

src = fullfile(OUTDIR,'radialcenter','source','radialcenter.cpp'); %SOURCE FILE NAME

%% Construct Args
ArgsStruct = extras.mex_builds.DefaultMexArgStruct();

%% BUILD
[CA,AS] = extras.mex_builds.ArgStruct2Args(ArgsStruct);

mex('-v',CA{:},...
    '-outdir',OUTDIR,...
    '-output',OUTNAME,...
    AS{:},...
    src);

