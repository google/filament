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

All instructions on how to use this code are in the accompanying header file.

*/

#include "ArHosekSkyModel.h"
#include "ArHosekSkyModelData_Spectral.h"
#include "ArHosekSkyModelData_CIEXYZ.h"
#include "ArHosekSkyModelData_RGB.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//   Some macro definitions that occur elsewhere in ART, and that have to be
//   replicated to make this a stand-alone module.

#ifndef NIL
#define NIL                         0
#endif

#ifndef MATH_PI 
#define MATH_PI                     3.141592653589793
#endif

#ifndef MATH_DEG_TO_RAD
#define MATH_DEG_TO_RAD             ( MATH_PI / 180.0 )
#endif

#ifndef MATH_RAD_TO_DEG
#define MATH_RAD_TO_DEG             ( 180.0 / MATH_PI )
#endif

#ifndef DEGREES
#define DEGREES                     * MATH_DEG_TO_RAD
#endif

#ifndef TERRESTRIAL_SOLAR_RADIUS
#define TERRESTRIAL_SOLAR_RADIUS    ( ( 0.51 DEGREES ) / 2.0 )
#endif

#ifndef ALLOC
#define ALLOC(_struct)              ((_struct *)malloc(sizeof(_struct)))
#endif

// internal definitions

typedef double *ArHosekSkyModel_Dataset;
typedef double *ArHosekSkyModel_Radiance_Dataset;

// internal functions

void ArHosekSkyModel_CookConfiguration(
        ArHosekSkyModel_Dataset       dataset, 
        ArHosekSkyModelConfiguration  config, 
        double                        turbidity, 
        double                        albedo, 
        double                        solar_elevation
        )
{
    double  * elev_matrix;

    int     int_turbidity = (int)turbidity;
    double  turbidity_rem = turbidity - (double)int_turbidity;

    solar_elevation = pow(solar_elevation / (MATH_PI / 2.0), (1.0 / 3.0));

    // alb 0 low turb

    elev_matrix = dataset + ( 9 * 6 * (int_turbidity-1) );
    
    
    for( unsigned int i = 0; i < 9; ++i )
    {
        //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
        config[i] = 
        (1.0-albedo) * (1.0 - turbidity_rem) 
        * ( pow(1.0-solar_elevation, 5.0) * elev_matrix[i]  + 
           5.0  * pow(1.0-solar_elevation, 4.0) * solar_elevation * elev_matrix[i+9] +
           10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * elev_matrix[i+18] +
           10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * elev_matrix[i+27] +
           5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * elev_matrix[i+36] +
           pow(solar_elevation, 5.0)  * elev_matrix[i+45]);
    }

    // alb 1 low turb
    elev_matrix = dataset + (9*6*10 + 9*6*(int_turbidity-1));
    for(unsigned int i = 0; i < 9; ++i)
    {
        //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
        config[i] += 
        (albedo) * (1.0 - turbidity_rem)
        * ( pow(1.0-solar_elevation, 5.0) * elev_matrix[i]  + 
           5.0  * pow(1.0-solar_elevation, 4.0) * solar_elevation * elev_matrix[i+9] +
           10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * elev_matrix[i+18] +
           10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * elev_matrix[i+27] +
           5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * elev_matrix[i+36] +
           pow(solar_elevation, 5.0)  * elev_matrix[i+45]);
    }

    if(int_turbidity == 10)
        return;

    // alb 0 high turb
    elev_matrix = dataset + (9*6*(int_turbidity));
    for(unsigned int i = 0; i < 9; ++i)
    {
        //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
        config[i] += 
        (1.0-albedo) * (turbidity_rem)
        * ( pow(1.0-solar_elevation, 5.0) * elev_matrix[i]  + 
           5.0  * pow(1.0-solar_elevation, 4.0) * solar_elevation * elev_matrix[i+9] +
           10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * elev_matrix[i+18] +
           10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * elev_matrix[i+27] +
           5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * elev_matrix[i+36] +
           pow(solar_elevation, 5.0)  * elev_matrix[i+45]);
    }

    // alb 1 high turb
    elev_matrix = dataset + (9*6*10 + 9*6*(int_turbidity));
    for(unsigned int i = 0; i < 9; ++i)
    {
        //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
        config[i] += 
        (albedo) * (turbidity_rem)
        * ( pow(1.0-solar_elevation, 5.0) * elev_matrix[i]  + 
           5.0  * pow(1.0-solar_elevation, 4.0) * solar_elevation * elev_matrix[i+9] +
           10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * elev_matrix[i+18] +
           10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * elev_matrix[i+27] +
           5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * elev_matrix[i+36] +
           pow(solar_elevation, 5.0)  * elev_matrix[i+45]);
    }
}

