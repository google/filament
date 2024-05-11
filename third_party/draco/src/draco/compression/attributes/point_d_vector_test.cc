// Copyright 2018 The Draco Authors.
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
#include "draco/compression/attributes/point_d_vector.h"

#include "draco/compression/point_cloud/algorithms/point_cloud_types.h"
#include "draco/core/draco_test_base.h"

namespace draco {

class PointDVectorTest : public ::testing::Test {
 protected:
  template <typename PT>
  void TestIntegrity() {}
  template <typename PT>
  void TestSize() {
    for (uint32_t n_items = 0; n_items <= 10; ++n_items) {
      for (uint32_t dimensionality = 1; dimensionality <= 10;
           ++dimensionality) {
        draco::PointDVector<PT> var(n_items, dimensionality);
        ASSERT_EQ(n_items, var.size());
        ASSERT_EQ(n_items * dimensionality, var.GetBufferSize());
      }
    }
  }
  template <typename PT>
  void TestContentsContiguous() {
    for (uint32_t n_items = 1; n_items <= 1000; n_items *= 10) {
      for (uint32_t dimensionality = 1; dimensionality < 10;
           dimensionality += 2) {
        for (uint32_t att_dimensionality = 1;
             att_dimensionality <= dimensionality; att_dimensionality += 2) {
          for (uint32_t offset_dimensionality = 0;
               offset_dimensionality < dimensionality - att_dimensionality;
               ++offset_dimensionality) {
            PointDVector<PT> var(n_items, dimensionality);

            std::vector<PT> att(n_items * att_dimensionality);
            for (PT val = 0; val < n_items; val += 1) {
              for (PT att_dim = 0; att_dim < att_dimensionality; att_dim += 1) {
                att[val * att_dimensionality + att_dim] = val;
              }
            }
            const PT *const attribute_data = att.data();

            var.CopyAttribute(att_dimensionality, offset_dimensionality,
                              attribute_data);

            for (PT val = 0; val < n_items; val += 1) {
              for (PT att_dim = 0; att_dim < att_dimensionality; att_dim += 1) {
                ASSERT_EQ(var[val][offset_dimensionality + att_dim], val);
              }
            }
          }
        }
      }
    }
  }
  template <typename PT>
  void TestContentsDiscrete() {
    for (uint32_t n_items = 1; n_items <= 1000; n_items *= 10) {
      for (uint32_t dimensionality = 1; dimensionality < 10;
           dimensionality += 2) {
        for (uint32_t att_dimensionality = 1;
             att_dimensionality <= dimensionality; att_dimensionality += 2) {
          for (uint32_t offset_dimensionality = 0;
               offset_dimensionality < dimensionality - att_dimensionality;
               ++offset_dimensionality) {
            PointDVector<PT> var(n_items, dimensionality);

            std::vector<PT> att(n_items * att_dimensionality);
            for (PT val = 0; val < n_items; val += 1) {
              for (PT att_dim = 0; att_dim < att_dimensionality; att_dim += 1) {
                att[val * att_dimensionality + att_dim] = val;
              }
            }
            const PT *const attribute_data = att.data();

            for (PT item = 0; item < n_items; item += 1) {
              var.CopyAttribute(att_dimensionality, offset_dimensionality, item,
                                attribute_data + item * att_dimensionality);
            }

            for (PT val = 0; val < n_items; val += 1) {
              for (PT att_dim = 0; att_dim < att_dimensionality; att_dim += 1) {
                ASSERT_EQ(var[val][offset_dimensionality + att_dim], val);
              }
            }
          }
        }
      }
    }
  }

