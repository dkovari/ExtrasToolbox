% build MxFileWriter

[THIS_PATH,~,~] =  fileparts(mfilename('fullpath'));
OUTNAME = 'MxFileWriter_mex'; %output function name
OUTDIR = fullfile(THIS_PATH,'..'); %output to .../+extras/+mxfile

src = fullfile(THIS_PATH,'source','MxFileWriter.cpp'); %SOURCE FILE NAME

%% Construct Args
ArgsStruct = extras.mex_builds.MexArgsStruct_zlib();

%% BUILD
[CA,AS] = extras.mex_builds.ArgStruct2Args(ArgsStruct);

mex('-v',CA{:},...
    '-outdir',OUTDIR,...
    '-output',OUTNAME,...
    AS{:},...
    src);