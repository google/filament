#version 450

out gl_PerVertex
{
    vec4 gl_Position;
};

invariant gl_Position;

void main()
{
    gl_Position = vec4(1.0);
}

