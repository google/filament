// Copyright 2016 The Draco Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "draco/core/vector_d.h"

#include <sstream>

#include "draco/core/draco_test_base.h"

namespace {

typedef draco::Vector2f Vector2f;
typedef draco::Vector3f Vector3f;
typedef draco::Vector4f Vector4f;
typedef draco::Vector5f Vector5f;
typedef draco::Vector2ui Vector2ui;
typedef draco::Vector3ui Vector3ui;
typedef draco::Vector4ui Vector4ui;
typedef draco::Vector5ui Vector5ui;

typedef draco::VectorD<int32_t, 3> Vector3i;
typedef draco::VectorD<int32_t, 4> Vector4i;

TEST(VectorDTest, TestOperators) {
  {
    const Vector3f v;
    ASSERT_EQ(v[0], 0);
    ASSERT_EQ(v[1], 0);
    ASSERT_EQ(v[2], 0);
  }
  Vector3f v(1, 2, 3);
  ASSERT_EQ(v[0], 1);
  ASSERT_EQ(v[1], 2);
  ASSERT_EQ(v[2], 3);

  Vector3f w = v;
  ASSERT_TRUE(v == w);
  ASSERT_FALSE(v != w);
  ASSERT_EQ(w[0], 1);
  ASSERT_EQ(w[1], 2);
  ASSERT_EQ(w[2], 3);

  w = -v;
  ASSERT_EQ(w[0], -1);
  ASSERT_EQ(w[1], -2);
  ASSERT_EQ(w[2], -3);

  w = v + v;
  ASSERT_EQ(w[0], 2);
  ASSERT_EQ(w[1], 4);
  ASSERT_EQ(w[2], 6);

  w = w - v;
  ASSERT_EQ(w[0], 1);
  ASSERT_EQ(w[1], 2);
  ASSERT_EQ(w[2], 3);

  // Scalar multiplication from left and right.
  w = v * 2.f;
  ASSERT_EQ(w[0], 2);
  ASSERT_EQ(w[1], 4);
  ASSERT_EQ(w[2], 6);
  w = 2.f * v;
  ASSERT_EQ(w[0], 2);
  ASSERT_EQ(w[1], 4);
  ASSERT_EQ(w[2], 6);

  ASSERT_EQ(v.SquaredNorm(), 14);
  ASSERT_EQ(v.Dot(v), 14);

  Vector3f new_v = v;
  new_v.Normalize();
  const float tolerance = 1e-5;
  const float magnitude = std::sqrt(v.SquaredNorm());
  const float new_magnitude = std::sqrt(new_v.SquaredNorm());
  ASSERT_NEAR(new_magnitude, 1, tolerance);
  for (int i = 0; i < 3; ++i) {
    new_v[i] *= magnitude;
    ASSERT_NEAR(new_v[i], v[i], tolerance);
  }

  Vector3f x(0, 0, 0);
  x.Normalize();
  for (int i = 0; i < 3; ++i) {
    ASSERT_EQ(0, x[i]);
  }
}

TEST(VectorDTest, TestAdditionAssignmentOperator) {
  Vector3ui v(1, 2, 3);
  Vector3ui w(4, 5, 6);

  w += v;
  ASSERT_EQ(w[0], 5);
  ASSERT_EQ(w[1], 7);
  ASSERT_EQ(w[2], 9);

  w += w;
  ASSERT_EQ(w[0], 10);
  ASSERT_EQ(w[1], 14);
  ASSERT_EQ(w[2], 18);
}

TEST(VectorDTest, TestSubtractionAssignmentOperator) {
  Vector3ui v(1, 2, 3);
  Vector3ui w(4, 6, 8);

  w -= v;
  ASSERT_EQ(w[0], 3);
  ASSERT_EQ(w[1], 4);
  ASSERT_EQ(w[2], 5);

  w -= w;
  ASSERT_EQ(w[0], 0);
  ASSERT_EQ(w[1], 0);
  ASSERT_EQ(w[2], 0);
}

TEST(VectorDTest, TestMultiplicationAssignmentOperator) {
  Vector3ui v(1, 2, 3);
  Vector3ui w(4, 5, 6);

  w *= v;
  ASSERT_EQ(w[0], 4);
  ASSERT_EQ(w[1], 10);
  ASSERT_EQ(w[2], 18);

  v *= v;
  ASSERT_EQ(v[0], 1);
  ASSERT_EQ(v[1], 4);
  ASSERT_EQ(v[2], 9);
}

TEST(VectorTest, TestGetNormalized) {
  const Vector3f original(2, 3, -4);
  const Vector3f normalized = original.GetNormalized();
  const float magnitude = sqrt(original.SquaredNorm());
  const float tolerance = 1e-5f;
  ASSERT_NEAR(normalized[0], original[0] / magnitude, tolerance);
  ASSERT_NEAR(normalized[1], original[1] / magnitude, tolerance);
  ASSERT_NEAR(normalized[2], original[2] / magnitude, tolerance);
}

TEST(VectorTest, TestGetNormalizedWithZeroLengthVector) {
  const Vector3f original(0, 0, 0);
  const Vector3f normalized = original.GetNormalized();
  ASSERT_EQ(normalized[0], 0);
  ASSERT_EQ(normalized[1], 0);
  ASSERT_EQ(normalized[2], 0);
}

TEST(VectorDTest, TestCrossProduct3D) {
  const Vector3i e1(1, 0, 0);
  const Vector3i e2(0, 1, 0);
  const Vector3i e3(0, 0, 1);
  const Vector3i o(0, 0, 0);
  ASSERT_EQ(e3, draco::CrossProduct(e1, e2));
  ASSERT_EQ(e1, draco::CrossProduct(e2, e3));
  ASSERT_EQ(e2, draco::CrossProduct(e3, e1));
  ASSERT_EQ(-e3, draco::CrossProduct(e2, e1));
  ASSERT_EQ(-e1, draco::CrossProduct(e3, e2));
  ASSERT_EQ(-e2, draco::CrossProduct(e1, e3));
  ASSERT_EQ(o, draco::CrossProduct(e1, e1));
  ASSERT_EQ(o, draco::CrossProduct(e2, e2));
  ASSERT_EQ(o, draco::CrossProduct(e3, e3));

  // Orthogonality of result for some general vectors.
  const Vector3i v1(123, -62, 223);
  const Vector3i v2(734, 244, -13);
  const Vector3i orth = draco::CrossProduct(v1, v2);
  ASSERT_EQ(0, v1.Dot(orth));
  ASSERT_EQ(0, v2.Dot(orth));
}

TEST(VectorDTest, TestAbsSum) {
  // Testing const of function and zero.
  const Vector3i v(0, 0, 0);
  ASSERT_EQ(v.AbsSum(), 0);
  // Testing semantic.
  ASSERT_EQ(Vector3i(0, 0, 0).AbsSum(), 0);
  ASSERT_EQ(Vector3i(1, 2, 3).AbsSum(), 6);
  ASSERT_EQ(Vector3i(-1, -2, -3).AbsSum(), 6);
  ASSERT_EQ(Vector3i(-2, 4, -8).AbsSum(), 14);
  // Other dimension.
  ASSERT_EQ(Vector4i(-2, 4, -8, 3).AbsSum(), 17);
}

TEST(VectorDTest, TestMinMaxCoeff) {
  // Test verifies that MinCoeff() and MaxCoeff() functions work as intended.
  const Vector4i vi(-10, 5, 2, 3);
  ASSERT_EQ(vi.MinCoeff(), -10);
  ASSERT_EQ(vi.MaxCoeff(), 5);

  const Vector3f vf(6.f, 1000.f, -101.f);
  ASSERT_EQ(vf.MinCoeff(), -101.f);
  ASSERT_EQ(vf.MaxCoeff(), 1000.f);
}

TEST(VectorDTest, TestOstream) {
  // Tests that the vector can be stored in a provided std::ostream.
  const draco::VectorD<int64_t, 3> vector(1, 2, 3);
  std::stringstream str;
  str << vector << " ";
  ASSERT_EQ(str.str(), "1 2 3 ");
}

TEST(VectorDTest, TestConvertConstructor) {
  // Tests that a vector can be constructed from another vector with a different
  // type.
  const draco::VectorD<int64_t, 3> vector(1, 2, 3);

  const draco::VectorD<float, 3> vector3f(vector);
  ASSERT_EQ(vector3f, draco::Vector3f(1.f, 2.f, 3.f));

  const draco::VectorD<float, 2> vector2f(vector);
  ASSERT_EQ(vector2f, draco::Vector2f(1.f, 2.f));

  const draco::VectorD<float, 4> vector4f(vector3f);
  ASSERT_EQ(vector4f, draco::Vector4f(1.f, 2.f, 3.f, 0.f));

  const draco::VectorD<double, 1> vector1d(vector3f);
  ASSERT_EQ(vector1d[0], 1.0);
}

TEST(VectorDTest, TestBinaryOps) {
  // Tests the binary multiplication operator of the VectorD class.
  const draco::Vector4f vector_0(1.f, 2.3f, 4.2f, -10.f);
  ASSERT_EQ(vector_0 * draco::Vector4f(1.f, 1.f, 1.f, 1.f), vector_0);
  ASSERT_EQ(vector_0 * draco::Vector4f(0.f, 0.f, 0.f, 0.f),
            draco::Vector4f(0.f, 0.f, 0.f, 0.f));
  ASSERT_EQ(vector_0 * draco::Vector4f(0.1f, 0.2f, 0.3f, 0.4f),
            draco::Vector4f(0.1f, 0.46f, 1.26f, -4.f));
}

}  // namespace
