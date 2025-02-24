//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gtest/gtest.h"

#include "compiler/preprocessor/Token.h"

namespace angle
{

TEST(TokenTest, DefaultConstructor)
{
    pp::Token token;
    EXPECT_EQ(0, token.type);
    EXPECT_EQ(0u, token.flags);
    EXPECT_EQ(0, token.location.line);
    EXPECT_EQ(0, token.location.file);
    EXPECT_EQ("", token.text);
}

TEST(TokenTest, Assignment)
{
    pp::Token token;
    token.type          = 1;
    token.flags         = 1;
    token.location.line = 1;
    token.location.file = 1;
    token.text.assign("foo");

    token = pp::Token();
    EXPECT_EQ(0, token.type);
    EXPECT_EQ(0u, token.flags);
    EXPECT_EQ(0, token.location.line);
    EXPECT_EQ(0, token.location.file);
    EXPECT_EQ("", token.text);
}

TEST(TokenTest, Equals)
{
    pp::Token token;
    EXPECT_TRUE(token.equals(pp::Token()));

    token.type = 1;
    EXPECT_FALSE(token.equals(pp::Token()));
    token.type = 0;

    token.flags = 1;
    EXPECT_FALSE(token.equals(pp::Token()));
    token.flags = 0;

    token.location.line = 1;
    EXPECT_FALSE(token.equals(pp::Token()));
    token.location.line = 0;

    token.location.file = 1;
    EXPECT_FALSE(token.equals(pp::Token()));
    token.location.file = 0;

    token.text.assign("foo");
    EXPECT_FALSE(token.equals(pp::Token()));
    token.text.clear();

    EXPECT_TRUE(token.equals(pp::Token()));
}

TEST(TokenTest, HasLeadingSpace)
{
    pp::Token token;
    EXPECT_FALSE(token.hasLeadingSpace());
    token.setHasLeadingSpace(true);
    EXPECT_TRUE(token.hasLeadingSpace());
    token.setHasLeadingSpace(false);
    EXPECT_FALSE(token.hasLeadingSpace());
}

TEST(TokenTest, Write)
{
    pp::Token token;
    token.text.assign("foo");
    std::stringstream out1;
    out1 << token;
    EXPECT_TRUE(out1.good());
    EXPECT_EQ("foo", out1.str());

    token.setHasLeadingSpace(true);
    std::stringstream out2;
    out2 << token;
    EXPECT_TRUE(out2.good());
    EXPECT_EQ(" foo", out2.str());
}

}  // namespace angle
