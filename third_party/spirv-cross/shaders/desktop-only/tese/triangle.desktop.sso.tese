#version 450

layout(cw, triangles, fractional_even_spacing) in;

in gl_PerVertex
{
   vec4 gl_Position;
} gl_in[gl_MaxPatchVertices];

out gl_PerVertex
{
   vec4 gl_Position;
};

void main()
{
   gl_Position =
      gl_in[0].gl_Position * gl_TessCoord.x +
      gl_in[1].gl_Position * gl_TessCoord.y +
      gl_in[2].gl_Position * gl_TessCoord.z;
}

