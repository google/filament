//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "PreprocessorTest.h"
#include "compiler/preprocessor/Token.h"

namespace angle
{

struct OperatorTestParam
{
    const char *str;
    int op;
};

class OperatorTest : public SimplePreprocessorTest,
                     public testing::WithParamInterface<OperatorTestParam>
{};

TEST_P(OperatorTest, Identified)
{
    OperatorTestParam param = GetParam();

    pp::Token token;
    lexSingleToken(param.str, &token);
    EXPECT_EQ(param.op, token.type);
    EXPECT_EQ(param.str, token.text);
}

static const OperatorTestParam kOperators[] = {{"(", '('},
                                               {")", ')'},
                                               {"[", '['},
                                               {"]", ']'},
                                               {".", '.'},
                                               {"+", '+'},
                                               {"-", '-'},
                                               {"~", '~'},
                                               {"!", '!'},
                                               {"*", '*'},
                                               {"/", '/'},
                                               {"%", '%'},
                                               {"<", '<'},
                                               {">", '>'},
                                               {"&", '&'},
                                               {"^", '^'},
                                               {"|", '|'},
                                               {"?", '?'},
                                               {":", ':'},
                                               {"=", '='},
                                               {",", ','},
                                               {"++", pp::Token::OP_INC},
                                               {"--", pp::Token::OP_DEC},
                                               {"<<", pp::Token::OP_LEFT},
                                               {">>", pp::Token::OP_RIGHT},
                                               {"<=", pp::Token::OP_LE},
                                               {">=", pp::Token::OP_GE},
                                               {"==", pp::Token::OP_EQ},
                                               {"!=", pp::Token::OP_NE},
                                               {"&&", pp::Token::OP_AND},
                                               {"^^", pp::Token::OP_XOR},
                                               {"||", pp::Token::OP_OR},
                                               {"+=", pp::Token::OP_ADD_ASSIGN},
                                               {"-=", pp::Token::OP_SUB_ASSIGN},
                                               {"*=", pp::Token::OP_MUL_ASSIGN},
                                               {"/=", pp::Token::OP_DIV_ASSIGN},
                                               {"%=", pp::Token::OP_MOD_ASSIGN},
                                               {"<<=", pp::Token::OP_LEFT_ASSIGN},
                                               {">>=", pp::Token::OP_RIGHT_ASSIGN},
                                               {"&=", pp::Token::OP_AND_ASSIGN},
                                               {"^=", pp::Token::OP_XOR_ASSIGN},
                                               {"|=", pp::Token::OP_OR_ASSIGN}};

INSTANTIATE_TEST_SUITE_P(All, OperatorTest, testing::ValuesIn(kOperators));

}  // namespace angle
