#version 460
#extension GL_EXT_shader_tile_image : require

precision highp float;
precision mediump int;

layout(binding=0, set=0, input_attachment_index=0) tileImageEXT highp attachmentEXT in_color;
layout(location=0) out highp vec4 out_color;

void main(void)
{
     out_color = colorAttachmentReadEXT(in_color);
}
