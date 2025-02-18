 struct Uniforms {
  i : u32,
  j : u32,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

var<private> m1 : mat2x4<f32>;

@compute @workgroup_size(1)
fn main() {
  m1[uniforms.i] = vec4<f32>(1.0);
}
