# extras.ParticleTracking
Image Processing functions for particle tracking

## Contents
* radialcenter()
  * MEX function for detecting origin of radial symmetry in an image. It is also capable of detecting symmetric origins for multiple ROI within an image
    * Implemented in .../radialcenter/source/radialcenter.hpp
    * The radialcenter code is wrapped in a function providing a standard mexFunction style interface in radialcenter_mex.hpp
    * Build using: extras.ParticleTracking.build_scripts.build_radialcenter
