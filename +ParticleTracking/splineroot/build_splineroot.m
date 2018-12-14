% Build Radial Center

[pth,~,~] = fileparts(mfilename('fullpath'));

INCLUDE = ['-I',fullfile(pth,'..','include')];
OUTNAME = 'splineroot';
compiler_options ='-std=c++14';

mex(['COMPFLAGS="$COMPFLAGS ' compiler_options '"'],INCLUDE,'-outdir',fullfile(pth,'..'),'-output',OUTNAME,fullfile(pth,'source','splineroot.cpp'))