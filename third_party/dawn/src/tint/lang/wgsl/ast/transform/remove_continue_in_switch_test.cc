// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/transform/remove_continue_in_switch.h"
#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {
namespace {

using RemoveContinueInSwitchTest = ast::transform::TransformTest;

TEST_F(RemoveContinueInSwitchTest, ShouldRun_True) {
    auto* src = R"(
fn f() {
  var i = 0;
  loop {
    switch(i) {
      case 0: {
        continue;
      }
      default: {
        break;
      }
    }
    break;
  }
}
)";

    EXPECT_TRUE(ShouldRun<RemoveContinueInSwitch>(src));
}

TEST_F(RemoveContinueInSwitchTest, ShouldRunEmptyModule_False) {
    auto* src = "";

    EXPECT_FALSE(ShouldRun<RemoveContinueInSwitch>(src));
}

TEST_F(RemoveContinueInSwitchTest, ShouldRunContinueNotInSwitch_False) {
    auto* src = R"(
fn f() {
  var i = 0;
  loop {
    switch(i) {
      case 0: {
        break;
      }
      default: {
        break;
      }
    }

    if (true) {
      continue;
    }
    break;
  }
}
)";

    EXPECT_FALSE(ShouldRun<RemoveContinueInSwitch>(src));
}

TEST_F(RemoveContinueInSwitchTest, ShouldRunContinueInLoopInSwitch_False) {
    auto* src = R"(
fn f() {
  var i = 0;
  switch(i) {
    case 0: {
      loop {
        if (true) {
          continue;
        }
        break;
      }
      break;
    }
    default: {
      break;
    }
  }
}
)";

    EXPECT_FALSE(ShouldRun<RemoveContinueInSwitch>(src));
}

TEST_F(RemoveContinueInSwitchTest, EmptyModule) {
    auto* src = "";
    auto* expect = src;

    ast::transform::DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, SingleContinue) {
    auto* src = R"(
fn f() {
  var i = 0;
  loop {
    let marker1 = 0;
    switch(i) {
      case 0: {
        continue;
      }
      default: {
        break;
      }
    }
    let marker2 = 0;
    break;

    continuing {
      let marker3 = 0;
    }
  }
}
)";

    auto* expect = R"(
