enable chromium_internal_graphite;

@group(1) @binding(0) var arg_0 : texture_storage_2d<r8unorm, write>;

fn textureStore_9e5bc2() {
  textureStore(arg_0, vec2<u32>(1u), vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_9e5bc2();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_9e5bc2();
}
