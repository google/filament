struct Uniforms {
  i : u32,
};

struct OuterS {
  a1 : array<u32, 8>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

fn f(i: u32) -> u32 {
  return i + 1u;
}

@compute @workgroup_size(1)
fn main() {
  var s1 : OuterS;
  var v : vec3<f32>;
  v[s1.a1[uniforms.i]] = 1.0;
  v[f(s1.a1[uniforms.i])] = 1.0;
}
