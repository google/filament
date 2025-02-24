@group(0) @binding(0) var<storage, read_write> s: i32;

var<private> a : i32 = 1;
var<private> _a : i32 = 2;

@compute @workgroup_size(1)
fn f() {
  var b : i32 = a;
  var _b : i32 = _a;

  s = b + _b;
}
