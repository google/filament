//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// QueryObjectValidation.cpp : Tests between gl.*Query.* functions and interactions with
// EGL_CONTEXT_OPENGL_NO_ERROR_KHR
#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"
#include "util/EGLWindow.h"
#include "util/test_utils.h"

namespace angle
{

using QueryObjectTestParams = std::tuple<angle::PlatformParameters, bool>;

std::string PrintToStringParamName(const ::testing::TestParamInfo<QueryObjectTestParams> &info)
{
    std::stringstream ss;
    ss << std::get<0>(info.param);
    if (std::get<1>(info.param))
    {
        ss << "__ValidationDisabled";
    }
    else
    {
        ss << "__ValidationEnabled";
    }
    return ss.str();
}

class QueryObjectTest : public ANGLETest<QueryObjectTestParams>
{
  protected:
    QueryObjectTest() : mQueryObjectName(0), mQueryResult(false)
    {
        setNoErrorEnabled(testing::get<1>(GetParam()));
    }

    void createQuery()
    {
        glGenQueries(1, &mQueryObjectName);
        ASSERT_NE(mQueryObjectName, (GLuint)0u);
        ASSERT_GL_NO_ERROR();
    }

    void testSetUp() override { createQuery(); }

    void testTearDown() override
    {
        if (mQueryObjectName)
        {
            glDeleteQueries(1, &mQueryObjectName);
        }
    }

    GLuint mQueryObjectName = 0;
    GLuint mQueryResult     = 0;
};

class QueryObjectTestES32 : public QueryObjectTest
{};

// Test if a generated query is a query before glBeginQuery
TEST_P(QueryObjectTest, QueryObjectIsQuery)
{
    GLboolean isQueryResult = glIsQuery(mQueryObjectName);
    ASSERT_GL_NO_ERROR();
    ASSERT_FALSE(isQueryResult);
}

// Negative test for glGetQueryObjectuiv before glBegin with GL_QUERY_RESULT
TEST_P(QueryObjectTest, QueryObjectResultBeforeBegin)
{
    glGetQueryObjectuiv(mQueryObjectName, GL_QUERY_RESULT, &mQueryResult);

    bool isNoError = testing::get<1>(GetParam());
    if (isNoError)
    {
        ASSERT_GL_NO_ERROR();
    }
    else
    {
        ASSERT_EQ(glGetError(), (GLenum)GL_INVALID_OPERATION);
    }
}

// Negative test for glGetQueryObjectuiv before glBegin with GL_QUERY_RESULT_AVAILABLE
TEST_P(QueryObjectTest, QueryObjectResultAvailableBeforeBegin)
{
    glGetQueryObjectuiv(mQueryObjectName, GL_QUERY_RESULT_AVAILABLE, &mQueryResult);

    bool isNoError = testing::get<1>(GetParam());
    if (isNoError)
    {
        ASSERT_GL_NO_ERROR();
    }
    else
    {
        ASSERT_EQ(glGetError(), (GLenum)GL_INVALID_OPERATION);
    }
}

// Test glGetQueryObjectuiv after glEndQuery
TEST_P(QueryObjectTest, QueryObjectResultAfterEnd)
{
    glBeginQuery(GL_ANY_SAMPLES_PASSED, mQueryObjectName);
    ASSERT_GL_NO_ERROR();

    glEndQuery(GL_ANY_SAMPLES_PASSED);
    ASSERT_GL_NO_ERROR();

    glGetQueryObjectuiv(mQueryObjectName, GL_QUERY_RESULT_AVAILABLE, &mQueryResult);
}

// Test glGetQueryObjectuiv after glEndQuery with GL_PRIMITIVES_GENERATED
TEST_P(QueryObjectTestES32, QueryObjectResultAfterEndPrimitivesGenerated)
{
    // TODO(anglebug.com/42263969): Allow GL_PRIMITIVES_GENERATED query objects
    // when transform feedback is not active
    ANGLE_SKIP_TEST_IF(IsVulkan());
    glBeginQuery(GL_PRIMITIVES_GENERATED, mQueryObjectName);
    ASSERT_GL_NO_ERROR();

    glEndQuery(GL_PRIMITIVES_GENERATED);
    ASSERT_GL_NO_ERROR();

    while (mQueryResult != GL_TRUE)
    {
        glGetQueryObjectuiv(mQueryObjectName, GL_QUERY_RESULT_AVAILABLE, &mQueryResult);
        ASSERT_GL_NO_ERROR();
        angle::Sleep(50);
    }

    GLboolean isQueryResult = glIsQuery(mQueryObjectName);
    ASSERT_GL_NO_ERROR();
    ASSERT_TRUE(isQueryResult);
}

static const bool noErrorFlags[] = {true, false};

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(QueryObjectTest);
ANGLE_INSTANTIATE_TEST_COMBINE_1(QueryObjectTest,
                                 PrintToStringParamName,
                                 testing::ValuesIn(noErrorFlags),
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(QueryObjectTestES32);
ANGLE_INSTANTIATE_TEST_COMBINE_1(QueryObjectTestES32,
                                 PrintToStringParamName,
                                 testing::ValuesIn(noErrorFlags),
                                 ANGLE_ALL_TEST_PLATFORMS_ES32);

}  // namespace angle
