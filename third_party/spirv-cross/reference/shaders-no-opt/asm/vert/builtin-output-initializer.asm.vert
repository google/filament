#version 450

out float gl_ClipDistance[1];
out float gl_CullDistance[1];

void main()
{
    gl_Position = vec4(0.0);
    gl_PointSize = 0.0;
    gl_ClipDistance = float[](0.0);
    gl_CullDistance = float[](0.0);
    gl_Position = vec4(1.0);
}

