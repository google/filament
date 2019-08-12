//------------------------------------------------------------------------------
// Dithering configuration
//------------------------------------------------------------------------------

// Dithering operators
#define DITHERING_NONE                 0
#define DITHERING_INTERLEAVED_NOISE    1
#define DITHERING_VLACHOS              2
#define DITHERING_TRIANGLE_NOISE       3
#define DITHERING_TRIANGLE_NOISE_RGB   4

// Workaround Adreno bug #1096 by using VLACHOS for Vulkan.
#if defined(TARGET_MOBILE) && !defined(TARGET_VULKAN_ENVIRONMENT)
    #define DITHERING_OPERATOR         DITHERING_INTERLEAVED_NOISE
#else
    #define DITHERING_OPERATOR         DITHERING_VLACHOS
#endif

//------------------------------------------------------------------------------
// Noise
//------------------------------------------------------------------------------

float triangleNoise(highp vec2 n) {
    // triangle noise, in [-1.0..1.0[ range
    n += vec2(0.07 * fract(postProcessUniforms.time));
    n  = fract(n * vec2(5.3987, 5.4421));
    n += dot(n.yx, n.xy + vec2(21.5351, 14.3137));

    highp float xy = n.x * n.y;
    // compute in [0..2[ and remap to [-1.0..1.0[
    return fract(xy * 95.4307) + fract(xy * 75.04961) - 1.0;
}

float interleavedGradientNoise(const highp vec2 n) {
    return fract(52.982919 * fract(dot(vec2(0.06711, 0.00584), n)));
}

//------------------------------------------------------------------------------
// Dithering
//------------------------------------------------------------------------------

vec4 Dither_InterleavedGradientNoise(vec4 rgba) {
    // Jimenez 2014, "Next Generation Post-Processing in Call of Duty"
    float noise = interleavedGradientNoise(gl_FragCoord.xy + postProcessUniforms.time);
    // remap from [0..1[ to [-1..1[
    noise = (noise * 2.0) - 1.0;
    return vec4(rgba.rgb + noise / 255.0, rgba.a);
}

vec4 Dither_Vlachos(vec4 rgba) {
    // Vlachos 2016, "Advanced VR Rendering"
    highp vec3 noise = vec3(dot(vec2(171.0, 231.0), gl_FragCoord.xy + postProcessUniforms.time));
    noise = fract(noise / vec3(103.0, 71.0, 97.0));
    // remap from [0..1[ to [-1..1[
    noise = (noise * 2.0) - 1.0;
    return vec4(rgba.rgb + (noise / 255.0), rgba.a);
}

vec4 Dither_TriangleNoise(vec4 rgba) {
    // Gjøl 2016, "Banding in Games: A Noisy Rant"
    return rgba + triangleNoise(gl_FragCoord.xy * frameUniforms.resolution.zw) / 255.0;
}

vec4 Dither_TriangleNoiseRGB(vec4 rgba) {
    // Gjøl 2016, "Banding in Games: A Noisy Rant"
    vec2 uv = gl_FragCoord.xy * frameUniforms.resolution.zw;
    vec3 dither = vec3(
            triangleNoise(uv),
            triangleNoise(uv + 0.1337),
            triangleNoise(uv + 0.3141)) / 255.0;
    return vec4(rgba.rgb + dither, rgba.a + dither.x);
}

//------------------------------------------------------------------------------
// Dithering dispatch
//------------------------------------------------------------------------------

/**
 * Dithers the specified RGB color based on the current time and fragment
 * coordinates the input must be in the final color space (including OECF).
 * This dithering function assumes we are dithering to an 8-bit target.
 * This function dithers the alpha channel assuming premultiplied output
 */
vec4 dither(vec4 rgba) {
#if DITHERING_OPERATOR == DITHERING_NONE
    return rgba;
#elif DITHERING_OPERATOR == DITHERING_INTERLEAVED_NOISE
    return Dither_InterleavedGradientNoise(rgba);
#elif DITHERING_OPERATOR == DITHERING_VLACHOS
    return Dither_Vlachos(rgba);
#elif DITHERING_OPERATOR  == DITHERING_TRIANGLE_NOISE
    return Dither_TriangleNoise(rgba);
#elif DITHERING_OPERATOR  == DITHERING_TRIANGLE_NOISE_RGB
    return Dither_TriangleNoiseRGB(rgba);
#endif
}
