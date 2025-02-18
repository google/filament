struct Uniforms {
  i : u32,
  j : u32,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var m1 : mat2x4<f32>;
  m1[uniforms.i] = vec4<f32>(1.0);
}
