fn main_1() {
  var i = 0i;
  i = 123i;
  i = 123i;
  i = ((100i + 20i) + 3i);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