double ArHosekSkyModel_CookRadianceConfiguration(
        ArHosekSkyModel_Radiance_Dataset  dataset, 
        double                            turbidity, 
        double                            albedo, 
        double                            solar_elevation
        )
{
    double* elev_matrix;

    int int_turbidity = (int)turbidity;
    double turbidity_rem = turbidity - (double)int_turbidity;
    double res;
    solar_elevation = pow(solar_elevation / (MATH_PI / 2.0), (1.0 / 3.0));

    // alb 0 low turb
    elev_matrix = dataset + (6*(int_turbidity-1));
    //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
    res = (1.0-albedo) * (1.0 - turbidity_rem) *
        ( pow(1.0-solar_elevation, 5.0) * elev_matrix[0] +
         5.0*pow(1.0-solar_elevation, 4.0)*solar_elevation * elev_matrix[1] +
         10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * elev_matrix[2] +
         10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * elev_matrix[3] +
         5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * elev_matrix[4] +
         pow(solar_elevation, 5.0) * elev_matrix[5]);

    // alb 1 low turb
    elev_matrix = dataset + (6*10 + 6*(int_turbidity-1));
    //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
    res += (albedo) * (1.0 - turbidity_rem) *
        ( pow(1.0-solar_elevation, 5.0) * elev_matrix[0] +
         5.0*pow(1.0-solar_elevation, 4.0)*solar_elevation * elev_matrix[1] +
         10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * elev_matrix[2] +
         10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * elev_matrix[3] +
         5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * elev_matrix[4] +
         pow(solar_elevation, 5.0) * elev_matrix[5]);
    if(int_turbidity == 10)
        return res;

    // alb 0 high turb
    elev_matrix = dataset + (6*(int_turbidity));
    //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
    res += (1.0-albedo) * (turbidity_rem) *
        ( pow(1.0-solar_elevation, 5.0) * elev_matrix[0] +
         5.0*pow(1.0-solar_elevation, 4.0)*solar_elevation * elev_matrix[1] +
         10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * elev_matrix[2] +
         10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * elev_matrix[3] +
         5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * elev_matrix[4] +
         pow(solar_elevation, 5.0) * elev_matrix[5]);

    // alb 1 high turb
    elev_matrix = dataset + (6*10 + 6*(int_turbidity));
    //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
    res += (albedo) * (turbidity_rem) *
        ( pow(1.0-solar_elevation, 5.0) * elev_matrix[0] +
         5.0*pow(1.0-solar_elevation, 4.0)*solar_elevation * elev_matrix[1] +
         10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * elev_matrix[2] +
         10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * elev_matrix[3] +
         5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * elev_matrix[4] +
         pow(solar_elevation, 5.0) * elev_matrix[5]);
    return res;
}

double ArHosekSkyModel_GetRadianceInternal(
        ArHosekSkyModelConfiguration  configuration, 
        double                        theta, 
        double                        gamma
        )
{
    const double expM = exp(configuration[4] * gamma);
    const double rayM = cos(gamma)*cos(gamma);
    const double mieM = (1.0 + cos(gamma)*cos(gamma)) / pow((1.0 + configuration[8]*configuration[8] - 2.0*configuration[8]*cos(gamma)), 1.5);
    const double zenith = sqrt(cos(theta));

    return (1.0 + configuration[0] * exp(configuration[1] / (cos(theta) + 0.01))) *
            (configuration[2] + configuration[3] * expM + configuration[5] * rayM + configuration[6] * mieM + configuration[7] * zenith);
}

// spectral version

ArHosekSkyModelState  * arhosekskymodelstate_alloc_init(
        const double  solar_elevation,
        const double  atmospheric_turbidity,
        const double  ground_albedo
        )
{
    ArHosekSkyModelState  * state = ALLOC(ArHosekSkyModelState);

    state->solar_radius = ( 0.51 DEGREES ) / 2.0;
    state->turbidity    = atmospheric_turbidity;
    state->albedo       = ground_albedo;
    state->elevation    = solar_elevation;

    for( unsigned int wl = 0; wl < 11; ++wl )
    {
        ArHosekSkyModel_CookConfiguration(
            datasets[wl], 
            state->configs[wl], 
            atmospheric_turbidity, 
            ground_albedo, 
            solar_elevation
            );

        state->radiances[wl] = 
            ArHosekSkyModel_CookRadianceConfiguration(
                datasetsRad[wl],
                atmospheric_turbidity,
                ground_albedo,
                solar_elevation
                );

        state->emission_correction_factor_sun[wl] = 1.0;
        state->emission_correction_factor_sky[wl] = 1.0;
    }

    return state;
}

