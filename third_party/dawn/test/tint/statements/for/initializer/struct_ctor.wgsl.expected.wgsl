struct S {
  i : i32,
}

fn f() {
  for(var i : i32 = S(1).i; false; ) {
  }
}
