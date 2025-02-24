@compute @workgroup_size(1)
fn f(@builtin(global_invocation_id) v : vec3<u32>) {
  let l = (v.x << (v.y % 1));
}
