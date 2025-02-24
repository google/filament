// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/utils/command/cli.h"

#include <sstream>

#include "gmock/gmock.h"
#include "src/tint/utils/text/string.h"

#include "src/tint/utils/containers/transform.h"  // Used by ToStringList()

namespace tint::cli {
namespace {

// Workaround for https://github.com/google/googletest/issues/3081
// Remove when using C++20
template <size_t N>
Vector<std::string, N> ToStringList(const Vector<std::string_view, N>& views) {
    return Transform(views, [](std::string_view view) { return std::string(view); });
}

using CLITest = testing::Test;

TEST_F(CLITest, ShowHelp_ValueWithParameter) {
    OptionSet opts;
    opts.Add<ValueOption<int>>("my_option", "sets the awesome value");

    std::stringstream out;
    out << "\n";
    opts.ShowHelp(out);
    EXPECT_EQ(out.str(), R"(
--my_option <value>  sets the awesome value
)");
}

TEST_F(CLITest, ShowHelp_ValueWithParameter_ExplicitEqualsFormFalse) {
    OptionSet opts;
    opts.Add<ValueOption<int>>("my_option", "sets the awesome value");

    std::stringstream out;
    out << "\n";
    opts.ShowHelp(out, false);
    EXPECT_EQ(out.str(), R"(
--my_option <value>  sets the awesome value
)");
}

TEST_F(CLITest, ShowHelp_ValueWithParameter_ExplicitEqualsFormTrue) {
    OptionSet opts;
    opts.Add<ValueOption<int>>("my_option", "sets the awesome value");

    std::stringstream out;
    out << "\n";
    opts.ShowHelp(out, true);
    EXPECT_EQ(out.str(), R"(
--my_option=<value>  sets the awesome value
)");
}

TEST_F(CLITest, ShowHelp_ValueWithAlias) {
    OptionSet opts;
    opts.Add<ValueOption<int>>("my_option", "sets the awesome value", Alias{"alias"});

    std::stringstream out;
    out << "\n";
    opts.ShowHelp(out);
    EXPECT_EQ(out.str(), R"(
--my_option <value>  sets the awesome value
--alias              alias for --my_option
)");
}
TEST_F(CLITest, ShowHelp_ValueWithShortName) {
    OptionSet opts;
    opts.Add<ValueOption<int>>("my_option", "sets the awesome value", ShortName{"a"});

    std::stringstream out;
    out << "\n";
    opts.ShowHelp(out);
    EXPECT_EQ(out.str(), R"(
--my_option <value>  sets the awesome value
 -a                  short name for --my_option
)");
}

TEST_F(CLITest, ShowHelp_MultilineDesc) {
    OptionSet opts;
    opts.Add<ValueOption<int>>("an-option", R"(this is a
multi-line description
for an option
)");

    std::stringstream out;
    out << "\n";
    opts.ShowHelp(out);
    EXPECT_EQ(out.str(), R"(
--an-option <value>  this is a
                     multi-line description
                     for an option

)");
}

TEST_F(CLITest, ShowHelp_LongName) {
    OptionSet opts;
    opts.Add<ValueOption<int>>("an-option-with-a-really-really-long-name",
                               "this is an option that has a silly long name", ShortName{"a"});

    std::stringstream out;
    out << "\n";
    opts.ShowHelp(out);
    EXPECT_EQ(out.str(), R"(
--an-option-with-a-really-really-long-name <value>
     this is an option that has a silly long name
 -a  short name for --an-option-with-a-really-really-long-name
)");
}

TEST_F(CLITest, ShowHelp_EnumValue) {
    enum class E { X, Y, Z };

    OptionSet opts;
    opts.Add<EnumOption<E>>("my_enum_option", "sets the awesome value",
                            Vector{
                                EnumName(E::X, "X"),
                                EnumName(E::Y, "Y"),
                                EnumName(E::Z, "Z"),
                            });

    std::stringstream out;
    out << "\n";
    opts.ShowHelp(out);
    EXPECT_EQ(out.str(), R"(
--my_enum_option <X|Y|Z>  sets the awesome value
)");
}

TEST_F(CLITest, ShowHelp_MixedValues) {
    enum class E { X, Y, Z };

    OptionSet opts;

    opts.Add<ValueOption<int>>("option-a", "an integer");
    opts.Add<BoolOption>("option-b", "a boolean");
    opts.Add<EnumOption<E>>("option-c", "sets the awesome value",
                            Vector{
                                EnumName(E::X, "X"),
                                EnumName(E::Y, "Y"),
                                EnumName(E::Z, "Z"),
                            });

    std::stringstream out;
    out << "\n";
    opts.ShowHelp(out);
    EXPECT_EQ(out.str(), R"(
--option-a <value>  an integer
--option-b <value>  a boolean
--option-c <X|Y|Z>  sets the awesome value
)");
}

TEST_F(CLITest, UnknownFlag) {
    OptionSet opts;
    opts.Add<BoolOption>("my_option", "a boolean value");

    auto res = opts.Parse(Split("--myoption false", " "));
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(error: unknown flag: --myoption
Did you mean '--my_option'?)");
}

TEST_F(CLITest, UnknownFlag_Ignored) {
    OptionSet opts;
    auto& opt = opts.Add<BoolOption>("my_option", "a boolean value");

    ParseOptions parse_opts;
    parse_opts.ignore_unknown = true;

    auto res = opts.Parse(Split("--myoption false", " "), parse_opts);
    ASSERT_EQ(res, Success);
    EXPECT_EQ(opt.value, std::nullopt);
}

TEST_F(CLITest, ParseBool_Flag) {
    OptionSet opts;
    auto& opt = opts.Add<BoolOption>("my_option", "a boolean value");

    auto res = opts.Parse(Split("--my_option unconsumed", " "));
    ASSERT_EQ(res, Success);
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre("unconsumed"));
    EXPECT_EQ(opt.value, true);
}

TEST_F(CLITest, ParseBool_ExplicitTrue) {
    OptionSet opts;
    auto& opt = opts.Add<BoolOption>("my_option", "a boolean value");

    auto res = opts.Parse(Split("--my_option true", " "));
    ASSERT_EQ(res, Success);
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre());
    EXPECT_EQ(opt.value, true);
}

