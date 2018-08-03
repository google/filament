#version 450

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[1];
    float gl_CullDistance[1];
};

invariant gl_Position;

void main()
{
    gl_Position = vec4(1.0);
}

