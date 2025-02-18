struct UBO {
  dynamic_idx: i32,
};
@group(0) @binding(0) var<uniform> ubo: UBO;
struct Result {
  out: i32,
};
@group(0) @binding(2) var<storage, read_write> result: Result;

struct SSBO {
  data: array<i32, 4>,
};
@group(0) @binding(1) var<storage, read_write> ssbo: SSBO;

@compute @workgroup_size(1)
fn f() {
  result.out = ssbo.data[ubo.dynamic_idx];
}
