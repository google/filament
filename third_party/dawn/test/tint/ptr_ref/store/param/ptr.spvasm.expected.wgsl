fn func(value : i32, pointer : ptr<function, i32>) {
  *(pointer) = value;
  return;
}

fn main_1() {
  var i : i32 = 0i;
  i = 123i;
  func(123i, &(i));
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
