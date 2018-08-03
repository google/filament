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
    return dither(color);
}
#endif

#if POST_PROCESS_ANTI_ALIASING
vec4 PostProcess_AntiAliasing() {
    vec4 resolution = frameUniforms.resolution;
    HIGHP vec2 texelCenter = vertex_uv;
    vec2 halfResolutionFraction = resolution.zw * 0.5;

    vec4 color = fxaa(
            texelCenter,
            vec4(texelCenter - halfResolutionFraction, texelCenter + halfResolutionFraction),
            postProcess_colorBuffer,
            resolution.zw,       // FxaaFloat4 fxaaConsoleRcpFrameOpt,
            2.0 * resolution.zw, // FxaaFloat4 fxaaConsoleRcpFrameOpt2,
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
