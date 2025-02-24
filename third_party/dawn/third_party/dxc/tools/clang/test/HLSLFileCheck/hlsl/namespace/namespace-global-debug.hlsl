// RUN: %dxc -E main -T vs_6_0 -Od %s | FileCheck %s

// CHECK: sc_Arr2D@foo{{.*}} = internal constant [9 x float] [float

namespace foo
{
  static const float a = 0.27901;
  static const float b = 0.44198;
  static const float c = -0.27901;
  static const float sc_Arr2D[3][3] =
  {
      { a * a, a * b, a * c },
      { b * a, b * b, b * c },
      { c * a, c * b, c * c },
  };
}

float main(int i : IN) : OUT {
  return foo::sc_Arr2D[i / 3][i % 3];
}
