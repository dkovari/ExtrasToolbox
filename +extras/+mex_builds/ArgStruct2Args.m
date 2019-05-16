function [args_cell,AuxSource] = ArgStruct2Args(build_struct)
% convert extras.mex_builds.DefaultMexArgStruct type output into cell array
% which can be directly passed to mex:
%
% Usage:
%   >> ba = extras.mex_builds.DefaultMexArgStruct(); %create arg struct
%   >> [arg_cell,AuxSource] = ArgStruct2Args(ba); %convert to cell
%   >> mex(arg_cell{:},...,AuxSource{:},YOUR_SOURCE_FILE); %call mex

args_cell = {...
    ['COMPFLAGS="$COMPFLAGS ' build_struct.CompilerOptions{:} '"'],...
    ['OPTIMFLAGS="',build_struct.OptimizationFlags{:},'"'],...
    build_struct.Include{:},...
    build_struct.LibDir{:},...
    build_struct.Lib{:}};

AuxSource = build_struct.AuxSource;