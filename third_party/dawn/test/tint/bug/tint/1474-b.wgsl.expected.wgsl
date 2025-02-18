<dawn>/test/tint/bug/tint/1474-b.wgsl:7:5 warning: code is unreachable
    let non_uniform_cond = non_uniform_value == 0;
    ^^^^^^^^^^^^^^^^^^^^

@group(0) @binding(0) var<storage, read_write> non_uniform_value : i32;

@compute @workgroup_size(1, 1, 1)
fn main() {
  return;
  let non_uniform_cond = (non_uniform_value == 0);
  if (non_uniform_cond) {
    workgroupBarrier();
  }
}
