@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn dpdyFine_df33aa() -> vec2<f32> {
  var res : vec2<f32> = dpdyFine(vec2<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdyFine_df33aa();
}
