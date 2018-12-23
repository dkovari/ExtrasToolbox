clear mex;
mex CXXFLAGS="\$CXXFLAGS -std=c++14" 'source/barycenter.cpp' -outdir '+ParticleTrackers' -output 'BaryCenter_DEBUG' -DDEBUG_PLOT -v