#version 460
#extension GL_EXT_shader_tile_image : require

precision highp float;

layout(location=0) out highp uvec4 stencil_out;
layout(location=1) out highp vec4 depth_out;


void main(void)
{
    float depth = depthAttachmentReadEXT();
    uint stencil_value = stencilAttachmentReadEXT();
    stencil_out = uvec4(stencil_value,0,0,0);
    depth_out = vec4(depth,0.0,0.0,0.0);
}
