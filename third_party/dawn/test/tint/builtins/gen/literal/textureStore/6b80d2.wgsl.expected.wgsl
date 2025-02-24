@group(1) @binding(0) var arg_0 : texture_storage_1d<r32sint, write>;

fn textureStore_6b80d2() {
  textureStore(arg_0, 1i, vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_6b80d2();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_6b80d2();
}
