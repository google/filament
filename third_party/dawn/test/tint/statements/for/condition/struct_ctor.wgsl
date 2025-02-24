struct S {
  i : i32,
};

fn f() {
  var i : i32;
  for (; i < S(1).i;) {}
}
