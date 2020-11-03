#version 320 es
precision mediump float;

layout(set = 0, binding = 0, std140) uniform ParamsBlock {
    float level;
} params;

uniform sampler2D tex;

layout(location = 0) out highp vec4 color;

void main() {
    color = texelFetch(tex, ivec2(gl_FragCoord.xy), int(params.level));
}
