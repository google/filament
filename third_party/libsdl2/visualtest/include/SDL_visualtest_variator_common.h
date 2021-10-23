/* See LICENSE.txt for the full license governing this code. */
/**
 * \file SDL_visualtest_variator_common.h
 *
 * Header for common functionality used by variators.
 */

#include <SDL_types.h>
#include "SDL_visualtest_sut_configparser.h"

#ifndef SDL_visualtest_variator_common_h_
#define SDL_visualtest_variator_common_h_

/** The number of variations one integer option would generate */
#define SDL_SUT_INTEGER_OPTION_TEST_STEPS 3

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/** enum for indicating the type of variator being used */
typedef enum SDLVisualTest_VariatorType
{
    SDL_VARIATOR_NONE = 0,
    SDL_VARIATOR_EXHAUSTIVE,
    SDL_VARIATOR_RANDOM
} SDLVisualTest_VariatorType;

/**
 * One possible value for a command line option to the SUT.
 */
typedef union SDLVisualTest_SUTOptionValue
{
    /*! Value if the option is of type boolean */
    SDL_bool bool_value;
    /*! Value if the option is of type integer. If on is true then the option
        will be passed to the SUT, otherwise it will be ignored. */
    struct {
        int value;
        SDL_bool on;
    } integer;
    /*! Index of the string in the enum_values field of the corresponding
        SDLVisualTest_SUTOption object. If on is true the option will passed
        to the SUT, otherwise it will be ignored. */
    struct {
        int index;
        SDL_bool on;
    } enumerated;
    /*! Value if the option is of type string. If on is true the option will 
        be passed to the SUT, otherwise it will be ignored. */
    struct {
        char* value;
        SDL_bool on;
    } string;
} SDLVisualTest_SUTOptionValue;

/**
 * Represents a valid combination of parameters that can be passed to the SUT.
 * The ordering of the values here is the same as the ordering of the options in
 * the SDLVisualTest_SUTConfig object for this variation.
 */
typedef struct SDLVisualTest_Variation
{
    /*! Pointer to array of option values */
    SDLVisualTest_SUTOptionValue* vars;
    /*! Number of option values in \c vars */
    int num_vars;
} SDLVisualTest_Variation;

/**
 * "Increments" the value of the option by one and returns the carry. We wrap
 * around to the initial value on overflow which makes the carry one.
 * For example: "incrementing" an SDL_FALSE option makes it SDL_TRUE with no
 * carry, and "incrementing" an SDL_TRUE option makes it SDL_FALSE with carry
 * one. For integers, a random value in the valid range for the option is used.
 *
 * \param var Value of the option
 * \param opt Object with metadata about the option
 *
 * \return 1 if there is a carry for enum and bool type options, 0 otherwise.
 *         1 is always returned for integer and string type options. -1 is
 *         returned on error.
 */
int SDLVisualTest_NextValue(SDLVisualTest_SUTOptionValue* var,
                            SDLVisualTest_SUTOption* opt);

/**
 * Converts a variation object into a string of command line arguments.
 *
 * \param variation Variation object to be converted.
 * \param config Config object for the SUT.
 * \param buffer Pointer to the buffer the arguments string will be copied into.
 * \param size Size of the buffer.
 *
 * \return 1 on success, 0 on failure
 */
int SDLVisualTest_MakeStrFromVariation(SDLVisualTest_Variation* variation,
                                       SDLVisualTest_SUTConfig* config,
                                       char* buffer, int size);

/**
 * Initializes the variation using the following rules:
 * - Boolean options are initialized to SDL_FALSE.
 * - Integer options are initialized to the minimum valid value they can hold.
 * - Enum options are initialized to the first element in the list of values they
 *   can take.
 * - String options are initialized to the name of the option.
 *
 * \return 1 on success, 0 on failure.
 */
int SDLVisualTest_InitVariation(SDLVisualTest_Variation* variation,
                                SDLVisualTest_SUTConfig* config);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif /* SDL_visualtest_variator_common_h_ */

/* vi: set ts=4 sw=4 expandtab: */
