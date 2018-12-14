% Build Radial Center

[pth,~,~] = fileparts(mfilename('fullpath'));

INCLUDE = ['-I',fullfile(pth,'..','include')];
OUTNAME = 'imradialavg';
compiler_options ='-std=c++14';

mex(['COMPFLAGS="$COMPFLAGS ' compiler_options '"'],...
    INCLUDE,'-outdir',fullfile(pth,'..'),'-output',OUTNAME,...
    fullfile(pth,'source','imradialavg.cpp'))