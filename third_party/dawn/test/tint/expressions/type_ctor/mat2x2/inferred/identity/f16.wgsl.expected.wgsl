enable f16;

var<private> m = mat2x2(mat2x2(0.0h, 1.0h, 2.0h, 3.0h));

@group(0) @binding(0) var<storage, read_write> out : mat2x2<f16>;

@compute @workgroup_size(1)
fn f() {
  out = mat2x2(m);
}
