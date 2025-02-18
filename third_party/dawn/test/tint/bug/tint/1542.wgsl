struct UniformBuffer {
  d: vec3<i32>,
}

@group(0) @binding(0)
var<uniform> u_input: UniformBuffer;

@compute @workgroup_size(1)
fn main() {
  let temp: vec3<i32> = (u_input.d << vec3<u32>());
}
