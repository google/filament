@group(0) @binding(0) var<storage, read_write> s : i32;

@compute @workgroup_size(1)
fn f() {
  let a = 1;
  let a__ = a;
  let b = a;
  let b__ = a__;
  s = (((a + a__) + b) + b__);
}
