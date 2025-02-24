#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2D t_s;
void main() {
  vec4 res = textureGather(t_s, vec2(0.0f), 1);
}
