struct S {
  i : i32,
}

var<private> V : S;

fn main_1() {
  var i : i32;
  i = V.i;
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
