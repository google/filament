#version 450
#extension GL_KHR_shader_subgroup_vote: enable
#extension GL_EXT_shader_quad_control: enable

layout(quad_derivatives) in;
layout(location = 0) in vec2 inCoords;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D tex1;
layout(binding = 1) uniform sampler2D tex2;

void main()
{
    bool condition = gl_FragCoord.y < 10.0;
    if (subgroupQuadAll(condition))
        outColor = texture(tex1, inCoords);
    else if (subgroupQuadAny(condition))
        outColor = texture(tex2, inCoords);
    else
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
}