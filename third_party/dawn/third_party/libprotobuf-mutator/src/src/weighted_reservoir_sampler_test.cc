// Copyright 2016 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/weighted_reservoir_sampler.h"

#include <numeric>
#include <tuple>
#include <vector>

#include "port/gtest.h"

using testing::Combine;
using testing::Range;
using testing::TestWithParam;
using testing::ValuesIn;

namespace protobuf_mutator {

class WeightedReservoirSamplerTest
    : public TestWithParam<std::tuple<int, std::vector<int>>> {};

const int kRuns = 1000000;

const std::vector<int> kTests[] = {
    {1},
    {1, 1, 1},
    {1, 1, 0},
    {1, 10, 100},
    {100, 1, 10},
    {1, 10000, 10000},
    {1, 3, 7, 100, 105},
    {93519, 52999, 354,   37837, 55285, 31787, 89096, 55695, 1587,
     18233, 77557, 67632, 59348, 51250, 17417, 96856, 78568, 44296,
     70170, 41328, 9206,  90187, 54086, 35602, 53167, 33791, 60118,
     52962, 10327, 80513, 49526, 18326, 83662, 49644, 70903, 4910,
     36309, 19196, 42982, 53316, 14773, 86607, 60835}};

INSTANTIATE_TEST_SUITE_P(AllTest, WeightedReservoirSamplerTest,
                         Combine(Range(1, 10, 3), ValuesIn(kTests)));

TEST_P(WeightedReservoirSamplerTest, Test) {
  std::vector<int> weights = std::get<1>(GetParam());
  std::vector<int> counts(weights.size(), 0);

  using RandomEngine = std::minstd_rand;
  RandomEngine rand(std::get<0>(GetParam()));
  for (int i = 0; i < kRuns; ++i) {
    WeightedReservoirSampler<int, RandomEngine> sampler(&rand);
    for (size_t j = 0; j < weights.size(); ++j) sampler.Try(weights[j], j);
    ++counts[sampler.selected()];
  }

  int sum = std::accumulate(weights.begin(), weights.end(), 0);
  for (size_t j = 0; j < weights.size(); ++j) {
    float expected = weights[j];
    expected /= sum;

    float actual = counts[j];
    actual /= kRuns;

    EXPECT_NEAR(expected, actual, 0.01);
  }
}

}  // namespace protobuf_mutator
