@compute @workgroup_size(1)
fn f() {
  var a = 1u;
  var b = 0u;
  let r : u32 = (a % b);
}
