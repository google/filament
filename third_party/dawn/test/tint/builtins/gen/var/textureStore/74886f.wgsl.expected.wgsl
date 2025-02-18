enable chromium_internal_graphite;

@group(1) @binding(0) var arg_0 : texture_storage_1d<r8unorm, write>;

fn textureStore_74886f() {
  var arg_1 = 1i;
  var arg_2 = vec4<f32>(1.0f);
  textureStore(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  textureStore_74886f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_74886f();
}
