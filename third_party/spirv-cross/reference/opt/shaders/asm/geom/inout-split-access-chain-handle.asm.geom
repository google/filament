#version 440
layout(triangles) in;
layout(max_vertices = 5, triangle_strip) out;

void main()
{
    gl_Position = gl_in[0].gl_Position;
}

