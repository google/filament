#version 450

layout(points) in;
layout(triangle_strip, max_vertices = 9) out;

layout(location = 0) in vec4 vPositionIn[1];
layout(location = 1) in vec3 vColorIn[1];

layout(location = 0) out vec3 gColor;

vec4 somePureFunction(vec4 data)
{
  // Making sure we don't add arguments to all functions
  return data.wzxy;
}

void emitPoint(vec4 point, vec3 color)
{
  gColor = color;
  gl_Position = point;
  EmitVertex();
}

void emitTriangle(vec4 center, vec3 color)
{
    emitPoint(center + vec4(-0.1, -0.1, 0.0, 0.0), color);

    emitPoint(center + vec4(0.1, -0.1, 0.0, 0.0), color.bgr);

    emitPoint(center + vec4(0.0, 0.1, 0.0, 0.0), color.grb);

    EndPrimitive();
}

void functionThatIndirectlyEmits()
{
  emitTriangle(vec4(1,2,3,4), vec3(1,0,1));
}

void main() 
{
  vec3 color = vColorIn[0];

  emitTriangle(vPositionIn[0], color);
  emitTriangle(somePureFunction(vPositionIn[0]), color);

  functionThatIndirectlyEmits();
}