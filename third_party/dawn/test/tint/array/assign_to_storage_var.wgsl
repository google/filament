alias ArrayType = array<vec4<i32>, 4>;

struct S {
  arr : ArrayType,
};

struct S_nested {
  arr : array<array<array<i32, 2>, 3>, 4>,
};

var<private> src_private : ArrayType;
var<workgroup> src_workgroup : ArrayType;
@group(0) @binding(0) var<uniform> src_uniform : S;
@group(0) @binding(1) var<storage, read_write> src_storage : S;

@group(0) @binding(2) var<storage, read_write> dst : S;
@group(0) @binding(3) var<storage, read_write> dst_nested : S_nested;

fn ret_arr() -> ArrayType {
  return ArrayType();
}

fn ret_struct_arr() -> S {
  return S();
}

fn foo(src_param : ArrayType) {
  var src_function : ArrayType;

  // Assign from value constructor.
  dst.arr = ArrayType(vec4(1), vec4(2), vec4(3), vec4(3));

  // Assign from parameter.
  dst.arr = src_param;

  // Assign from function call.
  dst.arr = ret_arr();

  // Assign from constant.
  let src_let : ArrayType = ArrayType();
  dst.arr = src_let;

  // Assign from var, various address spaces.
  dst.arr = src_function;
  dst.arr = src_private;
  dst.arr = src_workgroup;

  // Assign from struct.arr, various address spaces.
  dst.arr = ret_struct_arr().arr;
  dst.arr = src_uniform.arr;
  dst.arr = src_storage.arr;

  // Nested assignment.
  var src_nested : array<array<array<i32, 2>, 3>, 4>;
  dst_nested.arr = src_nested;
}

@compute @workgroup_size(1)
fn main() {
  let ary = ArrayType();
  foo(ary);
}
