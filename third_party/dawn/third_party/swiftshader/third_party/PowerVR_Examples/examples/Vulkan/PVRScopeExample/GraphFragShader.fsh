#version 320 es

#define NUM_COLORS 10

layout(std140, set = 0, binding = 0)uniform Color
{
   mediump vec4 fColor;
};

layout(location = 0) out mediump vec4 oColor;

void main()
{
    oColor = vec4(fColor.r, fColor.g, fColor.b, fColor.a);
}
