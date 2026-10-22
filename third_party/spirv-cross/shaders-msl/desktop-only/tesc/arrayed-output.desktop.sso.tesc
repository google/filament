#version 450

layout(vertices = 4) out;
layout(location = 0) patch out vec3 vPatch[2];
layout(location = 2) out vec3 vVertex[];
layout(location = 0) in vec3 vInput[];

void main()
{
        vVertex[gl_InvocationID] =
                vInput[gl_InvocationID] +
                vInput[gl_InvocationID ^ 1];

        barrier();

        if (gl_InvocationID == 0)
        {
                vPatch[0] = vec3(10.0);
                vPatch[1] = vec3(20.0);
                gl_TessLevelOuter[0] = 1.0;
                gl_TessLevelOuter[1] = 2.0;
                gl_TessLevelOuter[2] = 3.0;
                gl_TessLevelOuter[3] = 4.0;
                gl_TessLevelInner[0] = 1.0;
                gl_TessLevelInner[1] = 2.0;
        }
}
