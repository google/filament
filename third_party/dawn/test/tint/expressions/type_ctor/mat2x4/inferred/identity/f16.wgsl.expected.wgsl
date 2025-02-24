enable f16;

var<private> m = mat2x4(mat2x4(0.0h, 1.0h, 2.0h, 3.0h, 4.0h, 5.0h, 6.0h, 7.0h));

@group(0) @binding(0) var<storage, read_write> out : mat2x4<f16>;

@compute @workgroup_size(1)
fn f() {
  out = mat2x4(m);
}
