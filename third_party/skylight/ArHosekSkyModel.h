/*
This source is published under the following 3-clause BSD license.

Copyright (c) 2012 - 2013, Lukas Hosek and Alexander Wilkie
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * None of the names of the contributors may be used to endorse or promote 
      products derived from this software without specific prior written 
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/* ============================================================================

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

============================================================================ */


/*

This code is taken from ART, a rendering research system written in a
mix of C99 / Objective C. Since ART is not a small system and is intended to 
be inter-operable with other libraries, and since C does not have namespaces, 
the structures and functions in ART all have to have somewhat wordy 
canonical names that begin with Ar.../ar..., like those seen in this example.

Usage information:
==================


Model initialisation
--------------------

A separate ArHosekSkyModelState has to be maintained for each spectral
band you want to use the model for. So in a renderer with 'num_channels'
bands, you would need something like

    ArHosekSkyModelState  * skymodel_state[num_channels];

You then have to allocate and initialise these states. In the following code
snippet, we assume that 'albedo' is defined as

    double  albedo[num_channels];

with a ground albedo value between [0,1] for each channel. The solar elevation  
is given in radians.

    for ( unsigned int i = 0; i < num_channels; i++ )
        skymodel_state[i] =
            arhosekskymodelstate_alloc_init(
                  turbidity,
                  albedo[i],
                  solarElevation
                );

Note that starting with version 1.3, there is also a second initialisation 
function which generates skydome states for different solar emission spectra 
and solar radii: 'arhosekskymodelstate_alienworld_alloc_init()'.

See the notes about the "Alien World" functionality provided further down for a 
discussion of the usefulness and limits of that second initalisation function.
Sky model states that have been initialised with either function behave in a
completely identical fashion during use and cleanup.

Using the model to generate skydome samples
-------------------------------------------

Generating a skydome radiance spectrum "skydome_result" for a given location
on the skydome determined via the angles theta and gamma works as follows:

    double  skydome_result[num_channels];

    for ( unsigned int i = 0; i < num_channels; i++ )
        skydome_result[i] =
            arhosekskymodel_radiance(
                skymodel_state[i],
                theta,
                gamma,
                channel_center[i]
              );
              
The variable "channel_center" is assumed to hold the channel center wavelengths
for each of the num_channels samples of the spectrum we are building.


Cleanup after use
-----------------

After rendering is complete, the content of the sky model states should be
disposed of via

        for ( unsigned int i = 0; i < num_channels; i++ )
            arhosekskymodelstate_free( skymodel_state[i] );


CIE XYZ Version of the Model
----------------------------

Usage of the CIE XYZ version of the model is exactly the same, except that
num_channels is of course always 3, and that ArHosekTristimSkyModelState and
arhosek_tristim_skymodel_radiance() have to be used instead of their spectral
counterparts.

RGB Version of the Model
------------------------

The RGB version uses sRGB primaries with a linear gamma ramp. The same set of
functions as with the XYZ data is used, except the model is initialized
by calling arhosek_rgb_skymodelstate_alloc_init.

Solar Radiance Function
-----------------------

For each position on the solar disc, this function returns the entire radiance 
one sees - direct emission, as well as in-scattered light in the area of the 
solar disc. The latter is important for low solar elevations - nice images of 
the setting sun would not be possible without this. This is also the reason why 
this function, just like the regular sky dome model evaluation function, needs 
access to the sky dome data structures, as these provide information on 
in-scattered radiance.

CAVEAT #1: in this release, this function is only provided in spectral form!
           RGB/XYZ versions to follow at a later date.

CAVEAT #2: (fixed from release 1.3 onwards) 

CAVEAT #3: limb darkening renders the brightness of the solar disc
           inhomogeneous even for high solar elevations - only taking a single
           sample at the centre of the sun will yield an incorrect power
           estimate for the solar disc! Always take multiple random samples
           across the entire solar disc to estimate its power!
           
CAVEAT #4: in this version, the limb darkening calculations still use a fairly
           computationally expensive 5th order polynomial that was directly 
           taken from astronomical literature. For the purposes of Computer
           Graphics, this is needlessly accurate, though, and will be replaced 
           by a cheaper approximation in a future release.

"Alien World" functionality
---------------------------

The Hosek sky model can be used to roughly (!) predict the appearance of 
outdoor scenes on earth-like planets, i.e. planets of a similar size and 
atmospheric make-up. Since the spectral version of our model predicts sky dome 
luminance patterns and solar radiance independently for each waveband, and 
since the intensity of each waveband is solely dependent on the input radiance 
from the star that the world in question is orbiting, it is trivial to re-scale 
the wavebands to match a different star radiance.

At least in theory, the spectral version of the model has always been capable 
of this sort of thing, and the actual sky dome and solar radiance models were 
actually not altered at all in this release. All we did was to add some support
functionality for doing this more easily with the existing data and functions, 
and to add some explanations.

Just use 'arhosekskymodelstate_alienworld_alloc_init()' to initialise the sky
model states (you will have to provide values for star temperature and solar 
intensity compared to the terrestrial sun), and do everything else as you 
did before.

CAVEAT #1: we assume the emission of the star that illuminates the alien world 
           to be a perfect blackbody emission spectrum. This is never entirely 
           realistic - real star emission spectra are considerably more complex 
           than this, mainly due to absorption effects in the outer layers of 
           stars. However, blackbody spectra are a reasonable first assumption 
           in a usage scenario like this, where 100% accuracy is simply not 
           necessary: for rendering purposes, there are likely no visible 
           differences between a highly accurate solution based on a more 
           involved simulation, and this approximation.

CAVEAT #2: we always use limb darkening data from our own sun to provide this
           "appearance feature", even for suns of strongly different 
           temperature. Which is presumably not very realistic, but (as with 
           the unaltered blackbody spectrum from caveat #1) probably not a bad 
           first guess, either. If you need more accuracy than we provide here,
           please make inquiries with a friendly astro-physicst of your choice.

CAVEAT #3: you have to provide a value for the solar intensity of the star 
           which illuminates the alien world. For this, please bear in mind  
           that there is very likely a comparatively tight range of absolute  
           solar irradiance values for which an earth-like planet with an  
           atmosphere like the one we assume in our model can exist in the  
           first place!
            
           Too much irradiance, and the atmosphere probably boils off into 
           space, too little, it freezes. Which means that stars of 
           considerably different emission colour than our sun will have to be 
           fairly different in size from it, to still provide a reasonable and 
           inhabitable amount of irradiance. Red stars will need to be much 
           larger than our sun, while white or blue stars will have to be 
           comparatively tiny. The initialisation function handles this and 
           computes a plausible solar radius for a given emission spectrum. In
           terms of absolute radiometric values, you should probably not stray
           all too far from a solar intensity value of 1.0.

CAVEAT #4: although we now support different solar radii for the actual solar 
           disc, the sky dome luminance patterns are *not* parameterised by 
           this value - i.e. the patterns stay exactly the same for different 
           solar radii! Which is of course not correct. But in our experience, 
           solar discs up to several degrees in diameter (! - our own sun is 
           half a degree across) do not cause the luminance patterns on the sky 
           to change perceptibly. The reason we know this is that we initially 
           used unrealistically large suns in our brute force path tracer, in 
           order to improve convergence speeds (which in the beginning were 
           abysmal). Later, we managed to do the reference renderings much 
           faster even with realistically small suns, and found that there was 
           no real difference in skydome appearance anyway. 
           Conclusion: changing the solar radius should not be over-done, so  
           close orbits around red supergiants are a no-no. But for the  
           purposes of getting a fairly credible first impression of what an 
           alien world with a reasonably sized sun would look like, what we are  
           doing here is probably still o.k.

HINT #1:   if you want to model the sky of an earth-like planet that orbits 
           a binary star, just super-impose two of these models with solar 
           intensity of ~0.5 each, and closely spaced solar positions. Light is
           additive, after all. Tattooine, here we come... :-)

           P.S. according to Star Wars canon, Tattooine orbits a binary
           that is made up of a G and K class star, respectively. 
           So ~5500K and ~4200K should be good first guesses for their 
           temperature. Just in case you were wondering, after reading the
           previous paragraph.
*/


