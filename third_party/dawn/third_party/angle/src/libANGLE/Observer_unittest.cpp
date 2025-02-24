//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Observer_unittest:
//   Unit tests for Observers and related classes.

#include <gtest/gtest.h>

#include "libANGLE/Observer.h"

using namespace angle;
using namespace testing;

namespace
{

struct ObserverClass : public ObserverInterface
{
    void onSubjectStateChange(SubjectIndex index, SubjectMessage message) override
    {
        wasNotified = true;
    }
    bool wasNotified = false;
};

// Test that Observer/Subject state change notifications work.
TEST(ObserverTest, BasicUsage)
{
    Subject subject;
    ObserverClass observer;
    ObserverBinding binding(&observer, 0u);

    binding.bind(&subject);
    ASSERT_FALSE(observer.wasNotified);
    subject.onStateChange(SubjectMessage::SubjectChanged);
    ASSERT_TRUE(observer.wasNotified);
}

}  // anonymous namespace
