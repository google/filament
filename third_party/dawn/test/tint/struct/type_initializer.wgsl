struct S1 {
  a : i32,
  b : i32,
  c : i32,
  d : i32,
};

struct S2 {
  e : i32,
  f : S1,
};

struct S3 {
  g : i32,
  h : S1,
  i : S2,
};

struct T {
  a : array<i32, 2>,
};

@compute @workgroup_size(1)
fn main() {
  let x : i32 = 42;

  // Test basic usage.
  let empty : S1 = S1();
  let nonempty : S1 = S1(1, 2, 3, 4);
  let nonempty_with_expr : S1 = S1(1, x, x + 1, nonempty.d);

  // Test nested structs.
  let nested_empty : S3 = S3();
  let nested_nonempty : S3 = S3(1, S1(2, 3, 4, 5), S2(6, S1(7, 8, 9, 10)));
  let nested_nonempty_with_expr : S3 =
    S3(1, S1(2, x, x + 1, nested_nonempty.i.f.d), S2(6, nonempty));

  // Test use of constructors as sub-expressions.
  let subexpr_empty : i32 = S1().a;
  let subexpr_nonempty : i32 = S1(1, 2, 3, 4).b;
  let subexpr_nonempty_with_expr : i32 = S1(1, x, x + 1, nonempty.d).c;
  let subexpr_nested_empty : S1 = S2().f;
  let subexpr_nested_nonempty : S1 = S2(1, S1(2, 3, 4, 5)).f;
  let subexpr_nested_nonempty_with_expr : S1 =
    S2(1, S1(2, x, x + 1, nested_nonempty.i.f.d)).f;

  // Test arrays of structs containing arrays.
  let aosoa_empty : array<T, 2> = array<T, 2>();
  let aosoa_nonempty : array<T, 2> =
    array<T, 2>(
      T(array<i32, 2>(1, 2)),
      T(array<i32, 2>(3, 4)),
    );
  let aosoa_nonempty_with_expr : array<T, 2> =
    array<T, 2>(
      T(array<i32, 2>(1, aosoa_nonempty[0].a[0] + 1)),
      aosoa_nonempty[1],
    );
}
