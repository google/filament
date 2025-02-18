@group(0) @binding(0) var<storage, read_write> s : i32;

var<private> A : i32 = 1;

var<private> _A : i32 = 2;

@compute @workgroup_size(1)
fn f() {
  var B : i32 = A;
  var _B : i32 = _A;
  s = (B + _B);
}
