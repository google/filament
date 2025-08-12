#version 450

// Should generate a contradictory depth layout error
layout (depth_less) out float gl_FragDepth;

layout(location=0) out vec4 FragColor;

void outputColor(vec4 color)
{
  FragColor = color;
}
