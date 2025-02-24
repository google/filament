fn main_1() {
  var m = mat3x3f();
  let x_16 = m[1i].y;
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
