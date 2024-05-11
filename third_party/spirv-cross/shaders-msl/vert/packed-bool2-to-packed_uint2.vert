#version 450 core

struct Struct
{
    bvec2 flags[1];
};

layout(set=0, binding=0, std140) uniform defaultUniformsVS
{
    Struct flags;
    vec2 uquad[4];
    mat4 umatrix;
};

layout (location = 0) in vec4 a_position;

void main()
{
    gl_Position = umatrix * vec4(uquad[gl_VertexIndex], a_position.z, a_position.w);
    if (flags.flags[0].x)
        gl_Position.z = 0.0;
}
