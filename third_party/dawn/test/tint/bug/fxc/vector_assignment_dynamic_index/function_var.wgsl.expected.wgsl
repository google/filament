@group(0) @binding(0) var<uniform> i : u32;

@compute @workgroup_size(1)
fn main() {
  var v1 : vec3<f32>;
  v1[i] = 1.0;
}
