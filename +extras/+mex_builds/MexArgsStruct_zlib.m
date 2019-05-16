function [build_args] = MexArgsStruct_zlib(build_args)
% Add includes and libs to MexArgStruct to support zlib

if nargin<1
    build_args = extras.mex_builds.DefaultMexArgStruct();
else
    build_args = extras.mex_builds.DefaultMexArgStruct(build_args);
end

if ispc %use complied zlib
    
    extern_libs_dir = fullfile(extras.ToolboxPath,'external_libs','zlib');
    
    INCLUDE = {['-I',fullfile(extern_libs_dir,'include')]};
    LIBS = {'-lzlibstat'};
    LIB_DIR = {['-L',fullfile(extern_libs_dir,'lib','Win','x64')]};
else % posix try to use system zlib
    error('DAN YOU NEED TO IMPLEMENT MexArgsStruct_zlib on POSIX');
end


%% set output
build_args.Include = [build_args.Include,INCLUDE];
build_args.Lib = [build_args.Lib,LIBS];
build_args.LibDir = [build_args.LibDir,LIB_DIR];

