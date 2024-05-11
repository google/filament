#version 450
layout(triangles) in;
layout(max_vertices = 4, triangle_strip) out;

void main()
{
    gl_ViewportIndex = 1;
}

