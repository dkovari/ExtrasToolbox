% Build Radial Center

[THIS_PATH,~,~] =  fileparts(mfilename('fullpath'));
OUTNAME = 'radialcenter'; %output function name
OUTDIR = fullfile(THIS_PATH,'..');


%% Compiler options
compiler_options ='-std=c++14';

%% Setup Source Path
[pth,~,~] = fileparts(mfilename('fullpath'));
src = fullfile(pth,'..','radialcenter','source','mexFunction_radialcenter.cpp'); %SOURCE FILE NAME

%% Setup Include
[THIS_PATH,~,~] =  fileparts(mfilename('fullpath'));
%PATH_TO_MEXEXTRAS = fullfile(THIS_PATH,'..','..','mexextras');
PATH_TO_MEXEXTRAS = fullfile(THIS_PATH,'..','OUTDATED_include');


INCLUDE = ['-I',PATH_TO_MEXEXTRAS];

%% BUILD
mex(['COMPFLAGS="$COMPFLAGS ' compiler_options '"'],...
    INCLUDE,...
    '-outdir',OUTDIR,...
    '-output',OUTNAME,...
    src);