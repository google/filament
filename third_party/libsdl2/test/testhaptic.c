/*
Copyright (c) 2008, Edgar Simo Serra
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Simple Directmedia Layer (SDL) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * includes
 */
#include <stdlib.h>
#include <string.h>             /* strstr */
#include <ctype.h>              /* isdigit */

#include "SDL.h"

#ifndef SDL_HAPTIC_DISABLED

static SDL_Haptic *haptic;


/*
 * prototypes
 */
static void abort_execution(void);
static void HapticPrintSupported(SDL_Haptic * haptic);


/**
 * @brief The entry point of this force feedback demo.
 * @param[in] argc Number of arguments.
 * @param[in] argv Array of argc arguments.
 */
int
main(int argc, char **argv)
{
    int i;
    char *name;
    int index;
    SDL_HapticEffect efx[9];
    int id[9];
    int nefx;
    unsigned int supported;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    name = NULL;
    index = -1;
    if (argc > 1) {
        name = argv[1];
        if ((strcmp(name, "--help") == 0) || (strcmp(name, "-h") == 0)) {
            SDL_Log("USAGE: %s [device]\n"
                   "If device is a two-digit number it'll use it as an index, otherwise\n"
                   "it'll use it as if it were part of the device's name.\n",
                   argv[0]);
            return 0;
        }

        i = strlen(name);
        if ((i < 3) && isdigit(name[0]) && ((i == 1) || isdigit(name[1]))) {
            index = atoi(name);
            name = NULL;
        }
    }

    /* Initialize the force feedbackness */
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK |
             SDL_INIT_HAPTIC);
    SDL_Log("%d Haptic devices detected.\n", SDL_NumHaptics());
    if (SDL_NumHaptics() > 0) {
        /* We'll just use index or the first force feedback device found */
        if (name == NULL) {
            i = (index != -1) ? index : 0;
        }
        /* Try to find matching device */
        else {
            for (i = 0; i < SDL_NumHaptics(); i++) {
                if (strstr(SDL_HapticName(i), name) != NULL)
                    break;
            }

            if (i >= SDL_NumHaptics()) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to find device matching '%s', aborting.\n",
                       name);
                return 1;
            }
        }

        haptic = SDL_HapticOpen(i);
        if (haptic == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create the haptic device: %s\n",
                   SDL_GetError());
            return 1;
        }
        SDL_Log("Device: %s\n", SDL_HapticName(i));
        HapticPrintSupported(haptic);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No Haptic devices found!\n");
        return 1;
    }

    /* We only want force feedback errors. */
    SDL_ClearError();

    /* Create effects. */
    memset(&efx, 0, sizeof(efx));
    nefx = 0;
    supported = SDL_HapticQuery(haptic);

    SDL_Log("\nUploading effects\n");
    /* First we'll try a SINE effect. */
    if (supported & SDL_HAPTIC_SINE) {
        SDL_Log("   effect %d: Sine Wave\n", nefx);
        efx[nefx].type = SDL_HAPTIC_SINE;
        efx[nefx].periodic.period = 1000;
        efx[nefx].periodic.magnitude = -0x2000;    /* Negative magnitude and ...                      */
        efx[nefx].periodic.phase = 18000;          /* ... 180 degrees phase shift => cancel eachother */
        efx[nefx].periodic.length = 5000;
        efx[nefx].periodic.attack_length = 1000;
        efx[nefx].periodic.fade_length = 1000;
        id[nefx] = SDL_HapticNewEffect(haptic, &efx[nefx]);
        if (id[nefx] < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "UPLOADING EFFECT ERROR: %s\n", SDL_GetError());
            abort_execution();
        }
        nefx++;
    }
    /* Now we'll try a SAWTOOTHUP */
    if (supported & SDL_HAPTIC_SAWTOOTHUP) {
        SDL_Log("   effect %d: Sawtooth Up\n", nefx);
        efx[nefx].type = SDL_HAPTIC_SAWTOOTHUP;
        efx[nefx].periodic.period = 500;
        efx[nefx].periodic.magnitude = 0x5000;
        efx[nefx].periodic.length = 5000;
        efx[nefx].periodic.attack_length = 1000;
        efx[nefx].periodic.fade_length = 1000;
        id[nefx] = SDL_HapticNewEffect(haptic, &efx[nefx]);
        if (id[nefx] < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "UPLOADING EFFECT ERROR: %s\n", SDL_GetError());
            abort_execution();
        }
        nefx++;
    }
    
    /* Now the classical constant effect. */
    if (supported & SDL_HAPTIC_CONSTANT) {
        SDL_Log("   effect %d: Constant Force\n", nefx);
        efx[nefx].type = SDL_HAPTIC_CONSTANT;
        efx[nefx].constant.direction.type = SDL_HAPTIC_POLAR;
        efx[nefx].constant.direction.dir[0] = 20000;    /* Force comes from the south-west. */
        efx[nefx].constant.length = 5000;
        efx[nefx].constant.level = 0x6000;
        efx[nefx].constant.attack_length = 1000;
        efx[nefx].constant.fade_length = 1000;
        id[nefx] = SDL_HapticNewEffect(haptic, &efx[nefx]);
        if (id[nefx] < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "UPLOADING EFFECT ERROR: %s\n", SDL_GetError());
            abort_execution();
        }
        nefx++;
    }
    
    /* The cute spring effect. */
    if (supported & SDL_HAPTIC_SPRING) {
        SDL_Log("   effect %d: Condition Spring\n", nefx);
        efx[nefx].type = SDL_HAPTIC_SPRING;
        efx[nefx].condition.length = 5000;
        for (i = 0; i < SDL_HapticNumAxes(haptic); i++) {
            efx[nefx].condition.right_sat[i] = 0xFFFF;
            efx[nefx].condition.left_sat[i] = 0xFFFF;
            efx[nefx].condition.right_coeff[i] = 0x2000;
            efx[nefx].condition.left_coeff[i] = 0x2000;
            efx[nefx].condition.center[i] = 0x1000;     /* Displace the center for it to move. */
        }
        id[nefx] = SDL_HapticNewEffect(haptic, &efx[nefx]);
        if (id[nefx] < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "UPLOADING EFFECT ERROR: %s\n", SDL_GetError());
            abort_execution();
        }
        nefx++;
    }
    /* The interesting damper effect. */
    if (supported & SDL_HAPTIC_DAMPER) {
        SDL_Log("   effect %d: Condition Damper\n", nefx);
        efx[nefx].type = SDL_HAPTIC_DAMPER;
        efx[nefx].condition.length = 5000;
        for (i = 0; i < SDL_HapticNumAxes(haptic); i++) {
            efx[nefx].condition.right_sat[i] = 0xFFFF;
            efx[nefx].condition.left_sat[i] = 0xFFFF;
            efx[nefx].condition.right_coeff[i] = 0x2000;
            efx[nefx].condition.left_coeff[i] = 0x2000;
        }
        id[nefx] = SDL_HapticNewEffect(haptic, &efx[nefx]);
        if (id[nefx] < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "UPLOADING EFFECT ERROR: %s\n", SDL_GetError());
            abort_execution();
        }
        nefx++;
    }
    /* The pretty awesome inertia effect. */
    if (supported & SDL_HAPTIC_INERTIA) {
        SDL_Log("   effect %d: Condition Inertia\n", nefx);
        efx[nefx].type = SDL_HAPTIC_INERTIA;
        efx[nefx].condition.length = 5000;
        for (i = 0; i < SDL_HapticNumAxes(haptic); i++) {
            efx[nefx].condition.right_sat[i] = 0xFFFF;
            efx[nefx].condition.left_sat[i] = 0xFFFF;
            efx[nefx].condition.right_coeff[i] = 0x2000;
            efx[nefx].condition.left_coeff[i] = 0x2000;
            efx[nefx].condition.deadband[i] = 0x1000;    /* 1/16th of axis-range around the center is 'dead'. */
        }
        id[nefx] = SDL_HapticNewEffect(haptic, &efx[nefx]);
        if (id[nefx] < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "UPLOADING EFFECT ERROR: %s\n", SDL_GetError());
            abort_execution();
        }
        nefx++;
    }
    /* The hot friction effect. */
    if (supported & SDL_HAPTIC_FRICTION) {
        SDL_Log("   effect %d: Condition Friction\n", nefx);
        efx[nefx].type = SDL_HAPTIC_FRICTION;
        efx[nefx].condition.length = 5000;
        for (i = 0; i < SDL_HapticNumAxes(haptic); i++) {
            efx[nefx].condition.right_sat[i] = 0xFFFF;
            efx[nefx].condition.left_sat[i] = 0xFFFF;
            efx[nefx].condition.right_coeff[i] = 0x2000;
            efx[nefx].condition.left_coeff[i] = 0x2000;
        }
        id[nefx] = SDL_HapticNewEffect(haptic, &efx[nefx]);
        if (id[nefx] < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "UPLOADING EFFECT ERROR: %s\n", SDL_GetError());
            abort_execution();
        }
        nefx++;
    }
    
    /* Now we'll try a ramp effect */
    if (supported & SDL_HAPTIC_RAMP) {
        SDL_Log("   effect %d: Ramp\n", nefx);
        efx[nefx].type = SDL_HAPTIC_RAMP;
        efx[nefx].ramp.direction.type = SDL_HAPTIC_CARTESIAN;
        efx[nefx].ramp.direction.dir[0] = 1;     /* Force comes from                 */
        efx[nefx].ramp.direction.dir[1] = -1;    /*                  the north-east. */
        efx[nefx].ramp.length = 5000;
        efx[nefx].ramp.start = 0x4000;
        efx[nefx].ramp.end = -0x4000;
        efx[nefx].ramp.attack_length = 1000;
        efx[nefx].ramp.fade_length = 1000;
        id[nefx] = SDL_HapticNewEffect(haptic, &efx[nefx]);
        if (id[nefx] < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "UPLOADING EFFECT ERROR: %s\n", SDL_GetError());
            abort_execution();
        }
        nefx++;
    }

    /* Finally we'll try a left/right effect. */
    if (supported & SDL_HAPTIC_LEFTRIGHT) {
        SDL_Log("   effect %d: Left/Right\n", nefx);
        efx[nefx].type = SDL_HAPTIC_LEFTRIGHT;
        efx[nefx].leftright.length = 5000;
        efx[nefx].leftright.large_magnitude = 0x3000;
        efx[nefx].leftright.small_magnitude = 0xFFFF;
        id[nefx] = SDL_HapticNewEffect(haptic, &efx[nefx]);
        if (id[nefx] < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "UPLOADING EFFECT ERROR: %s\n", SDL_GetError());
            abort_execution();
        }
        nefx++;
    }


    SDL_Log
        ("\nNow playing effects for 5 seconds each with 1 second delay between\n");
    for (i = 0; i < nefx; i++) {
        SDL_Log("   Playing effect %d\n", i);
        SDL_HapticRunEffect(haptic, id[i], 1);
        SDL_Delay(6000);        /* Effects only have length 5000 */
    }

    /* Quit */
    if (haptic != NULL)
        SDL_HapticClose(haptic);
    SDL_Quit();

    return 0;
}


