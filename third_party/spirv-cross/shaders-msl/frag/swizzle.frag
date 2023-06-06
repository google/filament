#version 310 es
precision mediump float;

layout(binding = 0) uniform sampler2D samp;
layout(location = 0) out vec4 FragColor;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUV;

void main()
{
    FragColor = vec4(texture(samp, vUV).xyz, 1.0);
    FragColor = vec4(texture(samp, vUV).xz, 1.0, 4.0);
    FragColor = vec4(texture(samp, vUV).xx, texture(samp, vUV + vec2(0.1)).yy);
    FragColor = vec4(vNormal, 1.0);
    FragColor = vec4(vNormal + 1.8, 1.0);
    FragColor = vec4(vUV, vUV + 1.8);
}
