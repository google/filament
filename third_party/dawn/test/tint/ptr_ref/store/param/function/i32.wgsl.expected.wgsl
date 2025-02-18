fn func(pointer : ptr<function, i32>) {
  *(pointer) = 42;
}

@compute @workgroup_size(1)
fn main() {
  var F : i32;
  func(&(F));
}