/*
 * Cleans up a bit.
 */
static void
abort_execution(void)
{
    SDL_Log("\nAborting program execution.\n");

    SDL_HapticClose(haptic);
    SDL_Quit();

    exit(1);
}


/*
 * Displays information about the haptic device.
 */
static void
HapticPrintSupported(SDL_Haptic * haptic)
{
    unsigned int supported;

    supported = SDL_HapticQuery(haptic);
    SDL_Log("   Supported effects [%d effects, %d playing]:\n",
           SDL_HapticNumEffects(haptic), SDL_HapticNumEffectsPlaying(haptic));
    if (supported & SDL_HAPTIC_CONSTANT)
        SDL_Log("      constant\n");
    if (supported & SDL_HAPTIC_SINE)
        SDL_Log("      sine\n");
    /* !!! FIXME: put this back when we have more bits in 2.1 */
    /* if (supported & SDL_HAPTIC_SQUARE)
        SDL_Log("      square\n"); */
    if (supported & SDL_HAPTIC_TRIANGLE)
        SDL_Log("      triangle\n");
    if (supported & SDL_HAPTIC_SAWTOOTHUP)
        SDL_Log("      sawtoothup\n");
    if (supported & SDL_HAPTIC_SAWTOOTHDOWN)
        SDL_Log("      sawtoothdown\n");
    if (supported & SDL_HAPTIC_RAMP)
        SDL_Log("      ramp\n");
    if (supported & SDL_HAPTIC_FRICTION)
        SDL_Log("      friction\n");
    if (supported & SDL_HAPTIC_SPRING)
        SDL_Log("      spring\n");
    if (supported & SDL_HAPTIC_DAMPER)
        SDL_Log("      damper\n");
    if (supported & SDL_HAPTIC_INERTIA)
        SDL_Log("      inertia\n");
    if (supported & SDL_HAPTIC_CUSTOM)
        SDL_Log("      custom\n");
    if (supported & SDL_HAPTIC_LEFTRIGHT)
        SDL_Log("      left/right\n");
    SDL_Log("   Supported capabilities:\n");
    if (supported & SDL_HAPTIC_GAIN)
        SDL_Log("      gain\n");
    if (supported & SDL_HAPTIC_AUTOCENTER)
        SDL_Log("      autocenter\n");
    if (supported & SDL_HAPTIC_STATUS)
        SDL_Log("      status\n");
}

#else

int
main(int argc, char *argv[])
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL compiled without Haptic support.\n");
    return 1;
}

#endif
