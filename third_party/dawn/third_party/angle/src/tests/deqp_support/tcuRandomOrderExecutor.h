/*-------------------------------------------------------------------------
 * drawElements Quality Program Tester Core
 * ----------------------------------------
 *
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */ /*!
 * \file
 * \brief Generic main().
 */ /*--------------------------------------------------------------------*/

#ifndef TCU_RANDOM_ORDER_EXECUTOR_H_
#define TCU_RANDOM_ORDER_EXECUTOR_H_

#include "deUniquePtr.hpp"
#include "tcuTestHierarchyIterator.hpp"

#include "tests/test_utils/RenderDoc.h"

namespace tcu
{

class RandomOrderExecutor
{
  public:
    RandomOrderExecutor(TestPackageRoot &root, TestContext &testCtx, bool enableRenderDocCapture);
    ~RandomOrderExecutor(void);

    TestStatus execute(const std::string &path);

  private:
    void pruneStack(size_t newStackSize);
    TestCase *seekToCase(const std::string &path);

    TestStatus executeInner(TestCase *testCase, const std::string &casePath);

    struct NodeStackEntry
    {
        TestNode *node;
        std::vector<TestNode *> children;

        NodeStackEntry(void) : node(DE_NULL) {}
        NodeStackEntry(TestNode *node_) : node(node_) {}
    };

    TestContext &m_testCtx;

    DefaultHierarchyInflater m_inflater;
    std::vector<NodeStackEntry> m_nodeStack;

    de::MovePtr<TestCaseExecutor> m_caseExecutor;

    RenderDoc mRenderDoc;
};

}  // namespace tcu

#endif  // TCU_RANDOM_ORDER_EXECUTOR_H_
