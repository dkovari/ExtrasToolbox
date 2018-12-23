clear mex;

%% mean3x3
mex CXXFLAGS="\$CXXFLAGS -std=c++14" 'source/barycenter.cpp' -outdir '+ParticleTrackers' -output 'BaryCenter' -v

%% mean3x3
mex CXXFLAGS="\$CXXFLAGS -std=c++14" 'source/barycenter.cpp' -outdir '+ParticleTrackers' -output 'BaryCenter_m3x3' -v -DGI_M3X3


%% mean5x5
mex CXXFLAGS="\$CXXFLAGS -std=c++14" 'source/barycenter.cpp' -outdir '+ParticleTrackers' -output 'BaryCenter_m5x5' -v -DGI_M5X5
%% parab5x5
mex CXXFLAGS="\$CXXFLAGS -std=c++14" 'source/barycenter.cpp' -outdir '+ParticleTrackers' -output 'BaryCenter_p5x5' -v -DGI_P5X5
