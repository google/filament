@group(1) @binding(0) var arg_0 : texture_storage_1d<rg32uint, write>;

fn textureStore_83bcc1() {
  textureStore(arg_0, 1i, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_83bcc1();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_83bcc1();
}