TEST_F(CLITest, ParseBool_ExplicitFalse) {
    OptionSet opts;
    auto& opt = opts.Add<BoolOption>("my_option", "a boolean value", Default{true});

    auto res = opts.Parse(Split("--my_option false", " "));
    ASSERT_EQ(res, Success);
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre());
    EXPECT_EQ(opt.value, false);
}

TEST_F(CLITest, ParseInt) {
    OptionSet opts;
    auto& opt = opts.Add<ValueOption<int>>("my_option", "an integer value");

    auto res = opts.Parse(Split("--my_option 42", " "));
    ASSERT_EQ(res, Success);
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre());
    EXPECT_EQ(opt.value, 42);
}

TEST_F(CLITest, ParseUint64) {
    OptionSet opts;
    auto& opt = opts.Add<ValueOption<uint64_t>>("my_option", "a uint64_t value");

    auto res = opts.Parse(Split("--my_option 1000000", " "));
    ASSERT_EQ(res, Success);
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre());
    EXPECT_EQ(opt.value, 1000000);
}

TEST_F(CLITest, ParseFloat) {
    OptionSet opts;
    auto& opt = opts.Add<ValueOption<float>>("my_option", "a float value");

    auto res = opts.Parse(Split("--my_option 1.25", " "));
    ASSERT_EQ(res, Success);
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre());
    EXPECT_EQ(opt.value, 1.25f);
}

TEST_F(CLITest, ParseString) {
    OptionSet opts;
    auto& opt = opts.Add<StringOption>("my_option", "a string value");

    auto res = opts.Parse(Split("--my_option blah", " "));
    ASSERT_EQ(res, Success);
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre());
    EXPECT_EQ(opt.value, "blah");
}

TEST_F(CLITest, ParseEnum) {
    enum class E { X, Y, Z };

    OptionSet opts;
    auto& opt = opts.Add<EnumOption<E>>("my_option", "sets the awesome value",
                                        Vector{
                                            EnumName(E::X, "X"),
                                            EnumName(E::Y, "Y"),
                                            EnumName(E::Z, "Z"),
                                        });
    auto res = opts.Parse(Split("--my_option Y", " "));
    ASSERT_EQ(res, Success);
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre());
    EXPECT_EQ(opt.value, E::Y);
}

TEST_F(CLITest, ParseShortName) {
    OptionSet opts;
    auto& opt = opts.Add<ValueOption<int>>("my_option", "an integer value", ShortName{"o"});

    auto res = opts.Parse(Split("-o 42", " "));
    ASSERT_EQ(res, Success);
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre());
    EXPECT_EQ(opt.value, 42);
}

TEST_F(CLITest, ParseUnconsumed) {
    OptionSet opts;
    auto& opt = opts.Add<ValueOption<int32_t>>("my_option", "a int32_t value");

    auto res = opts.Parse(Split("abc --my_option -123 def", " "));
    ASSERT_EQ(res, Success);
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre("abc", "def"));
    EXPECT_EQ(opt.value, -123);
}

TEST_F(CLITest, ParseUsingEquals) {
    OptionSet opts;
    auto& opt = opts.Add<ValueOption<int>>("my_option", "an int value");

    auto res = opts.Parse(Split("--my_option=123", " "));
    ASSERT_EQ(res, Success);
    EXPECT_THAT(ToStringList(res.Get()), testing::ElementsAre());
    EXPECT_EQ(opt.value, 123);
}

TEST_F(CLITest, SetValueToDefault) {
    OptionSet opts;
    auto& opt = opts.Add<BoolOption>("my_option", "a boolean value", Default{true});

    auto res = opts.Parse(tint::Empty);
    ASSERT_EQ(res, Success);
    EXPECT_EQ(opt.value, true);
}

}  // namespace
}  // namespace tint::cli
