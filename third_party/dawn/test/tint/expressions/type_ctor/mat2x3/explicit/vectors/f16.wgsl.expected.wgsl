enable f16;

var<private> m = mat2x3<f16>(vec3<f16>(0.0h, 1.0h, 2.0h), vec3<f16>(3.0h, 4.0h, 5.0h));

@group(0) @binding(0) var<storage, read_write> out : mat2x3<f16>;

@compute @workgroup_size(1)
fn f() {
  out = m;
}
