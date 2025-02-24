@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba8uint, read_write>;

fn textureStore_9d7c62() {
  textureStore(arg_0, 1i, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_9d7c62();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_9d7c62();
}
