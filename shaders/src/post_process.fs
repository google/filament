LAYOUT_LOCATION(0) in HIGHP vec2 vertex_uv;

LAYOUT_LOCATION(0) out vec4 fragColor;

#if POST_PROCESS_TONE_MAPPING
vec3 resolveFragment(const ivec2 uv) {
    return texelFetch(postProcess_colorBuffer, uv, 0).rgb;
}

vec4 resolveAlphaFragment(const ivec2 uv) {
    return texelFetch(postProcess_colorBuffer, uv, 0);
}

vec4 resolve() {
#if POST_PROCESS_OPAQUE
    vec4 color = vec4(resolveFragment(ivec2(vertex_uv)), 1.0);
    color.rgb  = tonemap(color.rgb);
    color.rgb  = OECF(color.rgb);
    color.a    = luminance(color.rgb);
#else
    vec4 color = resolveAlphaFragment(ivec2(vertex_uv));
    color.rgb /= color.a + FLT_EPS;
    color.rgb  = tonemap(color.rgb);
    color.rgb  = OECF(color.rgb);
    color.rgb *= color.a + FLT_EPS;
#endif
    return color;
}

vec4 PostProcess_ToneMapping() {
    vec4 color = resolve();
    if (postProcessUniforms.dithering > 0) {
        color = dither(color);
    }
    return color;
}
#endif

#if POST_PROCESS_ANTI_ALIASING
vec4 PostProcess_AntiAliasing() {

    // First, compute an exact upper bound for the area we need to sample from. The render target
    // may be larger than the viewport that was used for scene rendering, so we cannot rely on the
    // wrap mode alone.
    HIGHP vec2 fboSize = vec2(textureSize(postProcess_colorBuffer, 0));
    HIGHP vec2 invSize = 1.0 / fboSize;
    HIGHP vec2 halfTexel = 0.5 * invSize;
    HIGHP vec2 viewportSize = frameUniforms.resolution.xy;

    // The clamp needs to be over-aggressive by a half-texel due to bilinear sampling.
    HIGHP vec2 excessSize = 0.5 + fboSize - viewportSize;
    HIGHP vec2 upperBound = 1.0 - excessSize * invSize;

    // Next, compute the coordinates of the texel center and its bounding box. There is no need to
    // clamp the min corner since the wrap mode will do it automatically.

    // vertex_uv is already interpolated to pixel center by the GPU
    HIGHP vec2 texelCenter = min(vertex_uv, upperBound);
    HIGHP vec2 texelMaxCorner = min(vertex_uv + halfTexel, upperBound);
    HIGHP vec2 texelMinCorner = vertex_uv - halfTexel;

    vec4 color = fxaa(
            texelCenter,
            vec4(texelMinCorner, texelMaxCorner),
            postProcess_colorBuffer,
            invSize,             // FxaaFloat4 fxaaConsoleRcpFrameOpt,
            2.0 * invSize,       // FxaaFloat4 fxaaConsoleRcpFrameOpt2,
            8.0,                 // FxaaFloat fxaaConsoleEdgeSharpness,
#if defined(G3D_FXAA_PATCHES) && G3D_FXAA_PATCHES == 1
            0.08,                // FxaaFloat fxaaConsoleEdgeThreshold,
#else
            0.125,               // FxaaFloat fxaaConsoleEdgeThreshold,
#endif
            0.04                 // FxaaFloat fxaaConsoleEdgeThresholdMin
    );
#if POST_PROCESS_OPAQUE
    color.a = 1.0;
#endif
    return color;
}
#endif

vec4 postProcess() {
#if POST_PROCESS_TONE_MAPPING
    return PostProcess_ToneMapping();
#elif POST_PROCESS_ANTI_ALIASING
    return PostProcess_AntiAliasing();
#endif
}

void main() {
    fragColor = postProcess();
}
