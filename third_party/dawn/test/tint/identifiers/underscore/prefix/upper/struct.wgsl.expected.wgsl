@group(0) @binding(0) var<storage, read_write> s : i32;

struct A {
  B : i32,
}

struct _A {
  _B : i32,
}

@compute @workgroup_size(1)
fn f() {
  let c = _A();
  let d = c._B;
  s = (c._B + d);
}
