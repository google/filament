@group(1) @binding(0) var arg_0 : texture_storage_1d<r32uint, read_write>;

fn textureStore_8676c9() {
  textureStore(arg_0, 1i, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_8676c9();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_8676c9();
}
