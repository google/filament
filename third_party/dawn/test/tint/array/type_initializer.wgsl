@group(0) @binding(0) var<storage, read_write> s: i32;

@compute @workgroup_size(1)
fn main() {
  let x : i32 = 42;

  // Test basic usage.
  let empty : array<i32, 4> = array<i32, 4>();
  let nonempty : array<i32, 4> = array<i32, 4>(1, 2, 3, 4);
  let nonempty_with_expr : array<i32, 4> =
    array<i32, 4>(1, x, x + 1, nonempty[3]);

  // Test nested arrays.
  let nested_empty : array<array<array<i32, 4>, 3>, 2> =
    array<array<array<i32, 4>, 3>, 2>();
  let nested_nonempty : array<array<array<i32, 4>, 3>, 2> =
    array<array<array<i32, 4>, 3>, 2>(
      array<array<i32, 4>, 3>(
        array<i32, 4>(1, 2, 3, 4),
        array<i32, 4>(5, 6, 7, 8),
        array<i32, 4>(9, 10, 11, 12)),
      array<array<i32, 4>, 3>(
        array<i32, 4>(13, 14, 15, 16),
        array<i32, 4>(17, 18, 19, 20),
        array<i32, 4>(21, 22, 23, 24)));
  let nested_nonempty_with_expr : array<array<array<i32, 4>, 3>, 2> =
    array<array<array<i32, 4>, 3>, 2>(
      array<array<i32, 4>, 3>(
        array<i32, 4>(1, 2, x, x + 1),
        array<i32, 4>(5, 6, nonempty[2], nonempty[3] + 1),
        nonempty),
      nested_nonempty[1]);

  // Test use of constructors as sub-expressions.
  let subexpr_empty : i32 = array<i32, 4>()[1];
  let subexpr_nonempty : i32 = array<i32, 4>(1, 2, 3, 4)[2];
  let subexpr_nonempty_with_expr : i32 =
    array<i32, 4>(1, x, x + 1, nonempty[3])[2];
  let subexpr_nested_empty : array<i32, 4> = array<array<i32, 4>, 2>()[1];
  let subexpr_nested_nonempty : array<i32, 4> =
    array<array<i32, 4>, 2>(
      array<i32, 4>(1, 2, 3, 4),
      array<i32, 4>(5, 6, 7, 8)
    )[1];
  let subexpr_nested_nonempty_with_expr : array<i32, 4> =
    array<array<i32, 4>, 2>(
      array<i32, 4>(1, x, x + 1, nonempty[3]),
      nested_nonempty[1][2],
    )[1];


  s = empty[0] + nonempty[0] + nonempty_with_expr[0] + nested_empty[0][0][0] +
    nested_nonempty[0][0][0] + nested_nonempty_with_expr[0][0][0] +
    subexpr_empty + subexpr_nonempty + subexpr_nonempty_with_expr + subexpr_nested_empty[0] +
    subexpr_nested_nonempty[0] + subexpr_nested_nonempty_with_expr[0];
}
