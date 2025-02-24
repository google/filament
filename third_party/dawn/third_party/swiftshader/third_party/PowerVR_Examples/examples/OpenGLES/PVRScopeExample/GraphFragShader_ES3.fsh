#version 300 es
uniform mediump vec4 fColor;
layout(location = 0) out mediump vec4 oColor;
void main()
{
    oColor = vec4(fColor.r, fColor.g, fColor.b, fColor.a);
}
