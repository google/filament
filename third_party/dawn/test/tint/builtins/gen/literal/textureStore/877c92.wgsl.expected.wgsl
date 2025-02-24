@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba8snorm, read_write>;

fn textureStore_877c92() {
  textureStore(arg_0, 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_877c92();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_877c92();
}
