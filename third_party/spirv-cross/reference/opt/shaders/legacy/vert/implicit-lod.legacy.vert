#version 100

uniform mediump sampler2D tex;

void main()
{
    gl_Position = texture2DLod(tex, vec2(0.4000000059604644775390625, 0.60000002384185791015625), 0.0);
}