//   'blackbody_scaling_factor'
//
//   Fudge factor, computed in Mathematica, to scale the results of the
//   following function to match the solar radiance spectrum used in the
//   original simulation. The scaling is done so their integrals over the
//   range from 380.0 to 720.0 nanometers match for a blackbody temperature
//   of 5800 K.
//   Which leaves the original spectrum being less bright overall than the 5.8k
//   blackbody radiation curve if the ultra-violet part of the spectrum is
//   also considered. But the visible brightness should be very similar.

const double blackbody_scaling_factor = 3.19992 * 10E-11;

//   'art_blackbody_dd_value()' function
//
//   Blackbody radiance, Planck's formula

double art_blackbody_dd_value(
        const double  temperature,
        const double  lambda
        )
{
    double  c1 = 3.74177 * 10E-17;
    double  c2 = 0.0143878;
    double  value;
    
    value =   ( c1 / ( pow( lambda, 5.0 ) ) )
            * ( 1.0 / ( exp( c2 / ( lambda * temperature ) ) - 1.0 ) );

    return value;
}

//   'originalSolarRadianceTable[]'
//
//   The solar spectrum incident at the top of the atmosphere, as it was used 
//   in the brute force path tracer that generated the reference results the 
//   model was fitted to. We need this as the yardstick to compare any altered 
//   Blackbody emission spectra for alien world stars to.

//   This is just the data from the Preetham paper, extended into the UV range.

const double originalSolarRadianceTable[] =
{
     7500.0,
    12500.0,
    21127.5,
    26760.5,
    30663.7,
    27825.0,
    25503.8,
    25134.2,
    23212.1,
    21526.7,
    19870.8
};

ArHosekSkyModelState  * arhosekskymodelstate_alienworld_alloc_init(
        const double  solar_elevation,
        const double  solar_intensity,
        const double  solar_surface_temperature_kelvin,
        const double  atmospheric_turbidity,
        const double  ground_albedo
        )
{
    ArHosekSkyModelState  * state = ALLOC(ArHosekSkyModelState);

    state->turbidity    = atmospheric_turbidity;
    state->albedo       = ground_albedo;
    state->elevation    = solar_elevation;
    
    for( unsigned int wl = 0; wl < 11; ++wl )
    {
        //   Basic init as for the normal scenario
        
        ArHosekSkyModel_CookConfiguration(
            datasets[wl], 
            state->configs[wl], 
            atmospheric_turbidity, 
            ground_albedo, 
            solar_elevation
            );

        state->radiances[wl] = 
            ArHosekSkyModel_CookRadianceConfiguration(
                datasetsRad[wl],
                atmospheric_turbidity, 
                ground_albedo,
                solar_elevation
                );
        
        //   The wavelength of this band in nanometers
        
        double  owl = ( 320.0 + 40.0 * wl ) * 10E-10;
        
        //   The original intensity we just computed
        
        double  osr = originalSolarRadianceTable[wl];
        
        //   The intensity of a blackbody with the desired temperature
        //   The fudge factor described above is used to make sure the BB
        //   function matches the used radiance data reasonably well
        //   in magnitude.
        
        double  nsr =
              art_blackbody_dd_value(solar_surface_temperature_kelvin, owl)
            * blackbody_scaling_factor;

        //   Correction factor for this waveband is simply the ratio of
        //   the two.

        state->emission_correction_factor_sun[wl] = nsr / osr;
    }

    //   We then compute the average correction factor of all wavebands.

    //   Theoretically, some weighting to favour wavelengths human vision is
    //   more sensitive to could be introduced here - think V(lambda). But 
    //   given that the whole effort is not *that* accurate to begin with (we
    //   are talking about the appearance of alien worlds, after all), simple
    //   averaging over the visible wavelenghts (! - this is why we start at
    //   WL #2, and only use 2-11) seems like a sane first approximation.
    
    double  correctionFactor = 0.0;
    
    for ( unsigned int i = 2; i < 11; i++ )
    {
        correctionFactor +=
            state->emission_correction_factor_sun[i];
    }
    
    //   This is the average ratio in emitted energy between our sun, and an 
    //   equally large sun with the blackbody spectrum we requested.
    
    //   Division by 9 because we only used 9 of the 11 wavelengths for this
    //   (see above).
    
    double  ratio = correctionFactor / 9.0;

    //   This ratio is then used to determine the radius of the alien sun
    //   on the sky dome. The additional factor 'solar_intensity' can be used
    //   to make the alien sun brighter or dimmer compared to our sun.
    
    state->solar_radius =
          ( sqrt( solar_intensity ) * TERRESTRIAL_SOLAR_RADIUS )
        / sqrt( ratio );

    //   Finally, we have to reduce the scaling factor of the sky by the
    //   ratio used to scale the solar disc size. The rationale behind this is 
    //   that the scaling factors apply to the new blackbody spectrum, which 
    //   can be more or less bright than the one our sun emits. However, we 
    //   just scaled the size of the alien solar disc so it is roughly as 
    //   bright (in terms of energy emitted) as the terrestrial sun. So the sky 
    //   dome has to be reduced in brightness appropriately - but not in an 
    //   uniform fashion across wavebands. If we did that, the sky colour would
    //   be wrong.
    
    for ( unsigned int i = 0; i < 11; i++ )
    {
        state->emission_correction_factor_sky[i] =
              solar_intensity
            * state->emission_correction_factor_sun[i] / ratio;
    }
    
    return state;
}

