% Build barycenter

[THIS_PATH,~,~] =  fileparts(mfilename('fullpath'));
OUTNAME = 'barycenter'; %output function name
OUTDIR = fullfile(THIS_PATH,'..'); %output to .../+extras/+ParticleTracking

src = fullfile(OUTDIR,'barycenter','source','barycenter.cpp'); %SOURCE FILE NAME

%% Construct Args
ArgsStruct = extras.mex_builds.DefaultMexArgStruct();

%% BUILD
[CA,AS] = extras.mex_builds.ArgStruct2Args(ArgsStruct);

mex('-v',CA{:},...
    '-outdir',OUTDIR,...
    '-output',OUTNAME,...
    AS{:},...
    src);