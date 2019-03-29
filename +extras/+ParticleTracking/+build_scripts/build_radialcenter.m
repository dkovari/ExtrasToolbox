% Build Radial Center
clear mex;

[THIS_PATH,~,~] =  fileparts(mfilename('fullpath'));
OUTNAME = 'radialcenter'; %output function name
OUTDIR = fullfile(THIS_PATH,'..'); %output to .../+extras/+ParticleTracking


%% Compiler options
if ispc
compiler_options ='/std:c++17';
optimization_flags='/DNDEBUG /O2 /Oy /GL';
else
compiler_options ='-std=c++17';
optimization_flags = '$OPTIMFLAGS';
end

%% Setup Source Path
[pth,~,~] = fileparts(mfilename('fullpath'));
src = fullfile(pth,'..','radialcenter','source','radialcenter.cpp'); %SOURCE FILE NAME

%% Setup Include
INCLUDE = ['-I',extras.IncludePath()]; %include .../+extras/include

%% BUILD
mex('-v',['COMPFLAGS="$COMPFLAGS ' compiler_options '"'],...
    ['OPTIMFLAGS="',optimization_flags,'"'],...
    INCLUDE,...
    '-outdir',OUTDIR,...
    '-output',OUTNAME,...
    src);
