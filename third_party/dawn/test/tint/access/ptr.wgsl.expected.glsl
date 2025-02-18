#version 310 es


struct S {
  int a;
  int b;
};

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  int inner;
} v;
shared int g1;
int accept_value(int val) {
  return val;
}
int accept_ptr_deref_call_func(inout int val) {
  int v_1 = val;
  return (v_1 + accept_value(val));
}
int accept_ptr_deref_pass_through(inout int val) {
  int v_2 = val;
  return (v_2 + accept_ptr_deref_call_func(val));
}
int accept_ptr_to_struct_and_access(inout S val) {
  return (val.a + val.b);
}
int accept_ptr_to_struct_access_pass_ptr(inout S val) {
  val.a = 2;
  return val.a;
}
int tint_f32_to_i32(float value) {
  return mix(2147483647, mix((-2147483647 - 1), int(value), (value >= -2147483648.0f)), (value <= 2147483520.0f));
}
int accept_ptr_vec_access_elements(inout vec3 v1) {
  v1.x = cross(v1, v1).x;
  return tint_f32_to_i32(v1.x);
}
int call_builtin_with_mod_scope_ptr() {
  return atomicOr(g1, 0);
}
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    atomicExchange(g1, 0);
  }
  barrier();
  int v1 = 0;
  S v2 = S(0, 0);
  vec3 v4 = vec3(0.0f);
  int t1 = atomicOr(g1, 0);
  int v_3 = accept_ptr_deref_pass_through(v1);
  int v_4 = (v_3 + accept_ptr_to_struct_and_access(v2));
  int v_5 = (v_4 + accept_ptr_to_struct_and_access(v2));
  int v_6 = (v_5 + accept_ptr_vec_access_elements(v4));
  int v_7 = (v_6 + accept_ptr_to_struct_access_pass_ptr(v2));
  v.inner = ((v_7 + call_builtin_with_mod_scope_ptr()) + t1);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
