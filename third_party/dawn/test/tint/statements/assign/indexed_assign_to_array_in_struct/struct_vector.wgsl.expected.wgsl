struct Uniforms {
  i : u32,
}

struct OuterS {
  v1 : vec3<f32>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var s1 : OuterS;
  s1.v1[uniforms.i] = 1.0;
}
