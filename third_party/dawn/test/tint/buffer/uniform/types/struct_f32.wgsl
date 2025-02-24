struct Inner {
  scalar_f32 : f32,
  vec3_f32 : vec3<f32>,
  mat2x4_f32 : mat2x4<f32>,
};
struct S {
  inner : Inner,
};

@group(0) @binding(0) var<uniform> u : S;
@group(0) @binding(1) var<storage, read_write> s: S;

@compute @workgroup_size(1)
fn main() {
  let x = u;

  s = x;
}
