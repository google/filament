fn func(value : i32, pointer : ptr<function, i32>) {
  *pointer = value;
}

@compute @workgroup_size(1)
fn main() {
  var i : i32 = 123;
  func(123, &i);
}
