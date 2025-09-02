#version 320 es
#extension GL_EXT_shader_tile_image : require

layout(non_coherent_color_attachment_readEXT) in;

layout(location=0) tileImageEXT highp iattachmentEXT in_color_i;
layout(location=0) out highp vec4 out_color_f;

void main(void)
{
     highp ivec4 read = colorAttachmentReadEXT(in_color_i);
     out_color_f = vec4(read);
}
