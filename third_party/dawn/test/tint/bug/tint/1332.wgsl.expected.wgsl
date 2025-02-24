@compute @workgroup_size(1)
fn compute_main() {
  let a = 1.23999999999999999112;
  var b = max(a, 1.17549435e-38);
}
