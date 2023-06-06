#version 450
layout(triangles) in;
layout(max_vertices = 2, points) out;

void main()
{
    gl_Position = gl_in[0].gl_Position;
    EmitStreamVertex(0);
    EndStreamPrimitive(0);
    gl_Position = gl_in[0].gl_Position + vec4(2.0);
    EmitStreamVertex(1);
    EndStreamPrimitive(1);
}

