struct S {
  a : i32,
  b : f32,
}

var<private> v : S;

@compute @workgroup_size(1)
fn main() {
  _ = v;
}
