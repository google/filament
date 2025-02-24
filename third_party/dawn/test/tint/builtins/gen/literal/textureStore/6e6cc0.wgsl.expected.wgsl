@group(1) @binding(0) var arg_0 : texture_storage_1d<r32float, read_write>;

fn textureStore_6e6cc0() {
  textureStore(arg_0, 1i, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_6e6cc0();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_6e6cc0();
}
