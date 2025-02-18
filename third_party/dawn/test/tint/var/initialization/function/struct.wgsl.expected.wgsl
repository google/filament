struct S {
  a : i32,
  b : f32,
}

@compute @workgroup_size(1)
fn main() {
  var v : S;
  _ = v;
}
