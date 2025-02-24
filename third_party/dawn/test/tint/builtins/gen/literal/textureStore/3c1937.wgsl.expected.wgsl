@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba8uint, write>;

fn textureStore_3c1937() {
  textureStore(arg_0, 1u, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_3c1937();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_3c1937();
}
