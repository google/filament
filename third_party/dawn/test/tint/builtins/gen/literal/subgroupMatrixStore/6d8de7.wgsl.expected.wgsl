enable chromium_experimental_subgroup_matrix;
enable f16;

struct SB_RW {
  arg_0 : array<f16>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn subgroupMatrixStore_6d8de7() {
  subgroupMatrixStore(&(sb_rw.arg_0), 1u, subgroup_matrix_right<f16, 8, 8>(), true, 1u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_6d8de7();
}
