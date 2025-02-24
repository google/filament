#version 310 es
precision highp float;
precision highp int;

uniform highp isampler2D t_s;
void main() {
  ivec4 res = textureGather(t_s, vec2(0.0f), 1);
}
