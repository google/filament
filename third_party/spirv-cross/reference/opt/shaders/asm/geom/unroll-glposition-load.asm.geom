#version 450
layout(triangles) in;
layout(max_vertices = 3, triangle_strip) out;

void main()
{
    for (int _74 = 0; _74 < 3; )
    {
        gl_Position = gl_in[_74].gl_Position;
        EmitVertex();
        _74++;
        continue;
    }
    EndPrimitive();
}

