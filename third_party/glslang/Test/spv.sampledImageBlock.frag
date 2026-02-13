#version 450

layout(set = 0, binding = 0) uniform texture2D tex0;
layout(set = 0, binding = 1) uniform sampler samp0;
layout(set = 0, binding = 2) uniform ParamBuffer {
    int cond;
} paramBuffer;

layout(location = 0) out vec4 fragColor;
layout(location = 0) in flat ivec2 texCoord;

void main() {
    // get input

    const vec4 texel = texelFetch(sampler2D(tex0, samp0),
            paramBuffer.cond == 0 ? texCoord.xy : texCoord.yx,
            0);

    fragColor = vec4(texel.xyz, 1.0);

}
