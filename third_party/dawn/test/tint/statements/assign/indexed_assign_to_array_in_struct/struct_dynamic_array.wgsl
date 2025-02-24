struct Uniforms {
  i : u32,
};

struct InnerS {
  v : i32,
};

struct OuterS {
  a1 : array<InnerS>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;
@binding(0) @group(0) var<storage, read_write> s1 : OuterS;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  s1.a1[uniforms.i] = v;
}
