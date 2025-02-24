alias T1 = T2;

alias T2 = i32;

@fragment
fn f() {
  var v : T1;
}
