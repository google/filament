@group(0) @binding(0) var<storage, read_write> s: f32;

@compute @workgroup_size(1)
fn main() {
  var m : mat3x3<f32>;
  let v : vec3<f32> = m[1];
  let f : f32 = v[1];

  s = f;
}
