#version 430

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[1];
};

struct t_9
{
    mat2x4 m_m0;
    vec4 m_m1;
    float m_m2;
    float m_m3;
    float m_m4;
    float m_m5;
};

struct t_10
{
    t_9 m_m0;
};

layout(location = 0) in vec4 v_4;
layout(location = 1) out vec4 v_5;
layout(location = 1) in vec4 v_6;
layout(location = 2) out t_10 v_7;
t_10 v_33 = t_10(t_9(mat2x4(vec4(1.0), vec4(1.0)), vec4(1.0), 1.0, 1.0, 1.0, 1.0));
const t_10 v_7_init = t_10(t_9(mat2x4(vec4(1.0), vec4(1.0)), vec4(1.0), 1.0, 1.0, 1.0, 1.0));

void main()
{
    v_7 = v_7_init;
    gl_Position = v_4;
    v_5 = v_6;
}
