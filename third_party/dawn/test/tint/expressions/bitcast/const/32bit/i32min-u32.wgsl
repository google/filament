@compute @workgroup_size(1)
fn f() {
  let b : u32 = bitcast<u32>(-2147483648);
}
