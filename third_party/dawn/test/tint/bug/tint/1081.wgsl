fn f(x : i32) -> i32 {
  if (x == 10) {
    discard;
  }
  return x;
}

@fragment
fn main(@location(1) @interpolate(flat) x: vec3<i32>) -> @location(2) i32 {
  var y = x.x;
  loop {
    let r = f(y);
    if (r == 0) {
      break;
    }
  }
  return y;
}
