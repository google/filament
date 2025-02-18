enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<f32, 1024>;

var<workgroup> arg_0 : array<f32, 64>;

fn subgroupMatrixLoad_c857d1() -> subgroup_matrix_result<f32, 8, 8> {
  var res : subgroup_matrix_result<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>>(&(arg_0), 1u, true, 1u);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixLoad_c857d1(), false, 64);
}
