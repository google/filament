struct Uniforms {
  i : u32,
};

struct InnerS {
  v : i32,
};

struct S1 {
  s2 : InnerS,
};

struct OuterS {
  a1 : array<S1, 8>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  s1.a1[uniforms.i].s2 = v;
}
