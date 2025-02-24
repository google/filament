#version 310 es

layout(location=0) out mediump vec4 fullDimensionOutColor;
layout(location=1) out mediump vec4 halfDimensionOutColor;

void main()
{
    halfDimensionOutColor = fullDimensionOutColor = vec4(0.35, 0.0, 0.5, 1.0);
}