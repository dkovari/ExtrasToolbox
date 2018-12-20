% Build ExampleProcessor

[THIS_PATH,~,~] =  fileparts(mfilename('fullpath'));
OUTNAME = 'ExampleProcessor_PersistArgsMex'; %output function name
OUTDIR = THIS_PATH; %output to .../+extras/+ParticleTracking


%% Compiler options
if ispc
compiler_options ='/std:c++17';
else
compiler_options ='-std=c++17';
end

%% Setup Source Path
[pth,~,~] = fileparts(mfilename('fullpath'));
src = fullfile(pth,'ExampleProcessor_PersistArgs.cpp'); %SOURCE FILE NAME

%% Setup Include
INCLUDE = ['-I',extras.IncludePath];

%% BUILD
mex(['COMPFLAGS="$COMPFLAGS ' compiler_options '"'],...
    INCLUDE,...
    '-outdir',OUTDIR,...
    '-output',OUTNAME,...
    src);
