% Build ExampleProcessor

[THIS_PATH,~,~] =  fileparts(mfilename('fullpath'));
OUTNAME = 'ExampleProcessor_PersistArgsMex'; %output function name
OUTDIR = THIS_PATH;%fullfile(THIS_PATH,'..'); %output to .../+extras/+ParticleTracking

src = fullfile(THIS_PATH,'ExampleProcessor_PersistArgs.cpp'); %SOURCE FILE NAME

%% Construct Args
ArgsStruct = extras.mex_builds.MexArgsStruct_zlib();

%% BUILD
[CA,AS] = extras.mex_builds.ArgStruct2Args(ArgsStruct);

mex('-v',CA{:},...
    '-outdir',OUTDIR,...
    '-output',OUTNAME,...
    AS{:},...
    src);