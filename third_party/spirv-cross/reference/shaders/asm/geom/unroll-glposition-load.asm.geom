#version 450
layout(triangles) in;
layout(max_vertices = 3, triangle_strip) out;

struct SceneOut
{
    vec4 pos;
};

void _main(vec4 positions[3], SceneOut OUT)
{
    SceneOut o;
    for (int i = 0; i < 3; i++)
    {
        o.pos = positions[i];
        gl_Position = o.pos;
        EmitVertex();
    }
    EndPrimitive();
}

void main()
{
    vec4 _48_unrolled[3];
    for (int i = 0; i < int(3); i++)
    {
        _48_unrolled[i] = gl_in[i].gl_Position;
    }
    vec4 positions[3] = _48_unrolled;
    vec4 param[3] = positions;
    SceneOut param_1;
    _main(param, param_1);
    SceneOut OUT = param_1;
}

