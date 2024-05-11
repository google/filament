#version 450
layout(quads) in;

struct ControlPoint
{
    vec4 baz;
};

layout(location = 0) patch in vec4 input_foo;
layout(location = 1) patch in vec4 input_bar;
layout(location = 2) in ControlPoint CPData[];

void main()
{
    gl_Position = (((input_foo + input_bar) + vec2(gl_TessCoord.xy).xyxy) + CPData[0u].baz) + CPData[3u].baz;
}

