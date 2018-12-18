% Build Radial Center
clear mex;

[THIS_PATH,~,~] =  fileparts(mfilename('fullpath'));
OUTNAME = 'radialcenterAsync'; %output function name
OUTDIR = fullfile(THIS_PATH,'..'); %output to .../+extras/+ParticleTracking


%% Compiler options
compiler_options ='-std=c++14';

%% Setup Source Path
[pth,~,~] = fileparts(mfilename('fullpath'));
src = fullfile(pth,'..','radialcenter','radialcenterAsync.cpp'); %SOURCE FILE NAME

%% Setup Include
[THIS_PATH,~,~] =  fileparts(mfilename('fullpath'));

PATH_TO_EXTRASINCLUDE = fullfile(THIS_PATH,'..','..','include'); %include .../+extras/include

INCLUDE = ['-I',PATH_TO_EXTRASINCLUDE];

%% BUILD
mex(['COMPFLAGS="$COMPFLAGS ' compiler_options '"'],...
    INCLUDE,...
    '-outdir',OUTDIR,...
    '-output',OUTNAME,...
    src);
