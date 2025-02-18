fn main_1() {
  var i = 0i;
  i = 123i;
  let x_12 = (i + 1i);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
