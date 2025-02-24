#version 310 es

uniform mediump sampler2D tex;

in mediump vec2 TexCoord;

layout(location = 0) out mediump vec4 oColor;

void main()
{
    oColor = textureLod(tex, TexCoord, 1.0);
}
