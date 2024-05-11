#version 310 es
precision mediump float;

layout(input_attachment_index = 0, set = 0, binding = 0) uniform mediump subpassInput uSubpass0;
layout(input_attachment_index = 1, set = 0, binding = 1) uniform mediump subpassInput uSubpass1;
layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = subpassLoad(uSubpass0) + subpassLoad(uSubpass1);
}
