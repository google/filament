#version 450
layout(triangles) in;
layout(max_vertices = 3, triangle_strip) out;

void main()
{
    vec4 _35_unrolled[3];
    for (int i = 0; i < int(3); i++)
    {
        _35_unrolled[i] = gl_in[i].gl_Position;
    }
    vec4 param[3] = _35_unrolled;
    for (int _73 = 0; _73 < 3; )
    {
        gl_Position = param[_73];
        EmitVertex();
        _73++;
        continue;
    }
    EndPrimitive();
}

