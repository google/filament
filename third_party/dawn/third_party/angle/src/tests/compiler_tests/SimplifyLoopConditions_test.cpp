//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SimplifyLoopConditions_test.cpp:
//   Tests that loop conditions are simplified.
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

namespace
{

class SimplifyLoopConditionsTest : public MatchOutputCodeTest
{
  public:
    SimplifyLoopConditionsTest() : MatchOutputCodeTest(GL_FRAGMENT_SHADER, SH_ESSL_OUTPUT)
    {
        ShCompileOptions defaultCompileOptions       = {};
        defaultCompileOptions.simplifyLoopConditions = true;
        defaultCompileOptions.validateAST            = true;
        setDefaultCompileOptions(defaultCompileOptions);
    }
};

TEST_F(SimplifyLoopConditionsTest, For)
{
    const char kShader[]   = R"(#version 300 es
void main() {
    for (;;) { }
})";
    const char kExpected[] = R"(#version 300 es
void main(){
  {
    bool sbba = true;
    while (sbba)
    {
      {
      }
    }
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SimplifyLoopConditionsTest, ForExprConstant)
{
    const char kShader[]   = R"(#version 300 es
void main() {
    for (;true;) { }
})";
    const char kExpected[] = R"(#version 300 es
void main(){
  {
    bool sbba = true;
    while (sbba)
    {
      {
      }
    }
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SimplifyLoopConditionsTest, ForExprSymbol)
{
    const char kShader[]   = R"(#version 300 es
void main() {
    bool b = true;
    for (;b;) { }
})";
    const char kExpected[] = R"(#version 300 es
void main(){
  bool _ub = true;
  {
    while (_ub)
    {
      {
      }
    }
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SimplifyLoopConditionsTest, ForExpr)
{
    const char kShader[]   = R"(#version 300 es
void main() {
    bool b = true;
    for (;b == true;) { }
})";
    const char kExpected[] = R"(#version 300 es
void main(){
  bool _ub = true;
  {
    bool sbbb = (_ub == true);
    while (sbbb)
    {
      {
      }
      (sbbb = (_ub == true));
    }
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SimplifyLoopConditionsTest, ForInitExprSymbol)
{
    const char kShader[]   = R"(#version 300 es
void main() {
    for (bool b = true; b;) { }
})";
    const char kExpected[] = R"(#version 300 es
void main(){
  {
    bool _ub = true;
    while (_ub)
    {
      {
      }
    }
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SimplifyLoopConditionsTest, ForInitExprSymbolExpr2)
{
    const char kShader[]   = R"(#version 300 es
void main() {
    for (bool b = true; b; b = false) { }
})";
    const char kExpected[] = R"(#version 300 es
void main(){
  {
    bool _ub = true;
    while (_ub)
    {
      {
      }
      (_ub = false);
    }
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SimplifyLoopConditionsTest, ForInitExprExpr2)
{
    const char kShader[]   = R"(#version 300 es
void main() {
        for (highp int i; i < 100; ++i) { }
})";
    const char kExpected[] = R"(#version 300 es
void main(){
  {
    highp int _ui;
    bool sbbb = (_ui < 100);
    while (sbbb)
    {
      {
      }
      (++_ui);
      (sbbb = (_ui < 100));
    }
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SimplifyLoopConditionsTest, ForInitExprExpr2Break)
{
    const char kShader[]   = R"(#version 300 es
uniform highp int u;
void main() {
    for (highp int i; i < 100; ++i) { if (i < u) break; }
})";
    const char kExpected[] = R"(#version 300 es
uniform highp int _uu;
void main(){
  {
    highp int _ui;
    bool sbbc = (_ui < 100);
    while (sbbc)
    {
      {
        if ((_ui < _uu))
        {
          break;
        }
      }
      (++_ui);
      (sbbc = (_ui < 100));
    }
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SimplifyLoopConditionsTest, ForInitExprExpr2Continue)
{
    const char kShader[]   = R"(#version 300 es
uniform highp int u;
void main() {
    for (highp int i; i < 100; ++i) { if (i < u) continue; ++i; }
})";
    const char kExpected[] = R"(#version 300 es
uniform highp int _uu;
void main(){
  {
    highp int _ui;
    bool sbbc = (_ui < 100);
    while (sbbc)
    {
      {
        if ((_ui < _uu))
        {
          (++_ui);
          (sbbc = (_ui < 100));
          continue;
        }
        (++_ui);
      }
      (++_ui);
      (sbbc = (_ui < 100));
    }
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

}  // namespace