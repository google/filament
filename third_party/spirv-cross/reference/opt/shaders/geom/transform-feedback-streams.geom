#version 450
layout(points) in;
layout(max_vertices = 2, points) out;

layout(xfb_buffer = 1, xfb_stride = 20, stream = 1) out gl_PerVertex
{
    layout(xfb_offset = 4) vec4 gl_Position;
    float gl_PointSize;
};

layout(location = 0, xfb_buffer = 2, xfb_stride = 32, xfb_offset = 16, stream = 1) out vec4 vFoo;
layout(xfb_buffer = 3, xfb_stride = 16, stream = 2) out VertOut
{
    layout(location = 1, xfb_offset = 0) vec4 vBar;
} _23;


void main()
{
    gl_Position = vec4(1.0);
    vFoo = vec4(3.0);
    EmitStreamVertex(1);
    _23.vBar = vec4(5.0);
    EmitStreamVertex(2);
}

