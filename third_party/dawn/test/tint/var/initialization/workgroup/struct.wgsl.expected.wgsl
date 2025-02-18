struct S {
  a : i32,
  b : f32,
}

var<workgroup> v : S;

@compute @workgroup_size(1)
fn main() {
  _ = v;
}
