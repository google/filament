enable chromium_internal_graphite;

@group(1) @binding(0) var arg_0 : texture_storage_1d<r8unorm, read_write>;

fn textureStore_a7fc47() {
  textureStore(arg_0, 1u, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_a7fc47();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_a7fc47();
}
