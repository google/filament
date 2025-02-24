enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<i32, 64>;

fn subgroupMatrixStore_5671e2() {
  subgroupMatrixStore(&(arg_0), 1u, subgroup_matrix_right<i32, 8, 8>(), true, 1u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_5671e2();
}
