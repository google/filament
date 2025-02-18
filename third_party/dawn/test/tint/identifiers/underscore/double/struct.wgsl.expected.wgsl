@group(0) @binding(0) var<storage, read_write> s : i32;

struct a {
  b : i32,
}

struct a__ {
  b__ : i32,
}

@compute @workgroup_size(1)
fn f() {
  let c = a__();
  let d = c.b__;
  s = (c.b__ + d);
}
