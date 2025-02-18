@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba16sint, write>;

fn textureStore_8c76e9() {
  textureStore(arg_0, 1u, vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_8c76e9();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_8c76e9();
}
