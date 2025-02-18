var<private> A : array<T, 4>;

alias T = i32;

@fragment
fn f() {
  A[0] = 1;
}
