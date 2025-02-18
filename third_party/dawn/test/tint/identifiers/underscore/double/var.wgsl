@group(0) @binding(0) var<storage, read_write> s: i32;

var<private> a : i32 = 1;
var<private> a__ : i32 = 2;

@compute @workgroup_size(1)
fn f() {
  var b : i32 = a;
  var b__ : i32 = a__;

  s = b + b__;
}
