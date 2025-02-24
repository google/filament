struct UBO {
  dynamic_idx : i32,
}

@group(0) @binding(0) var<uniform> ubo : UBO;

struct S {
  data : array<i32, 64>,
}

struct Result {
  out : i32,
}

@group(0) @binding(1) var<storage, read_write> result : Result;

@compute @workgroup_size(1)
fn f() {
  var s : S;
  result.out = s.data[ubo.dynamic_idx];
}
