struct S {
  i : i32,
}

var<private> V : S;

fn main_1() {
  V.i = 5i;
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
