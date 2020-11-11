#version 100

uniform mediump sampler2D tex;

varying mediump vec4 FragColor;

void main()
{
    FragColor = texture2DLod(tex, vec2(0.4000000059604644775390625, 0.60000002384185791015625), 3.0);
}