void arhosekskymodelstate_free(
        ArHosekSkyModelState  * state
        )
{
    free(state);
}

double arhosekskymodel_radiance(
        ArHosekSkyModelState  * state,
        double                  theta, 
        double                  gamma, 
        double                  wavelength
        )
{
    int low_wl = (wavelength - 320.0 ) / 40.0;

    if ( low_wl < 0 || low_wl >= 11 )
        return 0.0f;

    double interp = fmod((wavelength - 320.0 ) / 40.0, 1.0);

    double val_low = 
          ArHosekSkyModel_GetRadianceInternal(
                state->configs[low_wl],
                theta,
                gamma
              )
        * state->radiances[low_wl]
        * state->emission_correction_factor_sky[low_wl];

    if ( interp < 1e-6 )
        return val_low;

    double result = ( 1.0 - interp ) * val_low;

    if ( low_wl+1 < 11 )
    {
        result +=
              interp
            * ArHosekSkyModel_GetRadianceInternal(
                    state->configs[low_wl+1],
                    theta,
                    gamma
                  )
            * state->radiances[low_wl+1]
            * state->emission_correction_factor_sky[low_wl+1];
    }

    return result;
}


// xyz and rgb versions

ArHosekSkyModelState  * arhosek_xyz_skymodelstate_alloc_init(
        const double  turbidity, 
        const double  albedo, 
        const double  elevation
        )
{
    ArHosekSkyModelState  * state = ALLOC(ArHosekSkyModelState);

    state->solar_radius = TERRESTRIAL_SOLAR_RADIUS;
    state->turbidity    = turbidity;
    state->albedo       = albedo;
    state->elevation    = elevation;
    
    for( unsigned int channel = 0; channel < 3; ++channel )
    {
        ArHosekSkyModel_CookConfiguration(
            datasetsXYZ[channel], 
            state->configs[channel], 
            turbidity, 
            albedo, 
            elevation
            );
        
        state->radiances[channel] = 
        ArHosekSkyModel_CookRadianceConfiguration(
            datasetsXYZRad[channel],
            turbidity, 
            albedo,
            elevation
            );
    }
    
    return state;
}


ArHosekSkyModelState  * arhosek_rgb_skymodelstate_alloc_init(
        const double  turbidity, 
        const double  albedo, 
        const double  elevation
        )
{
    ArHosekSkyModelState* state = ALLOC(ArHosekSkyModelState);
    
    state->solar_radius = TERRESTRIAL_SOLAR_RADIUS;
    state->turbidity    = turbidity;
    state->albedo       = albedo;
    state->elevation    = elevation;

    for( unsigned int channel = 0; channel < 3; ++channel )
    {
        ArHosekSkyModel_CookConfiguration(
            datasetsRGB[channel], 
            state->configs[channel], 
            turbidity, 
            albedo, 
            elevation
            );
        
        state->radiances[channel] = 
        ArHosekSkyModel_CookRadianceConfiguration(
            datasetsRGBRad[channel],
            turbidity, 
            albedo,
            elevation
            );
    }
    
    return state;
}

double arhosek_tristim_skymodel_radiance(
    ArHosekSkyModelState  * state,
    double                  theta, 
    double                  gamma, 
    int                     channel
    )
{
    return
        ArHosekSkyModel_GetRadianceInternal(
            state->configs[channel], 
            theta, 
            gamma 
            ) 
        * state->radiances[channel];
}

const int pieces = 45;
const int order = 4;

