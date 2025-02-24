// Copyright 2022 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "System/Configurator.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdlib>
#include <sstream>

using namespace sw;

TEST(Configurator, IntegerOptionsAreParsedCorrectly)
{
	std::istringstream config{ R"(
    [SectionA]
    OptionA = 8
    OptionB = 0xff
    OptionC = -10
    )" };
	Configurator configurator{ config };

	EXPECT_EQ(configurator.getInteger("SectionA", "OptionA", 0), 8);
	EXPECT_EQ(configurator.getInteger("SectionA", "OptionB", 0), 255);
	EXPECT_EQ(configurator.getInteger("SectionA", "OptionC", 0), -10);
}

TEST(Configurator, FloatOptionsAreParsedCorrectly)
{
	std::istringstream config{ R"(
    [SectionA]
    OptionA = 1.25
    OptionB = 3
    OptionC = 1e2
    OptionD = -1.5
    )" };
	Configurator configurator{ config };

	EXPECT_EQ(configurator.getFloat("SectionA", "OptionA", 0.0f), 1.25f);
	EXPECT_EQ(configurator.getFloat("SectionA", "OptionB", 0.0f), 3.0f);
	EXPECT_EQ(configurator.getFloat("SectionA", "OptionC", 0.0f), 100.0f);
	EXPECT_EQ(configurator.getFloat("SectionA", "OptionD", 0.0f), -1.5f);
}

TEST(Configurator, BooleanOptionsAreParsedCorrectly)
{
	std::istringstream config{ R"(
    [SectionA]
    OptionA = true
    OptionB = false
    OptionC = 1
    OptionD = 0
    )" };
	Configurator configurator{ config };

	EXPECT_EQ(configurator.getBoolean("SectionA", "OptionA", false), true);
	EXPECT_EQ(configurator.getBoolean("SectionA", "OptionB", true), false);
	EXPECT_EQ(configurator.getBoolean("SectionA", "OptionC", false), true);
	EXPECT_EQ(configurator.getBoolean("SectionA", "OptionD", true), false);
}

TEST(Configurator, MultipleSectionsSameKeyAreDistinguished)
{
	std::istringstream config{ R"(
    [SectionA]
    OptionA = 1

    [SectionB]
    OptionA = 2
    )" };
	Configurator configurator{ config };

	EXPECT_EQ(configurator.getInteger("SectionA", "OptionA", 0), 1);
	EXPECT_EQ(configurator.getInteger("SectionB", "OptionA", 0), 2);
}

TEST(Configurator, SameKeyRepeatedHasLastValue)
{
	std::istringstream config{ R"(
    [SectionA]
    OptionA = 1
    OptionA = 2
    )" };
	Configurator configurator{ config };

	EXPECT_EQ(configurator.getInteger("SectionA", "OptionA", 0), 2);
}

TEST(Configurator, NonExistentKeyReturnsDefault)
{
	std::istringstream config{ R"(
    [SectionA]
    OptionA = 1
    )" };
	Configurator configurator{ config };

	EXPECT_EQ(configurator.getInteger("SectionA", "NonExistentOption", 123), 123);
	EXPECT_EQ(configurator.getFloat("SectionA", "NonExistentOption", 1.5f), 1.5f);
	EXPECT_EQ(configurator.getBoolean("SectionA", "NonExistentOption", true), true);
}

TEST(Configurator, SectionlessOptions)
{
	std::istringstream config{ R"(
    OptionA = 8
    OptionB = 1.5
    )" };
	Configurator configurator{ config };

	EXPECT_EQ(configurator.getInteger("", "OptionA", 0), 8);
	EXPECT_EQ(configurator.getFloat("", "OptionB", 0), 1.5f);
}