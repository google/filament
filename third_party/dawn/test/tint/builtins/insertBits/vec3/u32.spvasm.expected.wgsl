fn f_1() {
  var v = vec3u();
  var n = vec3u();
  var offset_1 = 0u;
  var count = 0u;
  let x_15 = insertBits(v, n, offset_1, count);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn f() {
  f_1();
}
