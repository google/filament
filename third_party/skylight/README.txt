This file is part of a sample implementation of the analytical skylight and
solar radiance models presented in the SIGGRAPH 2012 paper


           "An Analytic Model for Full Spectral Sky-Dome Radiance"

and the 2013 IEEE CG&A paper

       "Adding a Solar Radiance Function to the Hosek Skylight Model"

                                   both by 

                       Lukas Hosek and Alexander Wilkie
                Charles University in Prague, Czech Republic


                        Version: 1.4a, February 22nd, 2013
                        
Version history:

1.4a  February 22nd, 2013
      Removed unnecessary and counter-intuitive solar radius parameters 
      from the interface of the colourspace sky dome initialisation functions.

1.4   February 11th, 2013
      Fixed a bug which caused the relative brightness of the solar disc
      and the sky dome to be off by a factor of about 6. The sun was too 
      bright: this affected both normal and alien sun scenarios. The 
      coefficients of the solar radiance function were changed to fix this.

1.3   January 21st, 2013 (not released to the public)
      Added support for solar discs that are not exactly the same size as
      the terrestrial sun. Also added support for suns with a different
      emission spectrum ("Alien World" functionality).

1.2a  December 18th, 2012
      Fixed a mistake and some inaccuracies in the solar radiance function
      explanations found in ArHosekSkyModel.h. The actual source code is
      unchanged compared to version 1.2.

1.2   December 17th, 2012
      Native RGB data and a solar radiance function that matches the turbidity
      conditions were added.

1.1   September 2012
      The coefficients of the spectral model are now scaled so that the output
      is given in physical units: W / (m^-2 * sr * nm). Also, the output of the
      XYZ model is now no longer scaled to the range [0...1]. Instead, it is
      the result of a simple conversion from spectral data via the CIE 2 degree
      standard observer matching functions. Therefore, after multiplication
      with 683 lm / W, the Y channel now corresponds to luminance in lm.
     
1.0   May 11th, 2012
      Initial release.


Please visit http://cgg.mff.cuni.cz/projects/SkylightModelling/ to check if
an updated version of this code has been published!

This archive contains the following files:

README.txt                      This file.

ArHosekSkyModel.h               Header file for the reference functions. Their
                                usage is explained there, and sample code for 
                                calling them is given.

ArHosekSkyModel.c               Implementation of the functions.

ArHosekSkyModelData_Spectral.h  Spectral coefficient data.
ArHosekSkyModelData_CIEXYZ.h    CIE XYZ coefficient data.
ArHosekSkyModelData_RGB.h       RGB coefficient data.

Please note that the source files are in C99, and you have to set appropriate 
compiler flags for them to work. For example, when compiling this code with 
gcc, you have to add the "-std=c99" or "-std=gnu99" flags.