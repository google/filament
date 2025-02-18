enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<i32, 1024>;

struct SB_RW {
  arg_0 : array<i32, 64>,
}

@group(0) @binding(1) var<storage, read_write> sb_rw : SB_RW;

fn subgroupMatrixLoad_8bf049() -> subgroup_matrix_left<i32, 8, 8> {
  var res : subgroup_matrix_left<i32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<i32, 8, 8>>(&(sb_rw.arg_0), 1u, true, 1u);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixLoad_8bf049(), false, 64);
}
