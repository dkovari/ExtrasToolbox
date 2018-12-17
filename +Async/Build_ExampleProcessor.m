% Build ExampleProcessor
clear mex;
clear all;

[THIS_PATH,~,~] =  fileparts(mfilename('fullpath'));
OUTNAME = 'ExampleProcessorMex'; %output function name
OUTDIR = THIS_PATH; %output to .../+extras/+ParticleTracking


%% Compiler options
compiler_options ='-std=c++14';

%% Setup Source Path
[pth,~,~] = fileparts(mfilename('fullpath'));
src = fullfile(pth,'ExampleProcessor.cpp'); %SOURCE FILE NAME

%% Setup Include
[THIS_PATH,~,~] =  fileparts(mfilename('fullpath'));
PATH_TO_EXTRASINCLUDE = fullfile(THIS_PATH,'..','include'); %include .../+extras/include
INCLUDE = ['-I',PATH_TO_EXTRASINCLUDE];

%% BUILD
mex(['COMPFLAGS="$COMPFLAGS ' compiler_options '"'],...
    INCLUDE,...
    '-outdir',OUTDIR,...
    '-output',OUTNAME,...
    src);
