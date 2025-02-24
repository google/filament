#version 310 es
precision highp float;
precision highp int;

void main_1() {
  mat2 m2i = mat2(vec2(0.0f), vec2(0.0f));
  mat2 m2 = mat2(vec2(0.0f), vec2(0.0f));
  mat3 m3i = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  mat3 m3 = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  mat4 m4i = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  mat4 m4 = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  float s = (1.0f / determinant(m2));
  vec2 v = vec2((s * m2[1u].y), (-(s) * m2[0u].y));
  m2i = mat2(v, vec2((-(s) * m2[1u].x), (s * m2[0u].x)));
  float v_1 = (1.0f / determinant(m3));
  vec3 v_2 = vec3(((m3[1u].y * m3[2u].z) - (m3[1u].z * m3[2u].y)), ((m3[0u].z * m3[2u].y) - (m3[0u].y * m3[2u].z)), ((m3[0u].y * m3[1u].z) - (m3[0u].z * m3[1u].y)));
  vec3 v_3 = vec3(((m3[1u].z * m3[2u].x) - (m3[1u].x * m3[2u].z)), ((m3[0u].x * m3[2u].z) - (m3[0u].z * m3[2u].x)), ((m3[0u].z * m3[1u].x) - (m3[0u].x * m3[1u].z)));
  m3i = (v_1 * mat3(v_2, v_3, vec3(((m3[1u].x * m3[2u].y) - (m3[1u].y * m3[2u].x)), ((m3[0u].y * m3[2u].x) - (m3[0u].x * m3[2u].y)), ((m3[0u].x * m3[1u].y) - (m3[0u].y * m3[1u].x)))));
  float v_4 = (1.0f / determinant(m4));
  vec4 v_5 = vec4((((m4[1u].y * ((m4[2u].z * m4[3u].w) - (m4[2u].w * m4[3u].z))) - (m4[1u].z * ((m4[2u].y * m4[3u].w) - (m4[2u].w * m4[3u].y)))) + (m4[1u].w * ((m4[2u].y * m4[3u].z) - (m4[2u].z * m4[3u].y)))), (((-(m4[0u].y) * ((m4[2u].z * m4[3u].w) - (m4[2u].w * m4[3u].z))) + (m4[0u].z * ((m4[2u].y * m4[3u].w) - (m4[2u].w * m4[3u].y)))) - (m4[0u].w * ((m4[2u].y * m4[3u].z) - (m4[2u].z * m4[3u].y)))), (((m4[0u].y * ((m4[1u].z * m4[3u].w) - (m4[1u].w * m4[3u].z))) - (m4[0u].z * ((m4[1u].y * m4[3u].w) - (m4[1u].w * m4[3u].y)))) + (m4[0u].w * ((m4[1u].y * m4[3u].z) - (m4[1u].z * m4[3u].y)))), (((-(m4[0u].y) * ((m4[1u].z * m4[2u].w) - (m4[1u].w * m4[2u].z))) + (m4[0u].z * ((m4[1u].y * m4[2u].w) - (m4[1u].w * m4[2u].y)))) - (m4[0u].w * ((m4[1u].y * m4[2u].z) - (m4[1u].z * m4[2u].y)))));
  vec4 v_6 = vec4((((-(m4[1u].x) * ((m4[2u].z * m4[3u].w) - (m4[2u].w * m4[3u].z))) + (m4[1u].z * ((m4[2u].x * m4[3u].w) - (m4[2u].w * m4[3u].x)))) - (m4[1u].w * ((m4[2u].x * m4[3u].z) - (m4[2u].z * m4[3u].x)))), (((m4[0u].x * ((m4[2u].z * m4[3u].w) - (m4[2u].w * m4[3u].z))) - (m4[0u].z * ((m4[2u].x * m4[3u].w) - (m4[2u].w * m4[3u].x)))) + (m4[0u].w * ((m4[2u].x * m4[3u].z) - (m4[2u].z * m4[3u].x)))), (((-(m4[0u].x) * ((m4[1u].z * m4[3u].w) - (m4[1u].w * m4[3u].z))) + (m4[0u].z * ((m4[1u].x * m4[3u].w) - (m4[1u].w * m4[3u].x)))) - (m4[0u].w * ((m4[1u].x * m4[3u].z) - (m4[1u].z * m4[3u].x)))), (((m4[0u].x * ((m4[1u].z * m4[2u].w) - (m4[1u].w * m4[2u].z))) - (m4[0u].z * ((m4[1u].x * m4[2u].w) - (m4[1u].w * m4[2u].x)))) + (m4[0u].w * ((m4[1u].x * m4[2u].z) - (m4[1u].z * m4[2u].x)))));
  vec4 v_7 = vec4((((m4[1u].x * ((m4[2u].y * m4[3u].w) - (m4[2u].w * m4[3u].y))) - (m4[1u].y * ((m4[2u].x * m4[3u].w) - (m4[2u].w * m4[3u].x)))) + (m4[1u].w * ((m4[2u].x * m4[3u].y) - (m4[2u].y * m4[3u].x)))), (((-(m4[0u].x) * ((m4[2u].y * m4[3u].w) - (m4[2u].w * m4[3u].y))) + (m4[0u].y * ((m4[2u].x * m4[3u].w) - (m4[2u].w * m4[3u].x)))) - (m4[0u].w * ((m4[2u].x * m4[3u].y) - (m4[2u].y * m4[3u].x)))), (((m4[0u].x * ((m4[1u].y * m4[3u].w) - (m4[1u].w * m4[3u].y))) - (m4[0u].y * ((m4[1u].x * m4[3u].w) - (m4[1u].w * m4[3u].x)))) + (m4[0u].w * ((m4[1u].x * m4[3u].y) - (m4[1u].y * m4[3u].x)))), (((-(m4[0u].x) * ((m4[1u].y * m4[2u].w) - (m4[1u].w * m4[2u].y))) + (m4[0u].y * ((m4[1u].x * m4[2u].w) - (m4[1u].w * m4[2u].x)))) - (m4[0u].w * ((m4[1u].x * m4[2u].y) - (m4[1u].y * m4[2u].x)))));
  m4i = (v_4 * mat4(v_5, v_6, v_7, vec4((((-(m4[1u].x) * ((m4[2u].y * m4[3u].z) - (m4[2u].z * m4[3u].y))) + (m4[1u].y * ((m4[2u].x * m4[3u].z) - (m4[2u].z * m4[3u].x)))) - (m4[1u].z * ((m4[2u].x * m4[3u].y) - (m4[2u].y * m4[3u].x)))), (((m4[0u].x * ((m4[2u].y * m4[3u].z) - (m4[2u].z * m4[3u].y))) - (m4[0u].y * ((m4[2u].x * m4[3u].z) - (m4[2u].z * m4[3u].x)))) + (m4[0u].z * ((m4[2u].x * m4[3u].y) - (m4[2u].y * m4[3u].x)))), (((-(m4[0u].x) * ((m4[1u].y * m4[3u].z) - (m4[1u].z * m4[3u].y))) + (m4[0u].y * ((m4[1u].x * m4[3u].z) - (m4[1u].z * m4[3u].x)))) - (m4[0u].z * ((m4[1u].x * m4[3u].y) - (m4[1u].y * m4[3u].x)))), (((m4[0u].x * ((m4[1u].y * m4[2u].z) - (m4[1u].z * m4[2u].y))) - (m4[0u].y * ((m4[1u].x * m4[2u].z) - (m4[1u].z * m4[2u].x)))) + (m4[0u].z * ((m4[1u].x * m4[2u].y) - (m4[1u].y * m4[2u].x)))))));
}
void main() {
  main_1();
}
