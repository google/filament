/* See LICENSE.txt for the full license governing this code. */
/**
 * \file SDL_visualtest_sut_configparser.h
 *
 * Header for the parser for SUT config files.
 */

#ifndef SDL_visualtest_sut_configparser_h_
#define SDL_visualtest_sut_configparser_h_

/** Maximum length of the name of an SUT option */
#define MAX_SUTOPTION_NAME_LEN 100
/** Maximum length of the name of a category of an SUT option */
#define MAX_SUTOPTION_CATEGORY_LEN 40
/** Maximum length of one enum value of an SUT option */
#define MAX_SUTOPTION_ENUMVAL_LEN 40
/** Maximum length of a line in the paramters file */
#define MAX_SUTOPTION_LINE_LENGTH 256

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Describes the different kinds of options to the SUT.
 */
typedef enum {
    SDL_SUT_OPTIONTYPE_STRING = 0,
    SDL_SUT_OPTIONTYPE_INT,
    SDL_SUT_OPTIONTYPE_ENUM,
    SDL_SUT_OPTIONTYPE_BOOL
} SDLVisualTest_SUTOptionType;

/**
 * Represents the range of values an integer option can take.
 */
typedef struct SDLVisualTest_SUTIntRange {
    /*! Minimum value of the integer option */
    int min;
    /*! Maximum value of the integer option */
    int max;
} SDLVisualTest_SUTIntRange;

/**
 * Struct that defines an option to be passed to the SUT.
 */
typedef struct SDLVisualTest_SUTOption {
    /*! The name of the option. This is what you would pass in the command line
        along with two leading hyphens. */
    char name[MAX_SUTOPTION_NAME_LEN];
    /*! An array of categories that the option belongs to. The last element is
        NULL. */
    char** categories;
    /*! Type of the option - integer, boolean, etc. */
    SDLVisualTest_SUTOptionType type;
    /*! Whether the option is required or not */
    SDL_bool required;
    /*! extra data that is required for certain types */
    union {
        /*! This field is valid only for integer type options; it defines the
        valid range for such an option */
        SDLVisualTest_SUTIntRange range;
        /*! This field is valid only for enum type options; it holds the list of values
        that the option can take. The last element is NULL */
        char** enum_values;
    } data;
} SDLVisualTest_SUTOption;

/**
 * Struct to hold all the options to an SUT application.
 */
typedef struct SDLVisualTest_SUTConfig
{
    /*! Pointer to an array of options */
    SDLVisualTest_SUTOption* options;
    /*! Number of options in \c options */
    int num_options;
} SDLVisualTest_SUTConfig;

/**
 * Parses a configuration file that describes the command line options an SUT
 * application will take and populates a SUT config object. All lines in the
 * config file must be smaller than 
 *
 * \param file Path to the configuration file.
 * \param config Pointer to an object that represents an SUT configuration.
 *
 * \return zero on failure, non-zero on success
 */
int SDLVisualTest_ParseSUTConfig(char* file, SDLVisualTest_SUTConfig* config);

/**
 * Free any resources associated with the config object pointed to by \c config.
 */
void SDLVisualTest_FreeSUTConfig(SDLVisualTest_SUTConfig* config);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif /* SDL_visualtest_sut_configparser_h_ */

/* vi: set ts=4 sw=4 expandtab: */
