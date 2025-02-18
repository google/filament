@group(0) @binding(0) var<storage, read_write> s : f32;

@compute @workgroup_size(1)
fn main() {
  let m : mat3x3<f32> = mat3x3<f32>(vec3<f32>(1.0, 2.0, 3.0), vec3<f32>(4.0, 5.0, 6.0), vec3<f32>(7.0, 8.0, 9.0));
  let v : vec3<f32> = m[1];
  let f : f32 = v[1];
  s = f;
}