  template <typename PT>
  void TestContentsCopy() {
    for (uint32_t n_items = 1; n_items <= 1000; n_items *= 10) {
      for (uint32_t dimensionality = 1; dimensionality < 10;
           dimensionality += 2) {
        for (uint32_t att_dimensionality = 1;
             att_dimensionality <= dimensionality; att_dimensionality += 2) {
          for (uint32_t offset_dimensionality = 0;
               offset_dimensionality < dimensionality - att_dimensionality;
               ++offset_dimensionality) {
            PointDVector<PT> var(n_items, dimensionality);
            PointDVector<PT> dest(n_items, dimensionality);

            std::vector<PT> att(n_items * att_dimensionality);
            for (PT val = 0; val < n_items; val += 1) {
              for (PT att_dim = 0; att_dim < att_dimensionality; att_dim += 1) {
                att[val * att_dimensionality + att_dim] = val;
              }
            }
            const PT *const attribute_data = att.data();

            var.CopyAttribute(att_dimensionality, offset_dimensionality,
                              attribute_data);

            for (PT item = 0; item < n_items; item += 1) {
              dest.CopyItem(var, item, item);
            }

            for (PT val = 0; val < n_items; val += 1) {
              for (PT att_dim = 0; att_dim < att_dimensionality; att_dim += 1) {
                ASSERT_EQ(var[val][offset_dimensionality + att_dim], val);
                ASSERT_EQ(dest[val][offset_dimensionality + att_dim], val);
              }
            }
          }
        }
      }
    }
  }
  template <typename PT>
  void TestIterator() {
    for (uint32_t n_items = 1; n_items <= 1000; n_items *= 10) {
      for (uint32_t dimensionality = 1; dimensionality < 10;
           dimensionality += 2) {
        for (uint32_t att_dimensionality = 1;
             att_dimensionality <= dimensionality; att_dimensionality += 2) {
          for (uint32_t offset_dimensionality = 0;
               offset_dimensionality < dimensionality - att_dimensionality;
               ++offset_dimensionality) {
            PointDVector<PT> var(n_items, dimensionality);
            PointDVector<PT> dest(n_items, dimensionality);

            std::vector<PT> att(n_items * att_dimensionality);
            for (PT val = 0; val < n_items; val += 1) {
              for (PT att_dim = 0; att_dim < att_dimensionality; att_dim += 1) {
                att[val * att_dimensionality + att_dim] = val;
              }
            }
            const PT *const attribute_data = att.data();

            var.CopyAttribute(att_dimensionality, offset_dimensionality,
                              attribute_data);

            for (PT item = 0; item < n_items; item += 1) {
              dest.CopyItem(var, item, item);
            }

            auto V0 = var.begin();
            auto VE = var.end();
            auto D0 = dest.begin();
            auto DE = dest.end();

            while (V0 != VE && D0 != DE) {
              ASSERT_EQ(*D0, *V0);  // compare PseudoPointD
              // verify elemental values
              for (auto index = 0; index < dimensionality; index += 1) {
                ASSERT_EQ((*D0)[index], (*V0)[index]);
              }
              ++V0;
              ++D0;
            }

            for (PT val = 0; val < n_items; val += 1) {
              for (PT att_dim = 0; att_dim < att_dimensionality; att_dim += 1) {
                ASSERT_EQ(var[val][offset_dimensionality + att_dim], val);
                ASSERT_EQ(dest[val][offset_dimensionality + att_dim], val);
              }
            }
          }
        }
      }
    }
  }
  template <typename PT>
  void TestPoint3Iterator() {
    for (uint32_t n_items = 1; n_items <= 1000; n_items *= 10) {
      const uint32_t dimensionality = 3;
      // for (uint32_t dimensionality = 1; dimensionality < 10;
      //      dimensionality += 2) {
      const uint32_t att_dimensionality = 3;
      // for (uint32_t att_dimensionality = 1;
      //      att_dimensionality <= dimensionality; att_dimensionality += 2) {
      for (uint32_t offset_dimensionality = 0;
           offset_dimensionality < dimensionality - att_dimensionality;
           ++offset_dimensionality) {
        PointDVector<PT> var(n_items, dimensionality);
        PointDVector<PT> dest(n_items, dimensionality);

        std::vector<PT> att(n_items * att_dimensionality);
        std::vector<draco::Point3ui> att3(n_items);
        for (PT val = 0; val < n_items; val += 1) {
          att3[val][0] = val;
          att3[val][1] = val;
          att3[val][2] = val;
          for (PT att_dim = 0; att_dim < att_dimensionality; att_dim += 1) {
            att[val * att_dimensionality + att_dim] = val;
          }
        }
        const PT *const attribute_data = att.data();

        var.CopyAttribute(att_dimensionality, offset_dimensionality,
                          attribute_data);

        for (PT item = 0; item < n_items; item += 1) {
          dest.CopyItem(var, item, item);
        }

        auto aV0 = att3.begin();
        auto aVE = att3.end();
        auto V0 = var.begin();
        auto VE = var.end();
        auto D0 = dest.begin();
        auto DE = dest.end();

        while (aV0 != aVE && V0 != VE && D0 != DE) {
          ASSERT_EQ(*D0, *V0);  // compare PseudoPointD
          // verify elemental values
          for (auto index = 0; index < dimensionality; index += 1) {
            ASSERT_EQ((*D0)[index], (*V0)[index]);
            ASSERT_EQ((*D0)[index], (*aV0)[index]);
            ASSERT_EQ((*aV0)[index], (*V0)[index]);
          }
          ++aV0;
          ++V0;
          ++D0;
        }

        for (PT val = 0; val < n_items; val += 1) {
          for (PT att_dim = 0; att_dim < att_dimensionality; att_dim += 1) {
            ASSERT_EQ(var[val][offset_dimensionality + att_dim], val);
            ASSERT_EQ(dest[val][offset_dimensionality + att_dim], val);
          }
        }
      }
    }
  }

