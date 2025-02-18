struct S {
  i : i32,
}

fn main_1() {
  var V : S;
  V.i = 5i;
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
