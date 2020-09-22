#ifndef MATERIALS_SSAO_UTILS
#define MATERIALS_SSAO_UTILS

highp float linearizeDepth(highp float depth, highp float depthParams) {
    // Our far plane is at infinity, which causes a division by zero below, which in turn
    // causes some issues on some GPU. We workaround it by replacing "infinity" by the closest
    // value representable in  a 24 bit depth buffer.
    const float preventDiv0 = 1.0 / 16777216.0;
    return depthParams / max(depth, preventDiv0);
}

highp float sampleDepthLinear(const sampler2D depthTexture, const vec2 uv, float lod, highp float depthParams) {
    return linearizeDepth(textureLod(depthTexture, uv, lod).r, depthParams);
}

#endif // MATERIALS_SSAO_UTILS