  void TestPseudoPointDSwap() {
    draco::Point3ui val = {0, 1, 2};
    draco::Point3ui dest = {10, 11, 12};
    draco::PseudoPointD<uint32_t> val_src1(&val[0], 3);
    draco::PseudoPointD<uint32_t> dest_src1(&dest[0], 3);

    ASSERT_EQ(val_src1[0], 0);
    ASSERT_EQ(val_src1[1], 1);
    ASSERT_EQ(val_src1[2], 2);
    ASSERT_EQ(dest_src1[0], 10);
    ASSERT_EQ(dest_src1[1], 11);
    ASSERT_EQ(dest_src1[2], 12);

    ASSERT_NE(val_src1, dest_src1);

    swap(val_src1, dest_src1);

    ASSERT_EQ(dest_src1[0], 0);
    ASSERT_EQ(dest_src1[1], 1);
    ASSERT_EQ(dest_src1[2], 2);
    ASSERT_EQ(val_src1[0], 10);
    ASSERT_EQ(val_src1[1], 11);
    ASSERT_EQ(val_src1[2], 12);

    ASSERT_NE(val_src1, dest_src1);
  }
  void TestPseudoPointDEquality() {
    draco::Point3ui val = {0, 1, 2};
    draco::Point3ui dest = {0, 1, 2};
    draco::PseudoPointD<uint32_t> val_src1(&val[0], 3);
    draco::PseudoPointD<uint32_t> val_src2(&val[0], 3);
    draco::PseudoPointD<uint32_t> dest_src1(&dest[0], 3);
    draco::PseudoPointD<uint32_t> dest_src2(&dest[0], 3);

    ASSERT_EQ(val_src1, val_src1);
    ASSERT_EQ(val_src1, val_src2);
    ASSERT_EQ(dest_src1, val_src1);
    ASSERT_EQ(dest_src1, val_src2);
    ASSERT_EQ(val_src2, val_src1);
    ASSERT_EQ(val_src2, val_src2);
    ASSERT_EQ(dest_src2, val_src1);
    ASSERT_EQ(dest_src2, val_src2);

    for (auto i = 0; i < 3; i++) {
      ASSERT_EQ(val_src1[i], val_src1[i]);
      ASSERT_EQ(val_src1[i], val_src2[i]);
      ASSERT_EQ(dest_src1[i], val_src1[i]);
      ASSERT_EQ(dest_src1[i], val_src2[i]);
      ASSERT_EQ(val_src2[i], val_src1[i]);
      ASSERT_EQ(val_src2[i], val_src2[i]);
      ASSERT_EQ(dest_src2[i], val_src1[i]);
      ASSERT_EQ(dest_src2[i], val_src2[i]);
    }
  }
  void TestPseudoPointDInequality() {
    draco::Point3ui val = {0, 1, 2};
    draco::Point3ui dest = {1, 2, 3};
    draco::PseudoPointD<uint32_t> val_src1(&val[0], 3);
    draco::PseudoPointD<uint32_t> val_src2(&val[0], 3);
    draco::PseudoPointD<uint32_t> dest_src1(&dest[0], 3);
    draco::PseudoPointD<uint32_t> dest_src2(&dest[0], 3);

    ASSERT_EQ(val_src1, val_src1);
    ASSERT_EQ(val_src1, val_src2);
    ASSERT_NE(dest_src1, val_src1);
    ASSERT_NE(dest_src1, val_src2);
    ASSERT_EQ(val_src2, val_src1);
    ASSERT_EQ(val_src2, val_src2);
    ASSERT_NE(dest_src2, val_src1);
    ASSERT_NE(dest_src2, val_src2);

    for (auto i = 0; i < 3; i++) {
      ASSERT_EQ(val_src1[i], val_src1[i]);
      ASSERT_EQ(val_src1[i], val_src2[i]);
      ASSERT_NE(dest_src1[i], val_src1[i]);
      ASSERT_NE(dest_src1[i], val_src2[i]);
      ASSERT_EQ(val_src2[i], val_src1[i]);
      ASSERT_EQ(val_src2[i], val_src2[i]);
      ASSERT_NE(dest_src2[i], val_src1[i]);
      ASSERT_NE(dest_src2[i], val_src2[i]);
    }
  }
};

TEST_F(PointDVectorTest, VectorTest) {
  TestSize<uint32_t>();
  TestContentsDiscrete<uint32_t>();
  TestContentsContiguous<uint32_t>();
  TestContentsCopy<uint32_t>();
  TestIterator<uint32_t>();
  TestPoint3Iterator<uint32_t>();
}
TEST_F(PointDVectorTest, PseudoPointDTest) {
  TestPseudoPointDSwap();
  TestPseudoPointDEquality();
  TestPseudoPointDInequality();
}
}  // namespace draco
