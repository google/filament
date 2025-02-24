fn func(pointer : ptr<private, i32>) -> i32 {
  return *pointer;
}

var<private> P : i32;

@compute @workgroup_size(1)
fn main() {
  let r = func(&P);
}
