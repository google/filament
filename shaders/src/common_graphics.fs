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
         1.0000,  1.0000,  1.0000,  0.0000,
         0.0000, -0.3441,  1.7720,  0.0000,
         1.4020, -0.7141,  0.0000,  0.0000,
        -0.7010,  0.5291, -0.8860,  1.0000
    );
    return (ycbcrToRgbTransform * vec4(luminance, cbcr, 1.0)).rgb;
}

//------------------------------------------------------------------------------
// Tone mapping operations
//------------------------------------------------------------------------------

/*
 * The input must be in the [0, 1] range.
 */
vec3 Inverse_Tonemap_Filmic(const vec3 x) {
    return (0.03 - 0.59 * x - sqrt(0.0009 + 1.3702 * x - 1.0127 * x * x)) / (-5.02 + 4.86 * x);
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
    return Inverse_Tonemap_Filmic(pow(color, vec3(2.2)));
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
    return Inverse_Tonemap_Filmic(clamp(linear, 0.0, 1.0));
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

#if MATERIAL_FEATURE_LEVEL > 0

// PowerVR Vulkan drivers have broken textureGather support, so we fall back to textureLodOffset or
// texelFetch using the CONFIG_POWER_VR_SHADER_WORKAROUNDS specialization constant.

#if !defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER) || defined(TARGET_VULKAN_ENVIRONMENT)
// Emulates textureGather with four textureLodOffset samples. This is only equivalent to
// textureGather when uv is already shifted by -0.5 texel and the sampler uses NEAREST filtering.
vec4 filament_textureLodOffsetGather(const mediump sampler2D tex, const highp vec2 uv,
        const int comp) {
    vec4 d;
    d[0] = textureLodOffset(tex, uv, 0.0, ivec2(0, 1))[comp];
    d[1] = textureLodOffset(tex, uv, 0.0, ivec2(1, 1))[comp];
    d[2] = textureLodOffset(tex, uv, 0.0, ivec2(1, 0))[comp];
    d[3] = textureLodOffset(tex, uv, 0.0, ivec2(0, 0))[comp];
    return d;
}

// Returns the texel coordinates of the 2x2 footprint used by textureGather, in the order
// 01, 11, 10, 00, clamped to the texture bounds to match CLAMP_TO_EDGE addressing.
void filament_gatherTexelCoords(const highp vec2 uv, const highp ivec2 size,
        out highp ivec2 p01, out highp ivec2 p11, out highp ivec2 p10, out highp ivec2 p00) {
    highp ivec2 i = ivec2(floor(uv * vec2(size) - 0.5));
    highp ivec2 hi = size - 1;
    p01 = clamp(i + ivec2(0, 1), ivec2(0), hi);
    p11 = clamp(i + ivec2(1, 1), ivec2(0), hi);
    p10 = clamp(i + ivec2(1, 0), ivec2(0), hi);
    p00 = clamp(i, ivec2(0), hi);
}

vec4 filament_texelFetchGather(const mediump sampler2D tex, const highp vec2 uv,
        const highp vec2 texSize, const int comp) {
    highp ivec2 p01, p11, p10, p00;
    filament_gatherTexelCoords(uv, ivec2(texSize), p01, p11, p10, p00);
    vec4 d;
    d[0] = texelFetch(tex, p01, 0)[comp];
    d[1] = texelFetch(tex, p11, 0)[comp];
    d[2] = texelFetch(tex, p10, 0)[comp];
    d[3] = texelFetch(tex, p00, 0)[comp];
    return d;
}

void filament_texelFetchGatherRG(const mediump sampler2D tex, const highp vec2 uv,
        out vec4 r, out vec4 g) {
    highp ivec2 p01, p11, p10, p00;
    filament_gatherTexelCoords(uv, textureSize(tex, 0), p01, p11, p10, p00);
    vec2 s01 = texelFetch(tex, p01, 0).rg;
    vec2 s11 = texelFetch(tex, p11, 0).rg;
    vec2 s10 = texelFetch(tex, p10, 0).rg;
    vec2 s00 = texelFetch(tex, p00, 0).rg;
    r = vec4(s01.r, s11.r, s10.r, s00.r);
    g = vec4(s01.g, s11.g, s10.g, s00.g);
}

void filament_texelFetchGatherRGB(const mediump sampler2DArray tex, const highp vec3 uv,
        out vec4 r, out vec4 g, out vec4 b) {
    highp ivec2 p01, p11, p10, p00;
    filament_gatherTexelCoords(uv.xy, textureSize(tex, 0).xy, p01, p11, p10, p00);
    int layer = int(uv.z);
    vec3 s01 = texelFetch(tex, ivec3(p01, layer), 0).rgb;
    vec3 s11 = texelFetch(tex, ivec3(p11, layer), 0).rgb;
    vec3 s10 = texelFetch(tex, ivec3(p10, layer), 0).rgb;
    vec3 s00 = texelFetch(tex, ivec3(p00, layer), 0).rgb;
    r = vec4(s01.r, s11.r, s10.r, s00.r);
    g = vec4(s01.g, s11.g, s10.g, s00.g);
    b = vec4(s01.b, s11.b, s10.b, s00.b);
}
#endif // !defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER) || defined(TARGET_VULKAN_ENVIRONMENT)

#if defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER)
vec4 filament_textureGather(const mediump sampler2D tex, const highp vec2 uv, const int comp) {
    vec4 result;
    switch (comp) {
        case 0:
            result = textureGather(tex, uv, 0);
            break;
        case 1:
            result = textureGather(tex, uv, 1);
            break;
        case 2:
            result = textureGather(tex, uv, 2);
            break;
        default:
            result = textureGather(tex, uv, 3);
            break;
    }
    return result;
}
#endif // defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER)

vec4 filament_textureGatherLodOffset(const mediump sampler2D tex, const highp vec2 uv,
        const int comp) {
    vec4 result;
#if defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER)
#   if defined(TARGET_VULKAN_ENVIRONMENT)
    if (CONFIG_POWER_VR_SHADER_WORKAROUNDS) {
        result = filament_textureLodOffsetGather(tex, uv, comp);
    } else {
        result = filament_textureGather(tex, uv, comp);
    }
#   else
    result = filament_textureGather(tex, uv, comp);
#   endif // defined(TARGET_VULKAN_ENVIRONMENT)
#else
    result = filament_textureLodOffsetGather(tex, uv, comp);
#endif // defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER)
    return result;
}

void filament_textureGatherFetchRGB(const mediump sampler2DArray tex, const highp vec3 uv,
        out vec4 r, out vec4 g, out vec4 b) {
#if defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER)
#   if defined(TARGET_VULKAN_ENVIRONMENT)
    if (CONFIG_POWER_VR_SHADER_WORKAROUNDS) {
        filament_texelFetchGatherRGB(tex, uv, r, g, b);
    } else {
        r = textureGather(tex, uv, 0);
        g = textureGather(tex, uv, 1);
        b = textureGather(tex, uv, 2);
    }
#   else
    r = textureGather(tex, uv, 0);
    g = textureGather(tex, uv, 1);
    b = textureGather(tex, uv, 2);
#   endif // defined(TARGET_VULKAN_ENVIRONMENT)
#else
    filament_texelFetchGatherRGB(tex, uv, r, g, b);
#endif // defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER)
}

vec4 filament_textureGatherFetch(const mediump sampler2D tex, const highp vec2 uv,
        const highp vec2 texSize, const int comp) {
    vec4 result;
#if defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER)
#   if defined(TARGET_VULKAN_ENVIRONMENT)
    if (CONFIG_POWER_VR_SHADER_WORKAROUNDS) {
        result = filament_texelFetchGather(tex, uv, texSize, comp);
    } else {
        result = filament_textureGather(tex, uv, comp);
    }
#   else
    result = filament_textureGather(tex, uv, comp);
#   endif // defined(TARGET_VULKAN_ENVIRONMENT)
#else
    result = filament_texelFetchGather(tex, uv, texSize, comp);
#endif // defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER)
    return result;
}

vec4 filament_textureGatherFetch(const mediump sampler2D tex, const highp vec2 uv,
        const int comp) {
    vec4 result;
#if defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER)
#   if defined(TARGET_VULKAN_ENVIRONMENT)
    if (CONFIG_POWER_VR_SHADER_WORKAROUNDS) {
        result = filament_texelFetchGather(tex, uv, vec2(textureSize(tex, 0)), comp);
    } else {
        result = filament_textureGather(tex, uv, comp);
    }
#   else
    result = filament_textureGather(tex, uv, comp);
#   endif // defined(TARGET_VULKAN_ENVIRONMENT)
#else
    result = filament_texelFetchGather(tex, uv, vec2(textureSize(tex, 0)), comp);
#endif // defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER)
    return result;
}

void filament_textureGatherFetchRG(const mediump sampler2D tex, const highp vec2 uv,
        out vec4 r, out vec4 g) {
#if defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER)
#   if defined(TARGET_VULKAN_ENVIRONMENT)
    if (CONFIG_POWER_VR_SHADER_WORKAROUNDS) {
        filament_texelFetchGatherRG(tex, uv, r, g);
    } else {
        r = textureGather(tex, uv, 0);
        g = textureGather(tex, uv, 1);
    }
#   else
    r = textureGather(tex, uv, 0);
    g = textureGather(tex, uv, 1);
#   endif // defined(TARGET_VULKAN_ENVIRONMENT)
#else
    filament_texelFetchGatherRG(tex, uv, r, g);
#endif // defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER)
}

#endif // MATERIAL_FEATURE_LEVEL > 0

//------------------------------------------------------------------------------
// Common screen-space operations
//------------------------------------------------------------------------------

// returns the frag coord in the GL convention with (0, 0) at the bottom-left
// resolution : width, height
highp vec2 getFragCoord(const highp vec2 resolution) {
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT) || defined(TARGET_WEBGPU_ENVIRONMENT)
    return vec2(gl_FragCoord.x, resolution.y - gl_FragCoord.y);
#else
    return gl_FragCoord.xy;
#endif // defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT) || defined(TARGET_WEBGPU_ENVIRONMENT)
}

//------------------------------------------------------------------------------
// Common debug
//------------------------------------------------------------------------------

vec3 heatmap(float v) {
    vec3 r = v * 2.1 - vec3(1.8, 1.14, 0.3);
    return 1.0 - r * r;
}
