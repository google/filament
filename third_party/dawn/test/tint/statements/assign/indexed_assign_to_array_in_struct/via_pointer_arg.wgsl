struct Uniforms {
  i : u32,
};
struct InnerS {
  v : i32,
};
struct OuterS {
  a1 : array<InnerS, 8>,
};
@group(1) @binding(4) var<uniform> uniforms : Uniforms;

fn f(p : ptr<function, OuterS>) {
  var v : InnerS;
  (*p).a1[uniforms.i] = v;
}

@compute @workgroup_size(1)
fn main() {
  var s1 : OuterS;
  f(&s1);
}
