#version 460
#extension GL_EXT_shader_tile_image : require

precision highp float;

layout(location=1) tileImageEXT highp attachmentEXT in_color;
layout(location=0) out highp vec4 out_color;

void main(void)
{
     out_color = colorAttachmentReadEXT(in_color);
}
