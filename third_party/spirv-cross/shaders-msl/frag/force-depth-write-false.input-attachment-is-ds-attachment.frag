#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInput inputDepth;

layout (location = 0) out vec4 color;

void main()
{
    color = subpassLoad(inputDepth);
    gl_FragDepth = 1.0f;
}
