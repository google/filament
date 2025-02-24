#version 460

layout(binding = 0, rgba8) uniform highp image2D tex;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  imageStore(tex, ivec2(0), vec4(0.0f));
}
