fn deref() {
  var a : vec3<i32>;
  let p = &a;
  (*p)[0] += 42;
}

fn no_deref() {
  var a : vec3<i32>;
  let p = &a;
  p[0] += 42;
}

fn deref_inc() {
  var a : vec3<i32>;
  let p = &a;
  (*p)[0]++;
}

fn no_deref_inc() {
  var a : vec3<i32>;
  let p = &a;
  p[0]++;
}

@compute @workgroup_size(1)
fn main() {
  deref();
  no_deref();
  deref_inc();
  no_deref_inc();
}
