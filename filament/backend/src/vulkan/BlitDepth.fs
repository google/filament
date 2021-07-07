#version 320 es
precision mediump float;

precision lowp sampler2DMS;

layout(set = 0, binding = 0, std140) uniform ParamsBlock {
    int sampleCount;
    float inverseSampleCount;
} params;

layout(set = 1, binding = 0) uniform sampler2DMS tex;

void main() {
    float depth = 0.0;
    for (int sampleIndex = 0; sampleIndex < params.sampleCount; sampleIndex++) {
        depth += texelFetch(tex, ivec2(gl_FragCoord.xy), sampleIndex).r;
    }
    gl_FragDepth = depth * params.inverseSampleCount;
}
