//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef PREPROCESSOR_TESTS_MOCK_DIRECTIVE_HANDLER_H_
#define PREPROCESSOR_TESTS_MOCK_DIRECTIVE_HANDLER_H_

#include "compiler/preprocessor/DirectiveHandlerBase.h"
#include "gmock/gmock.h"

namespace angle
{

class MockDirectiveHandler : public pp::DirectiveHandler
{
  public:
    MOCK_METHOD2(handleError, void(const pp::SourceLocation &loc, const std::string &msg));

    MOCK_METHOD4(handlePragma,
                 void(const pp::SourceLocation &loc,
                      const std::string &name,
                      const std::string &value,
                      bool stdgl));

    MOCK_METHOD3(handleExtension,
                 void(const pp::SourceLocation &loc,
                      const std::string &name,
                      const std::string &behavior));

    MOCK_METHOD4(handleVersion,
                 void(const pp::SourceLocation &loc,
                      int version,
                      ShShaderSpec spec,
                      pp::MacroSet *macro_set));
};

}  // namespace angle

#endif  // PREPROCESSOR_TESTS_MOCK_DIRECTIVE_HANDLER_H_
