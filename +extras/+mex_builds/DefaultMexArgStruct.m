function build_args = DefaultMexArgStruct(build_args)
% returns struct with necessary include and linker options
%
% Usage:
%   Call
%       BA = extras.mex_builds.DefaultMexArgStruct()
%   to generate arguments
% When ready to build you can build using the arguments:
% mex(['COMPFLAGS="$COMPFLAGS ' BA.CompilerOptions{:} '"'],...
%     BA.Include{:},...
%     BA.LibDir{:},...
%     BA.Lib{:},...
%     '-outdir',YOUR_OUTDIR,...
%     '-output',YOUR_OUTPUT_NAME,...
%     BA.AuxSource{:},...
%     YOUR_SOURCE_FILE);
% You can also include previous arguments, and the function will append the
% new arguments

%% create build_args if needed
if nargin<1
    build_args = struct();
end
if ~isfield(build_args,'CompilerOptions')
    build_args.CompilerOptions = {};
end
if ~isfield(build_args,'Include')
    build_args.Include = {};
end
if ~isfield(build_args,'Lib')
    build_args.Lib = {};
end
if ~isfield(build_args,'LibDir')
    build_args.LibDir = {};
end
if ~isfield(build_args,'AuxSource')
    build_args.AuxSource = {};
end
if ~isfield(build_args,'OptimizationFlags')
    build_args.OptimizationFlags = {};
end

%% Compiler options
if ispc
compiler_options ={'/std:c++17'};
optimization_flags={'/DNDEBUG /O2 /Oy /GL'};
else
compiler_options ={'-std=c++17'};
optimization_flags = {'-DNDEBUG -O2 -Oy -GL'};
end

%% Setup Include
INCLUDE = {['-I',extras.IncludePath]};%


%% LIB
LIBS = {}; %c and c++ libtiff libraries
LIB_DIR = {};

%% Add StackWalker Dependencies
if ispc
    if ~exist(fullfile(extras.IncludePath,'StackWalker','x64','Release','StackWalker.lib'),'file')
        %error('StackWalker.lib does not exist. You may need to build it.');
        build_args.AuxSource = [build_args.AuxSource,fullfile(extras.IncludePath,'StackWalker','StackWalker.cpp')];
    else
        LIB_DIR = [LIB_DIR,['-L',fullfile(extras.IncludePath,'StackWalker','x64','Release')]];
        LIBS = [LIBS,'StackWalker.lib'];
    end
end

%% set output struct
build_args.CompilerOptions = [build_args.CompilerOptions,compiler_options];
build_args.OptimizationFlags = [build_args.OptimizationFlags,optimization_flags];
build_args.Include = [build_args.Include,INCLUDE];
build_args.Lib = [build_args.Lib,LIBS];
build_args.LibDir = [build_args.LibDir,LIB_DIR];
