enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn subgroupShuffleXor_e3c10b() -> vec2<u32> {
  var arg_0 = vec2<u32>(1u);
  var arg_1 = 1u;
  var res : vec2<u32> = subgroupShuffleXor(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleXor_e3c10b();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleXor_e3c10b();
}
