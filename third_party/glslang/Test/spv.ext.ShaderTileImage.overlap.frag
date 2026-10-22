#version 460
#extension GL_EXT_shader_tile_image : require

precision mediump int;
precision highp float;

layout(location=0) tileImageEXT highp attachmentEXT in_color[2];
layout(location=1) tileImageEXT highp attachmentEXT in_color_1;
layout(location=1) out highp vec4 out_color;

void main(void)
{
     out_color = colorAttachmentReadEXT(in_color[0]);
     out_color += colorAttachmentReadEXT(in_color_1);
}
