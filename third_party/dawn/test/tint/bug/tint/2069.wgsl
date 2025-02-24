const o0 : f32 = 1.f;

var<private> v = modf(f32(o0));

@compute @workgroup_size(1)
fn main() {
  _ = v;
}
