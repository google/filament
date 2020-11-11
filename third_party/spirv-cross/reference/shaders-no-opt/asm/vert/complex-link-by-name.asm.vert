#version 450

struct Struct_vec4
{
    vec4 m0;
};

layout(binding = 0, std140) uniform UBO
{
    Struct_vec4 m0;
    Struct_vec4 m1;
} ubo_binding_0;

layout(location = 0) out VertexOut
{
    Struct_vec4 m0;
    Struct_vec4 m1;
} output_location_0;

layout(location = 2) out Struct_vec4 output_location_2;
layout(location = 3) out Struct_vec4 output_location_3;

void main()
{
    Struct_vec4 c;
    c.m0 = ubo_binding_0.m0.m0;
    Struct_vec4 b;
    b.m0 = ubo_binding_0.m1.m0;
    gl_Position = c.m0 + b.m0;
    output_location_0.m0 = c;
    output_location_0.m1 = b;
    output_location_2 = c;
    output_location_3 = b;
}

