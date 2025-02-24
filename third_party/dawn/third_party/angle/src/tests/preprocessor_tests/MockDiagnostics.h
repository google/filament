//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef PREPROCESSOR_TESTS_MOCK_DIAGNOSTICS_H_
#define PREPROCESSOR_TESTS_MOCK_DIAGNOSTICS_H_

#include "compiler/preprocessor/DiagnosticsBase.h"
#include "gmock/gmock.h"

namespace angle
{

class MockDiagnostics : public pp::Diagnostics
{
  public:
    MOCK_METHOD3(print, void(ID id, const pp::SourceLocation &loc, const std::string &text));
};

}  // namespace angle

#endif  // PREPROCESSOR_TESTS_MOCK_DIAGNOSTICS_H_
