fn c() -> i32 {
  var a = 1;
  a = (a + 2);
  return a;
}

fn b() {
  var b = c();
  b += c();
}
