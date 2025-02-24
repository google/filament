#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec4 va = vec4(0.0f);
  vec4 a = degrees(va);
  vec4 vb = vec4(1.0f);
  vec4 b = degrees(vb);
  vec4 vc = vec4(1.0f, 2.0f, 3.0f, 4.0f);
  vec4 c = degrees(vc);
  vec3 vd = vec3(0.0f);
  vec3 d = degrees(vd);
  vec3 ve = vec3(1.0f);
  vec3 e = degrees(ve);
  vec3 vf = vec3(1.0f, 2.0f, 3.0f);
  vec3 f = degrees(vf);
  vec2 vg = vec2(0.0f);
  vec2 g = degrees(vg);
  vec2 vh = vec2(1.0f);
  vec2 h = degrees(vh);
  vec2 vi = vec2(1.0f, 2.0f);
  vec2 i = degrees(vi);
  float vj = 1.0f;
  float j = degrees(vj);
  float vk = 2.0f;
  float k = degrees(vk);
  float vl = 3.0f;
  float l = degrees(vl);
}
