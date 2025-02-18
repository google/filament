#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2D t_s;
layout(location = 0) out vec4 f_loc0_Output;
vec4 f_inner() {
  return textureOffset(t_s, vec2(0.0f), ivec2(4, 6));
}
void main() {
  f_loc0_Output = f_inner();
}
