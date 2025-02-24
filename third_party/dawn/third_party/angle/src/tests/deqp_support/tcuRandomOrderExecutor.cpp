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
 *//*!
 * \file
 * \brief Executor which can run randomly accessed tests.
 *//*--------------------------------------------------------------------*/

#include "tcuRandomOrderExecutor.h"

#include "deClock.h"
#include "deStringUtil.hpp"
#include "tcuCommandLine.hpp"
#include "tcuTestLog.hpp"

#include <algorithm>
#include <cstdio>
#include <fstream>

using std::string;
using std::vector;

namespace tcu
{

RandomOrderExecutor::RandomOrderExecutor(TestPackageRoot &root,
                                         TestContext &testCtx,
                                         bool enableRenderDocCapture)
    : m_testCtx(testCtx), m_inflater(testCtx)
{
    m_nodeStack.push_back(NodeStackEntry(&root));
    root.getChildren(m_nodeStack[0].children);

    if (enableRenderDocCapture)
    {
        mRenderDoc.attach();
    }
}

RandomOrderExecutor::~RandomOrderExecutor(void)
{
    pruneStack(1);
}

void RandomOrderExecutor::pruneStack(size_t newStackSize)
{
    // \note Root is not managed by us
    DE_ASSERT(de::inRange(newStackSize, size_t(1), m_nodeStack.size()));

    while (m_nodeStack.size() > newStackSize)
    {
        NodeStackEntry &curEntry = m_nodeStack.back();
        const bool isPkg         = curEntry.node->getNodeType() == NODETYPE_PACKAGE;

        DE_ASSERT((m_nodeStack.size() == 2) == isPkg);

        if (curEntry.node)  // Just in case we are in
                            // cleanup path after partial
                            // prune
        {
            if (curEntry.node->getNodeType() == NODETYPE_GROUP)
                m_inflater.leaveGroupNode(static_cast<TestCaseGroup *>(curEntry.node));
            else if (curEntry.node->getNodeType() == NODETYPE_PACKAGE)
                m_inflater.leaveTestPackage(static_cast<TestPackage *>(curEntry.node));
            else
                DE_ASSERT(curEntry.children.empty());

            curEntry.node = DE_NULL;
            curEntry.children.clear();
        }

        if (isPkg)
            m_caseExecutor.clear();

        m_nodeStack.pop_back();
    }
}

static TestNode *findNodeByName(vector<TestNode *> &nodes, const std::string &name)
{
    for (vector<TestNode *>::const_iterator node = nodes.begin(); node != nodes.end(); ++node)
    {
        if (name == (*node)->getName())
            return *node;
    }

    return DE_NULL;
}

TestCase *RandomOrderExecutor::seekToCase(const string &path)
{
    const vector<string> components = de::splitString(path, '.');
    size_t compNdx;

    DE_ASSERT(!m_nodeStack.empty() && m_nodeStack.front().node->getNodeType() == NODETYPE_ROOT);

    for (compNdx = 0; compNdx < components.size(); compNdx++)
    {
        const size_t stackPos = compNdx + 1;

        if (stackPos >= m_nodeStack.size())
            break;  // Stack end
        else if (components[compNdx] != m_nodeStack[stackPos].node->getName())
        {
            // Current stack doesn't match any more, prune.
            pruneStack(stackPos);
            break;
        }
    }

    DE_ASSERT(m_nodeStack.size() == compNdx + 1);

    for (; compNdx < components.size(); compNdx++)
    {
        const size_t parentStackPos = compNdx;
        TestNode *const curNode =
            findNodeByName(m_nodeStack[parentStackPos].children, components[compNdx]);

        if (!curNode)
            throw Exception(string("Test hierarchy node not found: ") + path);

        m_nodeStack.push_back(NodeStackEntry(curNode));

        if (curNode->getNodeType() == NODETYPE_PACKAGE)
        {
            TestPackage *const testPackage = static_cast<TestPackage *>(curNode);

            m_inflater.enterTestPackage(testPackage, m_nodeStack.back().children);
            DE_ASSERT(!m_caseExecutor);
            m_caseExecutor = de::MovePtr<TestCaseExecutor>(testPackage->createExecutor());
        }
        else if (curNode->getNodeType() == NODETYPE_GROUP)
            m_inflater.enterGroupNode(static_cast<TestCaseGroup *>(curNode),
                                      m_nodeStack.back().children);
        else if (compNdx + 1 != components.size())
            throw Exception(string("Invalid test hierarchy path: ") + path);
    }

    DE_ASSERT(m_nodeStack.size() == components.size() + 1);

    if (isTestNodeTypeExecutable(m_nodeStack.back().node->getNodeType()))
        return dynamic_cast<TestCase *>(m_nodeStack.back().node);
    else
        throw Exception(string("Not a test case: ") + path);
}

static qpTestCaseType nodeTypeToTestCaseType(TestNodeType nodeType)
{
    switch (nodeType)
    {
        case NODETYPE_SELF_VALIDATE:
            return QP_TEST_CASE_TYPE_SELF_VALIDATE;
        case NODETYPE_PERFORMANCE:
            return QP_TEST_CASE_TYPE_PERFORMANCE;
        case NODETYPE_CAPABILITY:
            return QP_TEST_CASE_TYPE_CAPABILITY;
        case NODETYPE_ACCURACY:
            return QP_TEST_CASE_TYPE_ACCURACY;
        default:
            DE_ASSERT(false);
            return QP_TEST_CASE_TYPE_LAST;
    }
}

TestStatus RandomOrderExecutor::execute(const std::string &casePath)
{
    TestCase *const testCase      = seekToCase(casePath);
    TestLog &log                  = m_testCtx.getLog();
    const qpTestCaseType caseType = nodeTypeToTestCaseType(testCase->getNodeType());

    m_testCtx.setTerminateAfter(false);
    log.startCase(casePath.c_str(), caseType);

    {
        const TestStatus result = executeInner(testCase, casePath);
        log.endCase(result.getCode(), result.getDescription().c_str());
        return result;
    }
}

tcu::TestStatus RandomOrderExecutor::executeInner(TestCase *testCase, const std::string &casePath)
{
    TestLog &log                 = m_testCtx.getLog();
    const uint64_t testStartTime = deGetMicroseconds();

    m_testCtx.setTestResult(QP_TEST_RESULT_LAST, "");

    // Initialize, will return immediately if fails
    try
    {
        mRenderDoc.startFrame();
        m_caseExecutor->init(testCase, casePath);
    }
    catch (const std::bad_alloc &)
    {
        m_testCtx.setTerminateAfter(true);
        return TestStatus(QP_TEST_RESULT_RESOURCE_ERROR,
                          "Failed to allocate memory in test case init");
    }
    catch (const TestException &e)
    {
        DE_ASSERT(e.getTestResult() != QP_TEST_RESULT_LAST);
        m_testCtx.setTerminateAfter(e.isFatal());
        log << e;
        return TestStatus(e.getTestResult(), e.getMessage());
    }
    catch (const Exception &e)
    {
        log << e;
        return TestStatus(QP_TEST_RESULT_FAIL, e.getMessage());
    }

    bool isFirstFrameBeingCaptured = true;

    // Execute
    for (;;)
    {
        TestCase::IterateResult iterateResult = TestCase::STOP;

        m_testCtx.touchWatchdog();

        try
        {
            // Make every iteration produce one renderdoc frame.  Include the init code in the first
            // frame, and the deinit code in the last frame.
            if (!isFirstFrameBeingCaptured)
            {
                mRenderDoc.endFrame();
                mRenderDoc.startFrame();
            }
            isFirstFrameBeingCaptured = false;

            iterateResult = m_caseExecutor->iterate(testCase);
        }
        catch (const std::bad_alloc &)
        {
            m_testCtx.setTestResult(QP_TEST_RESULT_RESOURCE_ERROR,
                                    "Failed to allocate memory during test "
                                    "execution");
        }
        catch (const TestException &e)
        {
            log << e;
            m_testCtx.setTestResult(e.getTestResult(), e.getMessage());
            m_testCtx.setTerminateAfter(e.isFatal());
        }
        catch (const Exception &e)
        {
            log << e;
            m_testCtx.setTestResult(QP_TEST_RESULT_FAIL, e.getMessage());
        }

        if (iterateResult == TestCase::STOP)
            break;
    }

    DE_ASSERT(m_testCtx.getTestResult() != QP_TEST_RESULT_LAST);

    if (m_testCtx.getTestResult() == QP_TEST_RESULT_RESOURCE_ERROR)
        m_testCtx.setTerminateAfter(true);

    // De-initialize
    try
    {
        m_caseExecutor->deinit(testCase);
        mRenderDoc.endFrame();
    }
    catch (const tcu::Exception &e)
    {
        log << e << TestLog::Message
            << "Error in test case deinit, test program "
               "will terminate."
            << TestLog::EndMessage;
        m_testCtx.setTerminateAfter(true);
    }

    if (m_testCtx.getWatchDog())
        qpWatchDog_reset(m_testCtx.getWatchDog());

    {
        const TestStatus result =
            TestStatus(m_testCtx.getTestResult(), m_testCtx.getTestResultDesc());
        m_testCtx.setTestResult(QP_TEST_RESULT_LAST, "");
        return result;
    }
}

}  // namespace tcu
