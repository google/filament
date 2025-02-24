@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba8snorm, write>;

fn textureStore_959d94() {
  textureStore(arg_0, 1u, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_959d94();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_959d94();
}
