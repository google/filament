#version 410

layout(triangles, equal_spacing, cw) in;

vec4 transformPosition(vec4 position);

void main()
{
  vec4 p0 = gl_TessCoord.x * gl_in[0].gl_Position;
  vec4 p1 = gl_TessCoord.y * gl_in[1].gl_Position;
  vec4 p2 = gl_TessCoord.z * gl_in[2].gl_Position;
  gl_Position = transformPosition(p0 + p1 + p2);
}
