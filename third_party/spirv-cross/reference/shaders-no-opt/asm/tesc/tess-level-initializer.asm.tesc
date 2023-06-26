#version 450
layout(vertices = 4) out;

const float _5_init[2] = float[](0.0, 0.0);
const float _6_init[4] = float[](0.0, 0.0, 0.0, 0.0);
void main()
{
    if (gl_InvocationID == 0)
    {
        gl_TessLevelInner = _5_init;
    }
    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter = _6_init;
    }
    gl_out[gl_InvocationID].gl_Position = vec4(1.0);
    gl_TessLevelInner[0] = 1.0;
    gl_TessLevelInner[1] = 2.0;
    gl_TessLevelOuter[0] = 3.0;
    gl_TessLevelOuter[1] = 4.0;
    gl_TessLevelOuter[2] = 5.0;
    gl_TessLevelOuter[3] = 6.0;
}

