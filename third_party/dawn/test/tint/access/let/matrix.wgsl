@group(0) @binding(0) var<storage, read_write> s: f32;

@compute @workgroup_size(1)
fn main() {
  let m : mat3x3<f32> = mat3x3<f32>(vec3<f32>(1., 2., 3.), vec3<f32>(4., 5., 6.), vec3<f32>(7., 8., 9.));
  let v : vec3<f32> = m[1];
  let f : f32 = v[1];

  s = f;
}
