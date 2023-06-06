#version 450

layout(ccw, quads, fractional_even_spacing) in;

// Try to use the whole taxonomy of input methods.

// Per-vertex vector.
layout(location = 0) in vec4 vColor[];
// Per-patch vector.
layout(location = 1) patch in vec4 vColors;
// Per-patch vector array.
layout(location = 2) patch in vec4 vColorsArray[2];

// I/O blocks, per patch and per control point.
layout(location = 4) in Block
{
        vec4 a;
        vec4 b;
} blocks[];

layout(location = 6) patch in PatchBlock
{
        vec4 a;
        vec4 b;
} patch_block;

// Composites.
struct Foo
{
        vec4 a;
        vec4 b;
};
layout(location = 8) patch in Foo vFoo;
//layout(location = 10) patch in Foo vFooArray[2];  // FIXME: Handling of array-of-struct input is broken!

// Per-control point struct.
layout(location = 14) in Foo vFoos[];

void set_from_function()
{
        gl_Position = blocks[0].a;
        gl_Position += blocks[0].b;
        gl_Position += blocks[1].a;
        gl_Position += blocks[1].b;
        gl_Position += patch_block.a;
        gl_Position += patch_block.b;
        gl_Position += vColor[0];
        gl_Position += vColor[1];
        gl_Position += vColors;

        Foo foo = vFoo;
        gl_Position += foo.a;
        gl_Position += foo.b;

        /*foo = vFooArray[0];
        gl_Position += foo.a;
        gl_Position += foo.b;

        foo = vFooArray[1];
        gl_Position += foo.a;
        gl_Position += foo.b;*/

        foo = vFoos[0];
        gl_Position += foo.a;
        gl_Position += foo.b;

        foo = vFoos[1];
        gl_Position += foo.a;
        gl_Position += foo.b;
}

void main()
{
        set_from_function();
}
