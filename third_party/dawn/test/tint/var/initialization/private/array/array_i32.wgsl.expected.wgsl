var<private> zero : array<array<i32, 3>, 2>;

var<private> init : array<array<i32, 3>, 2> = array<array<i32, 3>, 2>(array<i32, 3>(1, 2, 3), array<i32, 3>(4, 5, 6));

@compute @workgroup_size(1)
fn main() {
  var v0 = zero;
  var v1 = init;
}
