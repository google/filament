#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2D t_s;
uniform highp sampler2DShadow d_sc;
void main() {
  vec4 a = texture(t_s, vec2(1.0f));
  vec4 b = textureGather(d_sc, vec2(1.0f), 1.0f);
}
