enable chromium_experimental_subgroup_matrix;

struct SB_RW {
  arg_0 : array<i32>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn subgroupMatrixStore_3ea76e() {
  subgroupMatrixStore(&(sb_rw.arg_0), 1u, subgroup_matrix_left<i32, 8, 8>(), true, 1u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_3ea76e();
}
