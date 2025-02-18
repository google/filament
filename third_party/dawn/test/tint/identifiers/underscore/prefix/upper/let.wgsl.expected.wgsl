@group(0) @binding(0) var<storage, read_write> s : i32;

@compute @workgroup_size(1)
fn f() {
  let A : i32 = 1;
  let _A : i32 = 2;
  let B = A;
  let _B = _A;
  s = (((A + _A) + B) + _B);
}
