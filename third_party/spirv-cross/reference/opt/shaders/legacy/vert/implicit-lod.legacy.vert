#version 100

uniform mediump sampler2D tex;

void main()
{
    gl_Position = texture2D(tex, vec2(0.4000000059604644775390625, 0.60000002384185791015625));
}