double arhosekskymodel_sr_internal(
        ArHosekSkyModelState  * state,
        int                     turbidity,
        int                     wl,
        double                  elevation
        )
{
    int pos =
        (int) (pow(2.0*elevation / MATH_PI, 1.0/3.0) * pieces); // floor
    
    if ( pos > 44 ) pos = 44;
    
    const double break_x =
        pow(((double) pos / (double) pieces), 3.0) * (MATH_PI * 0.5);

    const double  * coefs =
        solarDatasets[wl] + (order * pieces * turbidity + order * (pos+1) - 1);

    double res = 0.0;
    const double x = elevation - break_x;
    double x_exp = 1.0;

    for (int i = 0; i < order; ++i)
    {
        res += x_exp * *coefs--;
        x_exp *= x;
    }

    return res * state->emission_correction_factor_sun[wl];
}

double arhosekskymodel_solar_radiance_internal2(
        ArHosekSkyModelState  * state,
        double                  wavelength,
        double                  elevation,
        double                  gamma
        )
{
    assert(
           wavelength >= 320.0
        && wavelength <= 720.0
        && state->turbidity >= 1.0
        && state->turbidity <= 10.0
        );
            
    
    int     turb_low  = (int) state->turbidity - 1;
    double  turb_frac = state->turbidity - (double) (turb_low + 1);
    
    if ( turb_low == 9 )
    {
        turb_low  = 8;
        turb_frac = 1.0;
    }

    int    wl_low  = (int) ((wavelength - 320.0) / 40.0);
    double wl_frac = fmod(wavelength, 40.0) / 40.0;
    
    if ( wl_low == 10 )
    {
        wl_low = 9;
        wl_frac = 1.0;
    }

    double direct_radiance =
          ( 1.0 - turb_frac )
        * (    (1.0 - wl_frac)
             * arhosekskymodel_sr_internal(
                     state,
                     turb_low,
                     wl_low,
                     elevation
                   )
           +   wl_frac
             * arhosekskymodel_sr_internal(
                     state,
                     turb_low,
                     wl_low+1,
                     elevation
                   )
          )
      +   turb_frac
        * (    ( 1.0 - wl_frac )
             * arhosekskymodel_sr_internal(
                     state,
                     turb_low+1,
                     wl_low,
                     elevation
                   )
           +   wl_frac
             * arhosekskymodel_sr_internal(
                     state,
                     turb_low+1,
                     wl_low+1,
                     elevation
                   )
          );

    double ldCoefficient[6];
    
    for ( int i = 0; i < 6; i++ )
        ldCoefficient[i] =
              (1.0 - wl_frac) * limbDarkeningDatasets[wl_low  ][i]
            +        wl_frac  * limbDarkeningDatasets[wl_low+1][i];
    
    // sun distance to diameter ratio, squared

    const double sol_rad_sin = sin(state->solar_radius);
    const double ar2 = 1 / ( sol_rad_sin * sol_rad_sin );
    const double singamma = sin(gamma);
    double sc2 = 1.0 - ar2 * singamma * singamma;
    if (sc2 < 0.0 ) sc2 = 0.0;
    double sampleCosine = sqrt (sc2);
    
    //   The following will be improved in future versions of the model:
    //   here, we directly use fitted 5th order polynomials provided by the
    //   astronomical community for the limb darkening effect. Astronomers need
    //   such accurate fittings for their predictions. However, this sort of
    //   accuracy is not really needed for CG purposes, so an approximated
    //   dataset based on quadratic polynomials will be provided in a future
    //   release.

    double  darkeningFactor =
          ldCoefficient[0]
        + ldCoefficient[1] * sampleCosine
        + ldCoefficient[2] * pow( sampleCosine, 2.0 )
        + ldCoefficient[3] * pow( sampleCosine, 3.0 )
        + ldCoefficient[4] * pow( sampleCosine, 4.0 )
        + ldCoefficient[5] * pow( sampleCosine, 5.0 );

    direct_radiance *= darkeningFactor;

    return direct_radiance;
}

double arhosekskymodel_solar_radiance(
        ArHosekSkyModelState  * state,
        double                  theta, 
        double                  gamma, 
        double                  wavelength
        )
{
    double  direct_radiance =
        arhosekskymodel_solar_radiance_internal2(
            state,
            wavelength,
            ((MATH_PI/2.0)-theta),
            gamma
            );

    double  inscattered_radiance =
        arhosekskymodel_radiance(
            state,
            theta,
            gamma,
            wavelength
            );
    
    return  direct_radiance + inscattered_radiance;
}

