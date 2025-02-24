struct Uniforms {
  i : u32,
  j : u32,
}

struct InnerS {
  v : i32,
}

struct S1 {
  a2 : array<InnerS, 8>,
}

struct OuterS {
  a1 : array<S1>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@binding(0) @group(0) var<storage, read_write> s : OuterS;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  s.a1[uniforms.i].a2[uniforms.j] = v;
}
