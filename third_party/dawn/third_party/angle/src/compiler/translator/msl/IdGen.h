//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_MSL_IDGEN_H_
#define COMPILER_TRANSLATOR_MSL_IDGEN_H_

#include "common/angleutils.h"
#include "compiler/translator/Name.h"

namespace sh
{

// For creating new fresh names.
// All names created are marked as SymbolType::AngleInternal.
class IdGen : angle::NonCopyable
{
  public:
    IdGen();

    Name createNewName(const ImmutableString &baseName);
    Name createNewName(const Name &baseName);
    Name createNewName(const char *baseName);
    Name createNewName(std::initializer_list<ImmutableString> baseNames);
    Name createNewName(std::initializer_list<Name> baseNames);
    Name createNewName(std::initializer_list<const char *> baseNames);
    Name createNewName();

  private:
    template <typename String, typename StringToImmutable>
    Name createNewName(size_t count, const String *baseNames, const StringToImmutable &toImmutable);

  private:
    unsigned mNext = 0;          // `unsigned` because of "%u" use in sprintf
    std::string mNewNameBuffer;  // reusable buffer to avoid tons of reallocations
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_IDGEN_H_
