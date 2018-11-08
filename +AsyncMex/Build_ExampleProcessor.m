% Build AsyncMexProcessorTest

%mex -setup C++
clear mex;

%% Compiler options
compiler_options ='-std=c++14';

%% Setup Include
INCLUDE = [];

[pth,~,~] = fileparts(mfilename('fullpath'));

src = fullfile(pth,'ExampleProcessor.cpp'); %SOURCE FILE NAME

%% Include source dir in AsyncMex
%INCLUDE = [INCLUDE,' -I',fullfile(pth,'source')];

%% Include SessionManager source
if isempty(meta.package.fromName('extras.SessionManager'))
    error('extras.SessionManager could not be found, it is required to build AsyncProcessor...');
end

[SESSIONMANAGER_PATH,~] = fileparts(which('extras.SessionManager.Session'));

INCLUDE = [INCLUDE,'-I','"',fullfile(SESSIONMANAGER_PATH,'source'),'" '];

%% Output
OUTNAME = 'ExampleProcessorMex';
OUTDIR = pth;

%% BUILD
mex(['COMPFLAGS="$COMPFLAGS ' compiler_options '"'],...
    INCLUDE,...
    '-outdir',OUTDIR,...
    '-output',OUTNAME,...
    src);
