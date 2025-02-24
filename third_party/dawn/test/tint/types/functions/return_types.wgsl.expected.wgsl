struct S {
  a : f32,
}

fn ret_bool() -> bool {
  return bool();
}

fn ret_i32() -> i32 {
  return i32();
}

fn ret_u32() -> u32 {
  return u32();
}

fn ret_f32() -> f32 {
  return f32();
}

fn ret_v2i32() -> vec2<i32> {
  return vec2<i32>();
}

fn ret_v3u32() -> vec3<u32> {
  return vec3<u32>();
}

fn ret_v4f32() -> vec4<f32> {
  return vec4<f32>();
}

fn ret_m2x3() -> mat2x3<f32> {
  return mat2x3<f32>();
}

fn ret_arr() -> array<f32, 4> {
  return array<f32, 4>();
}

fn ret_struct() -> S {
  return S();
}

@compute @workgroup_size(1)
fn main() {
  let a = ret_bool();
  let b = ret_i32();
  let c = ret_u32();
  let d = ret_f32();
  let e = ret_v2i32();
  let f = ret_v3u32();
  let g = ret_v4f32();
  let h = ret_m2x3();
  let i = ret_arr();
  let j = ret_struct();
}
