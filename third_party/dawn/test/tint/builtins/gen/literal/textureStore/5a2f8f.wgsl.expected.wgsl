@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba16sint, write>;

fn textureStore_5a2f8f() {
  textureStore(arg_0, 1i, vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_5a2f8f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_5a2f8f();
}
