#version 310 es
precision mediump float;
precision highp int;

layout(binding = 0) uniform mediump sampler2D samp;

layout(location = 0) out vec4 FragColor;
layout(location = 2) in vec2 vUV;
layout(location = 1) in vec3 vNormal;

void main()
{
    vec4 _19 = texture(samp, vUV);
    float _23 = _19.x;
    FragColor = vec4(_23, _19.yz, 1.0);
    FragColor = vec4(_23, _19.z, 1.0, 4.0);
    FragColor = vec4(_23, _23, texture(samp, vUV + vec2(0.100000001490116119384765625)).yy);
    FragColor = vec4(vNormal, 1.0);
    FragColor = vec4(vNormal + vec3(1.7999999523162841796875), 1.0);
    FragColor = vec4(vUV, vUV + vec2(1.7999999523162841796875));
}

