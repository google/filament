/**
 * MIT License
 *
 * Copyright (c) 2017 Thibaut Goetghebuer-Planchon <tessil@gmx.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <tsl/robin_growth_policy.h>

#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>
#include <cstddef>
#include <limits>
#include <ratio>
#include <stdexcept>

#include "utils.h"

BOOST_AUTO_TEST_SUITE(test_policy)

using test_types =
    boost::mpl::list<tsl::rh::power_of_two_growth_policy<2>,
                     tsl::rh::power_of_two_growth_policy<4>,
                     tsl::rh::prime_growth_policy, tsl::rh::mod_growth_policy<>,
                     tsl::rh::mod_growth_policy<std::ratio<7, 2>>>;

BOOST_AUTO_TEST_CASE_TEMPLATE(test_policy, Policy, test_types) {
  // Call next_bucket_count() on the policy until we reach its
  // max_bucket_count()
  std::size_t bucket_count = 0;
  Policy policy(bucket_count);

  BOOST_CHECK_EQUAL(policy.bucket_for_hash(0), 0);
  BOOST_CHECK_EQUAL(bucket_count, 0);

#ifndef TSL_RH_NO_EXCEPTIONS
  bool exception_thrown = false;
  try {
    while (true) {
      const std::size_t previous_bucket_count = bucket_count;

      bucket_count = policy.next_bucket_count();
      policy = Policy(bucket_count);

      BOOST_CHECK_EQUAL(policy.bucket_for_hash(0), 0);
      BOOST_CHECK(bucket_count > previous_bucket_count);
    }
  } catch (const std::length_error&) {
    exception_thrown = true;
  }

  BOOST_CHECK(exception_thrown);
#endif
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_policy_min_bucket_count, Policy,
                              test_types) {
  // Check policy when a bucket_count of 0 is asked.
  std::size_t bucket_count = 0;
  Policy policy(bucket_count);

  BOOST_CHECK_EQUAL(policy.bucket_for_hash(0), 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_policy_max_bucket_count, Policy,
                              test_types) {
  // Test a bucket_count equals to the max_bucket_count limit and above
  std::size_t bucket_count = 0;
  Policy policy(bucket_count);

  bucket_count = policy.max_bucket_count();
  Policy policy2(bucket_count);

  bucket_count = std::numeric_limits<std::size_t>::max();
  TSL_RH_CHECK_THROW((Policy(bucket_count)), std::length_error);

  bucket_count = policy.max_bucket_count() + 1;
  TSL_RH_CHECK_THROW((Policy(bucket_count)), std::length_error);
}

BOOST_AUTO_TEST_SUITE_END()
