enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<i32, 64>;

fn subgroupMatrixStore_f04d67() {
  var arg_1 = 1u;
  var arg_2 = subgroup_matrix_result<i32, 8, 8>();
  const arg_3 = true;
  var arg_4 = 1u;
  subgroupMatrixStore(&(arg_0), arg_1, arg_2, arg_3, arg_4);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_f04d67();
}
