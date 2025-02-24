fn add_int_min_explicit() -> i32 {
  var a = -(2147483648);
  var b = (a + 1);
  var c = (-(2147483648) + 1);
  return c;
}
