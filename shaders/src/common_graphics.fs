//------------------------------------------------------------------------------
// Common color operations
//------------------------------------------------------------------------------

/**
 * Computes the luminance of the specified linear RGB color using the
 * luminance coefficients from Rec. 709.
 *
 * @public-api
 */
float luminance(const vec3 linear) {
    return dot(linear, vec3(0.2126, 0.7152, 0.0722));
}

/**
 * Computes the pre-exposed intensity using the specified intensity and exposure.
 * This function exists to force highp precision on the two parameters
 */
float computePreExposedIntensity(const highp float intensity, const highp float exposure) {
    return intensity * exposure;
}

void unpremultiply(inout vec4 color) {
    color.rgb /= max(color.a, FLT_EPS);
}

/**
 * Applies a full range YCbCr to sRGB conversion and returns an RGB color.
 *
 * @public-api
 */
vec3 ycbcrToRgb(float luminance, vec2 cbcr) {
    // Taken from https://developer.apple.com/documentation/arkit/arframe/2867984-capturedimage
    const mat4 ycbcrToRgbTransform = mat4(
        1.0000, 1.0000, 1.0000, 0.0000,
        0.0000, -0.3441, 1.7720, 0.0000,
        1.4020, -0.7141, 0.0000, 0.0000,
        -0.7010, 0.5291, -0.8860, 1.0000
    );
    return (ycbcrToRgbTransform * vec4(luminance, cbcr, 1.0)).rgb;
}

//------------------------------------------------------------------------------
// Tone mapping operations
//------------------------------------------------------------------------------

/*
 * The input must be in the [0, 1] range.
 */
vec3 Inverse_Tonemap_Unreal(const vec3 x) {
    return (x * -0.155) / (x - 1.019);
}

/**
 * Applies the inverse of the tone mapping operator to the specified HDR or LDR
 * sRGB (non-linear) color and returns a linear sRGB color. The inverse tone mapping
 * operator may be an approximation of the real inverse operation.
 *
 * @public-api
 */
vec3 inverseTonemapSRGB(vec3 color) {
    // sRGB input
    color = clamp(color, 0.0, 1.0);
    return Inverse_Tonemap_Unreal(color);
}

/**
 * Applies the inverse of the tone mapping operator to the specified HDR or LDR
 * linear RGB color and returns a linear RGB color. The inverse tone mapping operator
 * may be an approximation of the real inverse operation.
 *
 * @public-api
 */
vec3 inverseTonemap(vec3 linear) {
    // Linear input
    linear = clamp(linear, 0.0, 1.0);
    return Inverse_Tonemap_Unreal(pow(linear, vec3(1.0 / 2.2)));
}

//------------------------------------------------------------------------------
// Common texture operations
//------------------------------------------------------------------------------

/**
 * Decodes the specified RGBM value to linear HDR RGB.
 */
vec3 decodeRGBM(vec4 c) {
    c.rgb *= (c.a * 16.0);
    return c.rgb * c.rgb;
}

//------------------------------------------------------------------------------
// Common debug
//------------------------------------------------------------------------------

vec3 heatmap(float v) {
    vec3 r = v * 2.1 - vec3(1.8, 1.14, 0.3);
    return 1.0 - r * r;
}
