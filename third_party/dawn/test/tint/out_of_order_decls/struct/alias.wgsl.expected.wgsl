struct S {
  m : T,
}

alias T = i32;

@fragment
fn f() {
  var v : S;
}
