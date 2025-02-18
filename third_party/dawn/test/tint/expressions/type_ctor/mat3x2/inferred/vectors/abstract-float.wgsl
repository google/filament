const m = mat3x2(vec2(0.0, 1.0),
                 vec2(2.0, 3.0),
                 vec2(4.0, 5.0));

@group(0) @binding(0)
var<storage, read_write> out : mat3x2<f32>;

@compute @workgroup_size(1)
fn f() {
  out = m;
}
