#version 460

layout(rgba8, binding = 0) uniform image2D test_image2D;

layout(rgba8ui, binding = 1) uniform uimage2D test_uimage2D;

layout(binding = 2) uniform sampler test_sampler;

layout(binding = 3) uniform sampler2D test_sampler2D;

layout(binding = 4) uniform usampler3D test_usampler3D;

layout(binding = 5) uniform samplerCubeArray test_samplerCubeArray;

layout(binding = 6) uniform texture2DArray test_texture2DArray;

void main() {

}