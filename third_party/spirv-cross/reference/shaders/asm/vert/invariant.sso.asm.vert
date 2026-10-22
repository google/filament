#version 450

out gl_PerVertex
{
    invariant vec4 gl_Position;
};

vec4 _main()
{
    return vec4(1.0);
}

void main()
{
    vec4 _17 = _main();
    gl_Position = _17;
}

