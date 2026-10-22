#version 450

out gl_PerVertex
{
    invariant vec4 gl_Position;
};

void main()
{
    gl_Position = vec4(1.0);
}

