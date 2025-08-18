// RUN: %dxc -Tlib_6_3 -HV 2018 -Wno-unused-value   -verify %s

void run() {
  RAY_FLAG rayFlags =
    RAY_FLAG_NONE                            +
    RAY_FLAG_FORCE_OPAQUE                    +
    RAY_FLAG_FORCE_NON_OPAQUE                +
    RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH +
    RAY_FLAG_SKIP_CLOSEST_HIT_SHADER         +
    RAY_FLAG_CULL_BACK_FACING_TRIANGLES      +
    RAY_FLAG_CULL_FRONT_FACING_TRIANGLES     +
    RAY_FLAG_CULL_OPAQUE                     +
    RAY_FLAG_CULL_NON_OPAQUE;

  rayFlags += RAY_FLAG_INVALID;                             /* expected-error {{use of undeclared identifier 'RAY_FLAG_INVALID'; did you mean 'RAY_FLAG_NONE'?}} */

  int intFlag = RAY_FLAG_CULL_OPAQUE;

  int hitKindFlag =
    HIT_KIND_TRIANGLE_FRONT_FACE + HIT_KIND_TRIANGLE_BACK_FACE;

  hitKindFlag += HIT_KIND_INVALID;                          /* expected-error {{use of undeclared identifier 'HIT_KIND_INVALID'; did you mean 'HIT_KIND_NONE'?}} */


  BuiltInTriangleIntersectionAttributes attr;
  attr.barycentrics = float2(0.3f, 0.4f);
  attr.barycentrics.z = 3.0f;                               /* expected-error {{vector swizzle 'z' is out of bounds}} */
}
