override O = 0; // Try switching to 'const'

fn f() {
  const smaller_than_any_f32 = 1e-50;
  // When O is an override, the outer index value is not constant, so the
  // value is not calculated at shader-creation time, and does not error.
  //
  // When O is a const, and 'smaller_than_any_f32' *is not* materialized, the
  // outer index value will evaluate to 10000, resulting in an out-of-bounds
  // error.
  //
  // When O is a const, and 'smaller_than_any_f32' *is* materialized,
  // the materialization of 'smaller_than_any_f32' to f32 will evaluate to zero,
  // and so the outer index value will be zero, and we get no error.
  var v = vec2(0)[i32(vec2(smaller_than_any_f32)[O]*1e27*1e27)];
}
