#version 310 es
precision mediump float;
precision highp int;

uniform mediump sampler2DShadow SPIRV_Cross_CombineduDepthuSampler;
uniform mediump sampler2D SPIRV_Cross_CombineduDepthuSampler1;

layout(location = 0) out float FragColor;

void main()
{
    FragColor = texture(SPIRV_Cross_CombineduDepthuSampler, vec3(vec3(1.0).xy, 1.0)) + texture(SPIRV_Cross_CombineduDepthuSampler1, vec2(1.0)).x;
}

