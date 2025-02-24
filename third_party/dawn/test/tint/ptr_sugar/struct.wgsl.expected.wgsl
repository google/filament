struct S {
  x : i32,
}

fn deref() {
  var a : S;
  let p = &(a);
  var b = (*(p)).x;
  (*(p)).x = 42;
}

fn no_deref() {
  var a : S;
  let p = &(a);
  var b = p.x;
  p.x = 42;
}

@compute @workgroup_size(1)
fn main() {
  deref();
  no_deref();
}
