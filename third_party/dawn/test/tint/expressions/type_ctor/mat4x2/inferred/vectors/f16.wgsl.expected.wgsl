enable f16;

var<private> m = mat4x2(vec2<f16>(0.0h, 1.0h), vec2<f16>(2.0h, 3.0h), vec2<f16>(4.0h, 5.0h), vec2<f16>(6.0h, 7.0h));

@group(0) @binding(0) var<storage, read_write> out : mat4x2<f16>;

@compute @workgroup_size(1)
fn f() {
  out = m;
}
