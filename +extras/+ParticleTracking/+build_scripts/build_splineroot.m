% Build splineroot

[THIS_PATH,~,~] =  fileparts(mfilename('fullpath'));
OUTNAME = 'splineroot'; %output function name
OUTDIR = fullfile(THIS_PATH,'..'); %output to .../+extras/+ParticleTracking

src = fullfile(OUTDIR,'splineroot','source','splineroot.cpp'); %SOURCE FILE NAME

%% Construct Args
ArgsStruct = extras.mex_builds.DefaultMexArgStruct();

%% BUILD
[CA,AS] = extras.mex_builds.ArgStruct2Args(ArgsStruct);

mex('-v',CA{:},...
    '-outdir',OUTDIR,...
    '-output',OUTNAME,...
    AS{:},...
    src);
