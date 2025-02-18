struct SSBO {
  m : mat2x2<f32>,
}

@group(0) @binding(0) var<storage, read_write> ssbo : SSBO;

@compute @workgroup_size(1)
fn f() {
  let v = ssbo.m;
  ssbo.m = v;
}
