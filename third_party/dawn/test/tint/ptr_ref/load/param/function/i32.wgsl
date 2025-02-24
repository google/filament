fn func(pointer : ptr<function, i32>) -> i32 {
  return *pointer;
}

@compute @workgroup_size(1)
fn main() {
  var F : i32;
  let r = func(&F);
}
