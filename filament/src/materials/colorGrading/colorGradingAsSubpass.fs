#include "../../../../shaders/src/inline_dithering.fs"
#include "../../../../shaders/src/inline_vignette.fs"
#include "colorGrading.fs"

void dummy() {}

void postProcess(inout PostProcessInputs postProcess) {
    highp vec2 uv = variable_vertex.xy;
    vec4 color = resolveFragment();

    if (materialParams.vignette.x < MEDIUMP_FLT_MAX) {
        color.rgb = vignette(color.rgb, uv, materialParams.vignette, materialParams.vignetteColor);
    }

    color.rgb = colorGrade(materialParams_lut, color.rgb);

#if !POST_PROCESS_OPAQUE
    color.rgb *= color.a + FLT_EPS;
#endif

    if (materialParams.dithering > 0) {
        color = dither(color, materialParams.temporalNoise);
    }

#if POST_PROCESS_OPAQUE
    color.a = 1.0;
    if (materialParams.outputLuminance > 0) {
        color.a = luminance(color.rgb);
    }
#endif

    postProcess.tonemappedOutput = color;
}
