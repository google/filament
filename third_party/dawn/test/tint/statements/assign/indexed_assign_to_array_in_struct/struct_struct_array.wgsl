struct Uniforms {
  i : u32,
};

struct InnerS {
  v : i32,
};

struct S1 {
  a : array<InnerS, 8>,
};

struct OuterS {
  s2 : S1,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  s1.s2.a[uniforms.i] = v;
}
