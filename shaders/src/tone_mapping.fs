//------------------------------------------------------------------------------
// Tone-mapping configuration
//------------------------------------------------------------------------------

// Operators for LDR output

// Operators with built-in sRGB
#define TONE_MAPPING_UNREAL           0
#define TONE_MAPPING_FILMIC_ALU       1
#define TONE_MAPPING_LUT              2

// Operators without built-in sRGB
#define TONE_MAPPING_LINEAR           3
#define TONE_MAPPING_REINHARD         4
#define TONE_MAPPING_ACES             5

// Operators for HDR output
#define TONE_MAPPING_ACES_REC2020_1K  6

// Debug operators
#define TONE_MAPPING_DISPLAY_RANGE    9

#ifdef TARGET_MOBILE
    #define TONE_MAPPING_OPERATOR     TONE_MAPPING_LUT
#else
    #define TONE_MAPPING_OPERATOR     TONE_MAPPING_LUT
#endif

//------------------------------------------------------------------------------
// Tone-mapping operators for LDR output
//------------------------------------------------------------------------------

vec3 Tonemap_Linear(const vec3 x) {
    return x;
}

vec3 Tonemap_Reinhard(const vec3 x) {
    // Reinhard et al. 2002, "Photographic Tone Reproduction for Digital Images", Eq. 3
    return x / (1.0 + luminance(x));
}

vec3 Tonemap_Unreal(const vec3 x) {
    // Unreal, Documentation: "Color Grading"
    // Adapted to be close to Tonemap_ACES, with similar range
    // Gamma 2.2 correction is baked in, don't use with sRGB conversion!
    return x / (x + 0.155) * 1.019;
}

vec3 Tonemap_FilmicALU(const vec3 x) {
    // Hable 2010, "Filmic Tonemapping Operators"
    // Based on Duiker's curve, optimized by Hejl and Burgess-Dawson
    // Gamma 2.2 correction is baked in, don't use with sRGB conversion!
    vec3 c = max(vec3(0.0), x - 0.004);
    return (c * (c * 6.2 + 0.5)) / (c * (c * 6.2 + 1.7) + 0.06);
}

