#version 430 core
#extension GL_ARB_enhanced_layouts : require

layout(isolines, point_mode) in;

layout (location = 1, component = 0)   in vec2 gohan[];
layout (location = 1, component = 2) patch  in vec2 goten;

in  vec4 tcs_tes[];
out vec4 tes_gs;

void main()
{
    vec4 result = tcs_tes[0];



    tes_gs += result;
}
