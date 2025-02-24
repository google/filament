struct Uniforms {
  i : u32,
};

struct OuterS {
  m1 : mat2x4<f32>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var s1 : OuterS;
  s1.m1[uniforms.i] = vec4<f32>(1.0);
  s1.m1[uniforms.i][uniforms.i] = 1.0;
}
