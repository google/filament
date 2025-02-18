var<workgroup> zero : array<i32, 3>;

@compute @workgroup_size(10)
fn main() {
  var v = zero;
}