#ifndef _ARHOSEK_SKYMODEL_H_
#define _ARHOSEK_SKYMODEL_H_

typedef double ArHosekSkyModelConfiguration[9];


//   Spectral version of the model

/* ----------------------------------------------------------------------------

    ArHosekSkyModelState struct
    ---------------------------

    This struct holds the pre-computation data for one particular albedo value.
    Most fields are self-explanatory, but users should never directly 
    manipulate any of them anyway. The only consistent way to manipulate such 
    structs is via the functions 'arhosekskymodelstate_alloc_init' and 
    'arhosekskymodelstate_free'.
    
    'emission_correction_factor_sky'
    'emission_correction_factor_sun'

        The original model coefficients were fitted against the emission of 
        our local sun. If a different solar emission is desired (i.e. if the
        model is being used to predict skydome appearance for an earth-like 
        planet that orbits a different star), these correction factors, which 
        are determined during the alloc_init step, are applied to each waveband 
        separately (they default to 1.0 in normal usage). This is the simplest 
        way to retrofit this sort of capability to the existing model. The 
        different factors for sky and sun are needed since the solar disc may 
        be of a different size compared to the terrestrial sun.

---------------------------------------------------------------------------- */

typedef struct ArHosekSkyModelState
{
    ArHosekSkyModelConfiguration  configs[11];
    double                        radiances[11];
    double                        turbidity;
    double                        solar_radius;
    double                        emission_correction_factor_sky[11];
    double                        emission_correction_factor_sun[11];
    double                        albedo;
    double                        elevation;
} 
ArHosekSkyModelState;

