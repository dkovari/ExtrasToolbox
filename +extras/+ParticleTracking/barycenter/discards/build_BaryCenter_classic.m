clear mex;

mex CXXFLAGS="\$CXXFLAGS -std=c++14" 'source/BaryCenter_classic.cpp' -outdir '+ParticleTrackers' -output 'BaryCenter_classic' -v

