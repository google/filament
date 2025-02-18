fn c(z : i32) {
  var a = (1 + z);
  a = (a + 2);
}

fn b() {
  c(2);
  c(3);
}
