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
  a1 : array<S1, 8>,
}

var<private> nextIndex : u32;

fn getNextIndex() -> u32 {
  nextIndex = (nextIndex + 1u);
  return nextIndex;
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s : OuterS;
  s.a1[getNextIndex()].a2[uniforms.j] = v;
}
