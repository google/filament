@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba8sint, read_write>;

fn textureStore_976636() {
  textureStore(arg_0, 1u, vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_976636();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_976636();
}
