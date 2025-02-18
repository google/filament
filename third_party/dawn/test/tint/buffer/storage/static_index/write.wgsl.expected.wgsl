struct Inner {
  scalar_i32 : i32,
  scalar_f32 : f32,
}

struct S {
  scalar_f32 : f32,
  scalar_i32 : i32,
  scalar_u32 : u32,
  vec2_f32 : vec2<f32>,
  vec2_i32 : vec2<i32>,
  vec2_u32 : vec2<u32>,
  vec3_f32 : vec3<f32>,
  vec3_i32 : vec3<i32>,
  vec3_u32 : vec3<u32>,
  vec4_f32 : vec4<f32>,
  vec4_i32 : vec4<i32>,
  vec4_u32 : vec4<u32>,
  mat2x2_f32 : mat2x2<f32>,
  mat2x3_f32 : mat2x3<f32>,
  mat2x4_f32 : mat2x4<f32>,
  mat3x2_f32 : mat3x2<f32>,
  mat3x3_f32 : mat3x3<f32>,
  mat3x4_f32 : mat3x4<f32>,
  mat4x2_f32 : mat4x2<f32>,
  mat4x3_f32 : mat4x3<f32>,
  mat4x4_f32 : mat4x4<f32>,
  arr2_vec3_f32 : array<vec3<f32>, 2>,
  struct_inner : Inner,
  array_struct_inner : array<Inner, 4>,
}

@binding(0) @group(0) var<storage, read_write> sb : S;

@compute @workgroup_size(1)
fn main() {
  sb.scalar_f32 = f32();
  sb.scalar_i32 = i32();
  sb.scalar_u32 = u32();
  sb.vec2_f32 = vec2<f32>();
  sb.vec2_i32 = vec2<i32>();
  sb.vec2_u32 = vec2<u32>();
  sb.vec3_f32 = vec3<f32>();
  sb.vec3_i32 = vec3<i32>();
  sb.vec3_u32 = vec3<u32>();
  sb.vec4_f32 = vec4<f32>();
  sb.vec4_i32 = vec4<i32>();
  sb.vec4_u32 = vec4<u32>();
  sb.mat2x2_f32 = mat2x2<f32>();
  sb.mat2x3_f32 = mat2x3<f32>();
  sb.mat2x4_f32 = mat2x4<f32>();
  sb.mat3x2_f32 = mat3x2<f32>();
  sb.mat3x3_f32 = mat3x3<f32>();
  sb.mat3x4_f32 = mat3x4<f32>();
  sb.mat4x2_f32 = mat4x2<f32>();
  sb.mat4x3_f32 = mat4x3<f32>();
  sb.mat4x4_f32 = mat4x4<f32>();
  sb.arr2_vec3_f32 = array<vec3<f32>, 2>();
  sb.struct_inner = Inner();
  sb.array_struct_inner = array<Inner, 4>();
}
