struct S {
  a : i32,
  b : vec4<f32>,
  c : mat2x2<f32>,
}
@group(0) @binding(0)
var<storage, read_write> v : S;

var<private> i : u32;

fn idx1() -> i32 {
  i += 1u;
  return 1;
}

fn idx2() -> i32 {
  i += 2u;
  return 1;
}

fn idx3() -> i32 {
  i += 3u;
  return 1;
}

fn foo() {
  var a  = array<f32, 4>();
  // Make sure that the functions are only evaluated once each.
  for (a[idx1()] *= 2.0; a[idx2()] < 10.0; a[idx3()] += 1.0) {
  }
}
