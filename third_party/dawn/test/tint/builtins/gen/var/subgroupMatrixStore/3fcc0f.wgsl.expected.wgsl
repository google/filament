enable chromium_experimental_subgroup_matrix;

struct SB_RW {
  arg_0 : array<f32, 64>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn subgroupMatrixStore_3fcc0f() {
  var arg_1 = 1u;
  var arg_2 = subgroup_matrix_right<f32, 8, 8>();
  const arg_3 = true;
  var arg_4 = 1u;
  subgroupMatrixStore(&(sb_rw.arg_0), arg_1, arg_2, arg_3, arg_4);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_3fcc0f();
}
