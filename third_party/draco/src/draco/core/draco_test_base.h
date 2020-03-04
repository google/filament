// Wrapper for including googletest indirectly. Useful when the location of the
// googletest sources must change depending on build environment and repository
// source location.
#ifndef DRACO_CORE_DRACO_TEST_BASE_H_
#define DRACO_CORE_DRACO_TEST_BASE_H_

static bool FLAGS_update_golden_files;
#include "gtest/gtest.h"
#include "testing/draco_test_config.h"

#endif  // DRACO_CORE_DRACO_TEST_BASE_H_
