struct S_inner {
  a : f32,
}

struct S {
  member_bool : bool,
  member_i32 : i32,
  member_u32 : u32,
  member_f32 : f32,
  member_v2i32 : vec2<i32>,
  member_v3u32 : vec3<u32>,
  member_v4f32 : vec4<f32>,
  member_m2x3 : mat2x3<f32>,
  member_arr : array<f32, 4>,
  member_struct : S_inner,
}

@compute @workgroup_size(1)
fn main() {
  let s : S = S();
}
