#version 450
layout(triangles, ccw, equal_spacing) in;

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[1];
    float gl_CullDistance[1];
};

struct t35
{
    vec2 m0;
    vec4 m1;
};

layout(location = 0) in t36
{
    vec2 m0;
    t35 m1;
} v40[32];

layout(location = 0) out float v80;

void main()
{
    gl_Position = vec4((gl_TessCoord.xy * 2.0) - vec2(1.0), 0.0, 1.0);
    float v34 = ((float(abs(v40[0].m1.m1.x - (-4.0)) < 0.001000000047497451305389404296875) * float(abs(v40[0].m1.m1.y - (-9.0)) < 0.001000000047497451305389404296875)) * float(abs(v40[0].m1.m1.z - 3.0) < 0.001000000047497451305389404296875)) * float(abs(v40[0].m1.m1.w - 7.0) < 0.001000000047497451305389404296875);
    v80 = v34;
}
