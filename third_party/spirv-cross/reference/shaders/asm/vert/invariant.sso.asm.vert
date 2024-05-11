#version 450

out gl_PerVertex
{
    vec4 gl_Position;
};

invariant gl_Position;

vec4 _main()
{
    return vec4(1.0);
}

void main()
{
    vec4 _14 = _main();
    gl_Position = _14;
}

