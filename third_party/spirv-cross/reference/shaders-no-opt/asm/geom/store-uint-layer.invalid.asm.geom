#version 450
layout(triangles) in;
layout(max_vertices = 3, triangle_strip) out;

struct VertexOutput
{
    vec4 pos;
};

struct GeometryOutput
{
    vec4 pos;
    uint layer;
};

void _main(VertexOutput _input[3], GeometryOutput stream)
{
    GeometryOutput _output;
    _output.layer = 1u;
    for (int v = 0; v < 3; v++)
    {
        _output.pos = _input[v].pos;
        gl_Position = _output.pos;
        gl_Layer = int(_output.layer);
        EmitVertex();
    }
    EndPrimitive();
}

void main()
{
    VertexOutput _input[3];
    _input[0].pos = gl_in[0].gl_Position;
    _input[1].pos = gl_in[1].gl_Position;
    _input[2].pos = gl_in[2].gl_Position;
    VertexOutput param[3] = _input;
    GeometryOutput param_1;
    _main(param, param_1);
    GeometryOutput stream = param_1;
}

