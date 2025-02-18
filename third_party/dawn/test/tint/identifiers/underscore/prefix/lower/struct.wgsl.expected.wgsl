@group(0) @binding(0) var<storage, read_write> s : i32;

struct a {
  b : i32,
}

struct _a {
  _b : i32,
}

@compute @workgroup_size(1)
fn f() {
  let c = _a();
  let d = c._b;
  s = (c._b + d);
}
