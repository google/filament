struct S {
  i : i32,
}

fn f() {
  for(var i = 0; false; i = (i + S(1).i)) {
  }
}
