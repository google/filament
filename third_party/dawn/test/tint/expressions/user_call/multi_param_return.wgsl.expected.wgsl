fn c(x : i32, y : i32, z : i32) -> i32 {
  var a = (((1 + x) + y) + z);
  a = (a + 2);
  return a;
}

fn b() {
  var b = c(2, 3, 4);
  b += c(3, 4, 5);
}
