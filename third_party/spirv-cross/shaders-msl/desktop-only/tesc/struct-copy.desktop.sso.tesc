#version 450

struct Boo
{
        vec3 a;
        vec3 b;
};

layout(vertices = 4) out;
layout(location = 0) out Boo vVertex[];
layout(location = 0) in Boo vInput[];

void main()
{
        vVertex[gl_InvocationID] = vInput[gl_InvocationID];
        gl_TessLevelOuter[0] = 1.0;
        gl_TessLevelOuter[1] = 2.0;
        gl_TessLevelOuter[2] = 3.0;
        gl_TessLevelOuter[3] = 4.0;
        gl_TessLevelInner[0] = 1.0;
        gl_TessLevelInner[1] = 2.0;
}
