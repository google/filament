fn deref() {
  var a : vec3<i32>;
  let p = &a;
  var b = (*p).x;
  (*p).x = 42;
}

fn no_deref() {
  var a : vec3<i32>;
  let p = &a;
  var b = p.x;
  p.x = 42;
}

@compute @workgroup_size(1)
fn main() {
  deref();
  no_deref();
}