fn f() {
  var i = 0;
  var tint_continue : bool;
  loop {
    tint_continue = false;
    let marker1 = 0;
    switch(i) {
      case 0: {
        tint_continue = true;
        break;
      }
      default: {
        break;
      }
    }
    if (tint_continue) {
      continue;
    }
    let marker2 = 0;
    break;

    continuing {
      let marker3 = 0;
    }
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, MultipleContinues) {
    auto* src = R"(
fn f() {
  var i = 0;
  loop {
    let marker1 = 0;
    switch(i) {
      case 0: {
        continue;
      }
      case 1: {
        continue;
      }
      case 2: {
        continue;
      }
      default: {
        break;
      }
    }
    let marker2 = 0;
    break;

    continuing {
      let marker3 = 0;
    }
  }
}
)";

    auto* expect = R"(
fn f() {
  var i = 0;
  var tint_continue : bool;
  loop {
    tint_continue = false;
    let marker1 = 0;
    switch(i) {
      case 0: {
        tint_continue = true;
        break;
      }
      case 1: {
        tint_continue = true;
        break;
      }
      case 2: {
        tint_continue = true;
        break;
      }
      default: {
        break;
      }
    }
    if (tint_continue) {
      continue;
    }
    let marker2 = 0;
    break;

    continuing {
      let marker3 = 0;
    }
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, MultipleSwitch) {
    auto* src = R"(
fn f() {
  var i = 0;
  loop {
    let marker1 = 0;
    switch(i) {
      case 0: {
        continue;
      }
      default: {
        break;
      }
    }
    let marker2 = 0;

    let marker3 = 0;
    switch(i) {
      case 0: {
        continue;
      }
      default: {
        break;
      }
    }
    let marker4 = 0;

    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  var i = 0;
  var tint_continue : bool;
  loop {
    tint_continue = false;
    let marker1 = 0;
    switch(i) {
      case 0: {
        tint_continue = true;
        break;
      }
      default: {
        break;
      }
    }
    if (tint_continue) {
      continue;
    }
    let marker2 = 0;
    let marker3 = 0;
    switch(i) {
      case 0: {
        tint_continue = true;
        break;
      }
      default: {
        break;
      }
    }
    if (tint_continue) {
      continue;
    }
    let marker4 = 0;
    break;
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, NestedLoopSwitchSwitch) {
    auto* src = R"(
fn f() {
  var j = 0;
  for (var i = 0; i < 2; i += 2) {
    switch(i) {
      case 0: {
        switch(j) {
          case 0: {
            continue;
          }
          default: {
          }
        }
      }
      default: {
      }
    }
  }
}
)";

    auto* expect = R"(
fn f() {
  var j = 0;
  var tint_continue : bool;
  for(var i = 0; (i < 2); i += 2) {
    tint_continue = false;
    switch(i) {
      case 0: {
        switch(j) {
          case 0: {
            tint_continue = true;
            break;
          }
          default: {
          }
        }
        if (tint_continue) {
          break;
        }
      }
      default: {
      }
    }
    if (tint_continue) {
      continue;
    }
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, NestedLoopLoopSwitch) {
    auto* src = R"(
fn f() {
  for (var i = 0; i < 2; i += 2) {
    for (var j = 0; j < 2; j += 2) {
      switch(i) {
        case 0: {
          continue;
        }
        default: {
        }
      }
    }
  }
}
)";

    auto* expect = R"(
fn f() {
  for(var i = 0; (i < 2); i += 2) {
    var tint_continue : bool;
    for(var j = 0; (j < 2); j += 2) {
      tint_continue = false;
      switch(i) {
        case 0: {
          tint_continue = true;
          break;
        }
        default: {
        }
      }
      if (tint_continue) {
        continue;
      }
    }
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, NestedLoopSwitchLoopSwitch) {
    auto* src = R"(
fn f() {
  for (var i = 0; i < 2; i += 2) {
    switch(i) {
      case 0: {
        for (var j = 0; j < 2; j += 2) {
          switch(j) {
            case 0: {
              continue; // j loop
            }
            default: {
            }
          }
        }
        continue; // i loop
      }
      default: {
      }
    }
  }
}
)";

    auto* expect = R"(
fn f() {
  var tint_continue_1 : bool;
  for(var i = 0; (i < 2); i += 2) {
    tint_continue_1 = false;
    switch(i) {
      case 0: {
        var tint_continue : bool;
        for(var j = 0; (j < 2); j += 2) {
          tint_continue = false;
          switch(j) {
            case 0: {
              tint_continue = true;
              break;
            }
            default: {
            }
          }
          if (tint_continue) {
            continue;
          }
        }
        tint_continue_1 = true;
        break;
      }
      default: {
      }
    }
    if (tint_continue_1) {
      continue;
    }
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, NestedLoopSwitchLoopSwitchSwitch) {
    auto* src = R"(
fn f() {
  var k = 0;
  for (var i = 0; i < 2; i += 2) {
    switch(i) {
      case 0: {
        for (var j = 0; j < 2; j += 2) {
          switch(j) {
            case 0: {
              continue; // j loop
            }
            case 1: {
              switch (k) {
                case 0: {
                  continue; // j loop
                }
                default: {
                }
              }
            }
            default: {
            }
          }
        }
        continue; // i loop
      }
      default: {
      }
    }
  }
}
)";

    auto* expect = R"(
fn f() {
  var k = 0;
  var tint_continue_1 : bool;
  for(var i = 0; (i < 2); i += 2) {
    tint_continue_1 = false;
    switch(i) {
      case 0: {
        var tint_continue : bool;
        for(var j = 0; (j < 2); j += 2) {
          tint_continue = false;
          switch(j) {
            case 0: {
              tint_continue = true;
              break;
            }
            case 1: {
              switch(k) {
                case 0: {
                  tint_continue = true;
                  break;
                }
                default: {
                }
              }
              if (tint_continue) {
                break;
              }
            }
            default: {
            }
          }
          if (tint_continue) {
            continue;
          }
        }
        tint_continue_1 = true;
        break;
      }
      default: {
      }
    }
    if (tint_continue_1) {
      continue;
    }
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, ExtraScopes) {
    auto* src = R"(
fn f() {
  var i = 0;
  var a = true;
  var b = true;
  var c = true;
  var d = true;
  loop {
    if (a) {
      if (b) {
        let marker1 = 0;
        switch(i) {
          case 0: {
            if (c) {
              if (d) {
                continue;
              }
            }
            break;
          }
          default: {
            break;
          }
        }
        let marker2 = 0;
        break;
      }
    }
  }
}
)";

    auto* expect = R"(
fn f() {
  var i = 0;
  var a = true;
  var b = true;
  var c = true;
  var d = true;
  var tint_continue : bool;
  loop {
    tint_continue = false;
    if (a) {
      if (b) {
        let marker1 = 0;
        switch(i) {
          case 0: {
            if (c) {
              if (d) {
                tint_continue = true;
                break;
              }
            }
            break;
          }
          default: {
            break;
          }
        }
        if (tint_continue) {
          continue;
        }
        let marker2 = 0;
        break;
      }
    }
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, ForLoop) {
    auto* src = R"(
fn f() {
  for (var i = 0; i < 4; i = i + 1) {
    let marker1 = 0;
    switch(i) {
      case 0: {
        continue;
        break;
      }
      default: {
        break;
      }
    }
    let marker2 = 0;
    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  var tint_continue : bool;
  for(var i = 0; (i < 4); i = (i + 1)) {
    tint_continue = false;
    let marker1 = 0;
    switch(i) {
      case 0: {
        tint_continue = true;
        break;
        break;
      }
      default: {
        break;
      }
    }
    if (tint_continue) {
      continue;
    }
    let marker2 = 0;
    break;
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveContinueInSwitchTest, While) {
    auto* src = R"(
fn f() {
  var i = 0;
  while (i < 4) {
    let marker1 = 0;
    switch(i) {
      case 0: {
        continue;
        break;
      }
      default: {
        break;
      }
    }
    let marker2 = 0;
    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  var i = 0;
  var tint_continue : bool;
  while((i < 4)) {
    tint_continue = false;
    let marker1 = 0;
    switch(i) {
      case 0: {
        tint_continue = true;
        break;
        break;
      }
      default: {
        break;
      }
    }
    if (tint_continue) {
      continue;
    }
    let marker2 = 0;
    break;
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<RemoveContinueInSwitch>(src, data);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
