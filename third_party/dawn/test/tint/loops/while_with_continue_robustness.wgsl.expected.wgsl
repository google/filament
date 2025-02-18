fn f() -> i32 {
  var i : i32;
  while((i < 4)) {
    i = (i + 1);
    continue;
  }
  return i;
}
