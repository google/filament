#version 450
#extension GL_EXT_shader_tile_image : require

precision highp float;

layout(set = 0, binding = 0, input_attachment_index = 0) uniform subpassInput i;

layout(location = 0) out vec4 fragColor;

void main()
{
  fragColor = subpassLoad(i);
}
