#version 430 core
#extension GL_ARB_enhanced_layouts : require

layout (location = 1, component = 0)  flat in uint gohan;
layout (location = 1, component = 2) sample flat in uvec2 goten;

in  vec4 gs_fs;
out vec4 fs_out;

void main()
{
    vec4 result = gs_fs;



    fs_out = result;
}
