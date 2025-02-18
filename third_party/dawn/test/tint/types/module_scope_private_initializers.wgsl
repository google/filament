var<private> a : f32 = 1.0;
var<private> b : f32 = f32();

@compute @workgroup_size(1)
fn main() {
  let x : f32 = a + b;
}
