#version 450

layout(ccw, quads, fractional_even_spacing) in;

layout(location = 0) in vec4 vColor[];
layout(location = 1) patch in vec4 vColors;
layout(location = 2) in Block
{
        vec4 a;
        vec4 b;
} blocks[];

struct Foo
{
        vec4 a;
        vec4 b;
};
layout(location = 4) patch in Foo vFoo;

void set_from_function()
{
        gl_Position = blocks[0].a;
        gl_Position += blocks[0].b;
        gl_Position += blocks[1].a;
        gl_Position += blocks[1].b;
        gl_Position += vColor[0];
        gl_Position += vColor[1];
        gl_Position += vColors;
        gl_Position += vFoo.a;
        gl_Position += vFoo.b;
}

void main()
{
        set_from_function();
}
