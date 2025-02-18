@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rg32sint, read_write>;

fn textureLoad_e2b3a1() -> vec4<i32> {
  var arg_1 = vec2<u32>(1u);
  var arg_2 = 1i;
  var res : vec4<i32> = textureLoad(arg_0, arg_1, arg_2);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_e2b3a1();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_e2b3a1();
}
