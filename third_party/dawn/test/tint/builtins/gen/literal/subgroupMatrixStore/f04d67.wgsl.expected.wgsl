enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<i32, 64>;

fn subgroupMatrixStore_f04d67() {
  subgroupMatrixStore(&(arg_0), 1u, subgroup_matrix_result<i32, 8, 8>(), true, 1u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_f04d67();
}
