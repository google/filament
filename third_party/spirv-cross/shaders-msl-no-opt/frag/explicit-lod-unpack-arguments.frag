#version 310 es

precision mediump float;

layout(set = 2, binding = 0, std140) uniform MaterialParams {
    float myLod[6];
} materialParams;

layout(binding = 1, set = 2) uniform  sampler2D mySampler;

layout(location = 0) out vec4 fragColor;

void main() {
   fragColor = textureLod(mySampler, vec2(0.0), materialParams.myLod[0]);
}