/* ----------------------------------------------------------------------------

    arhosekskymodelstate_alloc_init() function
    ------------------------------------------

    Initialises an ArHosekSkyModelState struct for a terrestrial setting.

---------------------------------------------------------------------------- */

ArHosekSkyModelState  * arhosekskymodelstate_alloc_init(
        const double  solar_elevation,
        const double  atmospheric_turbidity,
        const double  ground_albedo
        );


/* ----------------------------------------------------------------------------

    arhosekskymodelstate_alienworld_alloc_init() function
    -----------------------------------------------------

    Initialises an ArHosekSkyModelState struct for an "alien world" setting
    with a sun of a surface temperature given in 'kelvin'. The parameter
    'solar_intensity' controls the overall brightness of the sky, relative
    to the solar irradiance on Earth. A value of 1.0 yields a sky dome that
    is, on average over the wavelenghts covered in the model (!), as bright
    as the terrestrial sky in radiometric terms. 
    
    Which means that the solar radius has to be adjusted, since the 
    emissivity of a solar surface with a given temperature is more or less 
    fixed. So hotter suns have to be smaller to be equally bright as the 
    terrestrial sun, while cooler suns have to be larger. Note that there are
    limits to the validity of the luminance patterns of the underlying model:
    see the discussion above for more on this. In particular, an alien sun with
    a surface temperature of only 2000 Kelvin has to be very large if it is
    to be as bright as the terrestrial sun - so large that the luminance 
    patterns are no longer a really good fit in that case.
    
    If you need information about the solar radius that the model computes
    for a given temperature (say, for light source sampling purposes), you 
    have to query the 'solar_radius' variable of the sky model state returned 
    *after* running this function.

---------------------------------------------------------------------------- */

ArHosekSkyModelState  * arhosekskymodelstate_alienworld_alloc_init(
        const double  solar_elevation,
        const double  solar_intensity,
        const double  solar_surface_temperature_kelvin,
        const double  atmospheric_turbidity,
        const double  ground_albedo
        );

void arhosekskymodelstate_free(
        ArHosekSkyModelState  * state
        );

double arhosekskymodel_radiance(
        ArHosekSkyModelState  * state,
        double                  theta, 
        double                  gamma, 
        double                  wavelength
        );

// CIE XYZ and RGB versions


ArHosekSkyModelState  * arhosek_xyz_skymodelstate_alloc_init(
        const double  turbidity, 
        const double  albedo, 
        const double  elevation
        );


ArHosekSkyModelState  * arhosek_rgb_skymodelstate_alloc_init(
        const double  turbidity, 
        const double  albedo, 
        const double  elevation
        );


double arhosek_tristim_skymodel_radiance(
        ArHosekSkyModelState  * state,
        double                  theta,
        double                  gamma, 
        int                     channel
        );

//   Delivers the complete function: sky + sun, including limb darkening.
//   Please read the above description before using this - there are several
//   caveats!

double arhosekskymodel_solar_radiance(
        ArHosekSkyModelState      * state,
        double                      theta,
        double                      gamma,
        double                      wavelength
        );


#endif // _ARHOSEK_SKYMODEL_H_
