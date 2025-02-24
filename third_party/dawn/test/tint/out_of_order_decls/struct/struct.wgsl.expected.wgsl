struct S1 {
  m : S2,
}

struct S2 {
  m : i32,
}

@fragment
fn f() {
  var v : S1;
}
