% Build AsyncMexProcessorTest

%mex -setup C++
clear mex;

[pth,~,~] = fileparts(mfilename('fullpath'));

src = fullfile(pth,'source','AsyncProcessorTest.cpp');

mex('CXXFLAGS="\$CXXFLAGS -std=c++14"','-outdir',pth,src)