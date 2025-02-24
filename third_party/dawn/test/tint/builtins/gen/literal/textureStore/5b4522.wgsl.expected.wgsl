@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba8uint, read_write>;

fn textureStore_5b4522() {
  textureStore(arg_0, 1u, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_5b4522();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_5b4522();
}
