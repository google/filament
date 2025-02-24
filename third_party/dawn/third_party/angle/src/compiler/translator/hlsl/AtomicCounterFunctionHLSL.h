//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AtomicCounterFunctionHLSL: Class for writing implementation of atomic counter functions into HLSL
// output.
//

#ifndef COMPILER_TRANSLATOR_HLSL_ATOMICCOUNTERFUNCTIONHLSL_H_
#define COMPILER_TRANSLATOR_HLSL_ATOMICCOUNTERFUNCTIONHLSL_H_

#include <map>

#include "compiler/translator/Common.h"
#include "compiler/translator/ImmutableString.h"

namespace sh
{

class TInfoSinkBase;
struct TLayoutQualifier;

class AtomicCounterFunctionHLSL final : angle::NonCopyable
{
  public:
    AtomicCounterFunctionHLSL(bool forceResolution);

    ImmutableString useAtomicCounterFunction(const ImmutableString &name);

    void atomicCounterFunctionHeader(TInfoSinkBase &out);

  private:
    enum class AtomicCounterFunction
    {
        LOAD,
        INCREMENT,
        DECREMENT,
        INVALID
    };

    std::map<ImmutableString, AtomicCounterFunction> mAtomicCounterFunctions;
    bool mForceResolution;
};

ImmutableString getAtomicCounterNameForBinding(int binding);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_HLSL_ATOMICCOUNTERFUNCTIONHLSL_H_
