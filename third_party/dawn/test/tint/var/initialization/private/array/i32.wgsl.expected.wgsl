var<private> zero : array<i32, 3>;

var<private> init : array<i32, 3> = array<i32, 3>(1, 2, 3);

@compute @workgroup_size(1)
fn main() {
  var v0 = zero;
  var v1 = init;
}
