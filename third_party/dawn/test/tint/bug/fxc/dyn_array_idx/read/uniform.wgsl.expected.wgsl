struct UBO {
  data : array<vec4<i32>, 4>,
  dynamic_idx : i32,
}

@group(0) @binding(0) var<uniform> ubo : UBO;

struct Result {
  out : i32,
}

@group(0) @binding(2) var<storage, read_write> result : Result;

@compute @workgroup_size(1)
fn f() {
  result.out = ubo.data[ubo.dynamic_idx].x;
}