vec3 Tonemap_ACES(const vec3 x) {
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

vec3 Tonemap_LUT(mediump sampler3D lut, const vec3 x) {
    // Alexa LogC EI 1000
    const float a = 5.555556;
    const float b = 0.047996;
    const float c = 0.244161 / log2(10.0);
    const float d = 0.386036;
    vec3 logc = c * log2(a * x + b) + d;
    return textureLod(lut, logc, 0.0).rgb;
}

//------------------------------------------------------------------------------
// Tone-mapping operators for HDR output
//------------------------------------------------------------------------------

#if TONE_MAPPING_OPERATOR == TONE_MAPPING_ACES_REC2020_1K
vec3 Tonemap_ACES_Rec2020_1k(const vec3 x) {
    // Narkowicz 2016, "HDR Display – First Steps"
    const float a = 15.8;
    const float b = 2.12;
    const float c = 1.2;
    const float d = 5.92;
    const float e = 1.9;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}
#endif

//------------------------------------------------------------------------------
// Debug tone-mapping operators, for LDR output
//------------------------------------------------------------------------------

/**
 * Converts the input HDR RGB color into one of 16 debug colors that represent
 * the pixel's exposure. When the output is cyan, the input color represents
 * middle gray (18% exposure). Every exposure stop above or below middle gray
 * causes a color shift.
 *
 * The relationship between exposures and colors is:
 *
 * -5EV  - black
 * -4EV  - darkest blue
 * -3EV  - darker blue
 * -2EV  - dark blue
 * -1EV  - blue
 *  OEV  - cyan
 * +1EV  - dark green
 * +2EV  - green
 * +3EV  - yellow
 * +4EV  - yellow-orange
 * +5EV  - orange
 * +6EV  - bright red
 * +7EV  - red
 * +8EV  - magenta
 * +9EV  - purple
 * +10EV - white
 */
#if TONE_MAPPING_OPERATOR == TONE_MAPPING_DISPLAY_RANGE
vec3 Tonemap_DisplayRange(const vec3 x) {
    // 16 debug colors + 1 duplicated at the end for easy indexing
    const vec3 debugColors[17] = vec3[](
         vec3(0.0, 0.0, 0.0),         // black
         vec3(0.0, 0.0, 0.1647),      // darkest blue
         vec3(0.0, 0.0, 0.3647),      // darker blue
         vec3(0.0, 0.0, 0.6647),      // dark blue
         vec3(0.0, 0.0, 0.9647),      // blue
         vec3(0.0, 0.9255, 0.9255),   // cyan
         vec3(0.0, 0.5647, 0.0),      // dark green
         vec3(0.0, 0.7843, 0.0),      // green
         vec3(1.0, 1.0, 0.0),         // yellow
         vec3(0.90588, 0.75294, 0.0), // yellow-orange
         vec3(1.0, 0.5647, 0.0),      // orange
         vec3(1.0, 0.0, 0.0),         // bright red
         vec3(0.8392, 0.0, 0.0),      // red
         vec3(1.0, 0.0, 1.0),         // magenta
         vec3(0.6, 0.3333, 0.7882),   // purple
         vec3(1.0, 1.0, 1.0),         // white
         vec3(1.0, 1.0, 1.0)          // white
    );

    // The 5th color in the array (cyan) represents middle gray (18%)
    // Every stop above or below middle gray causes a color shift
    float v = log2(luminance(x) / 0.18);
    v = clamp(v + 5.0, 0.0, 15.0);
    int index = int(v);
    return mix(debugColors[index], debugColors[index + 1], v - float(index));
}
#endif

//------------------------------------------------------------------------------
// Tone-mapping dispatch
//------------------------------------------------------------------------------

/**
 * Tone-maps the specified RGB color. The input color must be in linear HDR and
 * pre-exposed. Our HDR to LDR tone mapping operators are designed to tone-map
 * the range [0..~8] to [0..1].
 */
vec3 tonemap(mediump sampler3D lut, const vec3 x) {
#if TONE_MAPPING_OPERATOR == TONE_MAPPING_UNREAL
    return Tonemap_Unreal(x);
#elif TONE_MAPPING_OPERATOR == TONE_MAPPING_FILMIC_ALU
    return Tonemap_FilmicALU(x);
#elif TONE_MAPPING_OPERATOR == TONE_MAPPING_LINEAR
    return Tonemap_Linear(x);
#elif TONE_MAPPING_OPERATOR == TONE_MAPPING_REINHARD
    return Tonemap_Reinhard(x);
#elif TONE_MAPPING_OPERATOR == TONE_MAPPING_ACES
    return Tonemap_ACES(x);
#elif TONE_MAPPING_OPERATOR == TONE_MAPPING_ACES_REC2020_1K
    return Tonemap_ACES_Rec2020_1k(x);
#elif TONE_MAPPING_OPERATOR == TONE_MAPPING_DISPLAY_RANGE
    return Tonemap_DisplayRange(x);
#elif TONE_MAPPING_OPERATOR == TONE_MAPPING_LUT
    return Tonemap_LUT(lut, x);
#endif
}

//------------------------------------------------------------------------------
// Processing tone-mappers
//------------------------------------------------------------------------------

vec3 Tonemap_ReinhardWeighted(const vec3 x, float weight) {
    // Weighted Reinhard tone-mapping operator designed for post-processing
    // This tone-mapping operator is invertible
    return x * (weight / (max3(x) + 1.0));
}

vec3 Tonemap_ReinhardWeighted_Invert(const vec3 x) {
    // Inverse Reinhard tone-mapping operator, designed to be used in conjunction
    // with the weighted Reinhard tone-mapping operator
    return x / (1.0 - max3(x));
}
