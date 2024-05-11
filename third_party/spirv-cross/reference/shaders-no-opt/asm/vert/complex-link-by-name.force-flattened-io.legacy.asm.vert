#version 100

struct Struct_vec4
{
    vec4 m0;
};

struct UBO
{
    Struct_vec4 m0;
    Struct_vec4 m1;
};

uniform UBO ubo_binding_0;

varying vec4 output_location_0_m0_m0;
varying vec4 output_location_0_m1_m0;
varying vec4 output_location_2_m0;
varying vec4 output_location_3_m0;

void main()
{
    Struct_vec4 c;
    c.m0 = ubo_binding_0.m0.m0;
    Struct_vec4 b;
    b.m0 = ubo_binding_0.m1.m0;
    gl_Position = c.m0 + b.m0;
    output_location_0_m0_m0 = c.m0;
    output_location_0_m1_m0 = b.m0;
    output_location_2_m0 = c.m0;
    output_location_3_m0 = b.m0;
}

