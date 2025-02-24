@group(0) @binding(0)
var<storage> v : array<i32, 1000000>;

struct A {
  a : array<f32, 1000000>,
}
@group(0) @binding(1)
var<storage> b : A;
