@group(1) @binding(0) var arg_0 : texture_storage_1d<r32float, write>;

fn textureStore_2ac6c7() {
  textureStore(arg_0, 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_2ac6c7();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_2ac6c7();
}
