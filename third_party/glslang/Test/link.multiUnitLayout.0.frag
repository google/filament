#version 450

layout (depth_greater) out float gl_FragDepth;

void outputColor(vec4 color);

void main()
{
  outputColor(vec4(1.0, 0.0, 0.0, 1.0));
  gl_FragDepth = 0.5;
}
