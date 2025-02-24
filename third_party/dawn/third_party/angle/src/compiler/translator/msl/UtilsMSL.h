//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_MSL_UTILSMSL_H_
#define COMPILER_TRANSLATOR_MSL_UTILSMSL_H_

#include "common/angleutils.h"
#include "common/debug.h"
#include "compiler/translator/BaseTypes.h"
#include "compiler/translator/Common.h"
#include "compiler/translator/HashNames.h"
#include "compiler/translator/ImmutableString.h"
#include "compiler/translator/SymbolUniqueId.h"
#include "compiler/translator/Types.h"
namespace sh
{

const char *getBasicMetalString(const TType *t);

const char *getBuiltInMetalTypeNameString(const TType *t);

ImmutableString GetMetalTypeName(const TType &type,
                                 ShHashFunction64 hashFunction,
                                 NameMap *nameMap);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_UTILSMSL_H_
