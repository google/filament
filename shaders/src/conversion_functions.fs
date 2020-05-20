//------------------------------------------------------------------------------
// Conversion functions configuration
//------------------------------------------------------------------------------

// Defines the set of opto-electronic and electro-optical conversion functions
#define CONVERSION_FUNCTION_LINEAR     0
#define CONVERSION_FUNCTION_sRGB       1
#define CONVERSION_FUNCTION_sRGB_FAST  2

#if TONE_MAPPING_OPERATOR < TONE_MAPPING_LINEAR
    #define CONVERSION_FUNCTION        CONVERSION_FUNCTION_LINEAR
#else
    #ifdef TARGET_MOBILE
    #define CONVERSION_FUNCTION        CONVERSION_FUNCTION_sRGB_FAST
    #else
    #define CONVERSION_FUNCTION        CONVERSION_FUNCTION_sRGB
    #endif
#endif

//------------------------------------------------------------------------------
// Opto-electronic conversion functions (linear to non-linear)
//------------------------------------------------------------------------------

float OECF_sRGB(const float linear) {
    // IEC 61966-2-1:1999
    float sRGBLow  = linear * 12.92;
    float sRGBHigh = (pow(linear, 1.0 / 2.4) * 1.055) - 0.055;
    return linear <= 0.0031308 ? sRGBLow : sRGBHigh;
}

vec3 OECF_sRGB(const vec3 linear) {
    return vec3(OECF_sRGB(linear.r), OECF_sRGB(linear.g), OECF_sRGB(linear.b));
}

vec3 OECF_sRGBFast(const vec3 linear) {
    return pow(linear, vec3(1.0 / 2.2));
}

//------------------------------------------------------------------------------
// Conversion functions
//------------------------------------------------------------------------------

/**
 * Applies the opto-electronic conversion function to the specified LDR RGB
 * linear color and outputs an LDR RGB non-linear color in sRGB space.
 */
vec3 OECF(const vec3 linear) {
#if CONVERSION_FUNCTION == CONVERSION_FUNCTION_LINEAR
    return linear;
#elif CONVERSION_FUNCTION == CONVERSION_FUNCTION_sRGB
    return OECF_sRGB(linear);
#elif CONVERSION_FUNCTION == CONVERSION_FUNCTION_sRGB_FAST
    return OECF_sRGBFast(linear);
#endif
}
