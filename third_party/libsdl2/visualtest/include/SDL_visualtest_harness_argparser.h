/**
 *  \file SDL_visualtest_harness_argparser.h
 *
 *  Provides functionality to parse command line arguments to the test harness.
 */

#include <SDL.h>
#include "SDL_visualtest_sut_configparser.h"
#include "SDL_visualtest_variator_common.h"
#include "SDL_visualtest_action_configparser.h"

#ifndef SDL_visualtest_harness_argparser_h_
#define SDL_visualtest_harness_argparser_h_

/** Maximum length of a path string */
#define MAX_PATH_LEN 300
/** Maximum length of a string of SUT arguments */
#define MAX_SUT_ARGS_LEN 600

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Stores the state of the test harness.
 */
typedef struct SDLVisualTest_HarnessState
{
    /*! Path to the System Under Test (SUT) executable */
    char sutapp[MAX_PATH_LEN];
    /*! Command line arguments to be passed to the SUT */
    char sutargs[MAX_SUT_ARGS_LEN];
    /*! Time in milliseconds after which to kill the SUT */
    int timeout;
    /*! Configuration object for the SUT */
    SDLVisualTest_SUTConfig sut_config;
    /*! What type of variator to use to generate argument strings */
    SDLVisualTest_VariatorType variator_type;
    /*! The number of variations to generate */
    int num_variations;
    /*! If true, the test harness will just print the different variations
        without launching the SUT for each one */
    SDL_bool no_launch;
    /*! A queue with actions to be performed while the SUT is running */
    SDLVisualTest_ActionQueue action_queue;
    /*! Output directory to save the screenshots */
    char output_dir[MAX_PATH_LEN];
    /*! Path to directory with the verification images */
    char verify_dir[MAX_PATH_LEN];
} SDLVisualTest_HarnessState;

/**
 * Parse command line paramters to the test harness and populate a state object.
 *
 * \param argv  The array of command line parameters.
 * \param state Pointer to the state object to be populated.
 *
 * \return Non-zero on success, zero on failure.
 */
int SDLVisualTest_ParseHarnessArgs(char** argv, SDLVisualTest_HarnessState* state);

/**
 * Frees any resources associated with the state object pointed to by \c state.
 */
void SDLVisualTest_FreeHarnessState(SDLVisualTest_HarnessState* state);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif /* SDL_visualtest_harness_argparser_h_ */

/* vi: set ts=4 sw=4 expandtab: */
