@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba32uint, write>;

fn textureStore_fb9a8f() {
  textureStore(arg_0, 1i, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_fb9a8f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_fb9a8f();
}
