fn func(pointer : ptr<private, i32>) {
  *pointer = 42;
}

var<private> P : i32;

@compute @workgroup_size(1)
fn main() {
  func(&P);
}
