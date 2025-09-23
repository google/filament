#version 430

struct t_6
{
    mat2x4 m_m0;
    vec4 m_m1;
    float m_m2;
    float m_m3;
    float m_m4;
    float m_m5;
};

struct t_19
{
    t_6 m_m0;
};

layout(set = 0, binding = 0, std430) buffer t_7_8
{
    t_6 m_m0;
} v_8;

layout(location = 0) out vec4 v_3;
layout(location = 1) in vec4 v_4;
layout(location = 2) in t_19 v_5;

void main()
{
    v_3 = v_4;
    v_8.m_m0 = v_5.m_m0;
}
