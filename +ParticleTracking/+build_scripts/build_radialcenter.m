% Build Radial Center
clear mex;

[THIS_PATH,~,~] =  fileparts(mfilename('fullpath'));
OUTNAME = 'radialcenter'; %output function name
OUTDIR = fullfile(THIS_PATH,'..'); %output to .../+extras/+ParticleTracking


%% Compiler options
compiler_options ='-std=c++14';

%% Setup Source Path
[pth,~,~] = fileparts(mfilename('fullpath'));
src = fullfile(pth,'..','radialcenter','source','radialcenter.cpp'); %SOURCE FILE NAME

%% Setup Include
INCLUDE = ['-I',extras.IncludePath()]; %include .../+extras/include

%% BUILD
mex(['COMPFLAGS="$COMPFLAGS ' compiler_options '"'],...
    INCLUDE,...
    '-outdir',OUTDIR,...
    '-output',OUTNAME,...
    src);
