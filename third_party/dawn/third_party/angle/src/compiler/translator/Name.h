//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_NAME_H_
#define COMPILER_TRANSLATOR_NAME_H_

#include "compiler/translator/ImmutableString.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/Types.h"

namespace sh
{

constexpr char kAngleInternalPrefix[] = "ANGLE";

// Represents the name of a symbol.
class Name
{
  public:
    constexpr Name(const Name &) = default;

    constexpr Name() : Name(kEmptyImmutableString, SymbolType::Empty) {}

    constexpr Name(ImmutableString rawName, SymbolType symbolType)
        : mRawName(rawName), mSymbolType(symbolType)
    {
        ASSERT(rawName.empty() == (symbolType == SymbolType::Empty));
    }

    explicit constexpr Name(const char *rawName, SymbolType symbolType = SymbolType::AngleInternal)
        : Name(ImmutableString(rawName), symbolType)
    {}

    Name(const std::string &rawName, SymbolType symbolType)
        : Name(ImmutableString(rawName), symbolType)
    {}

    explicit Name(const TField &field);
    explicit Name(const TSymbol &symbol);

    Name &operator=(const Name &) = default;
    bool operator==(const Name &other) const;
    bool operator!=(const Name &other) const;
    bool operator<(const Name &other) const;

    constexpr const ImmutableString &rawName() const { return mRawName; }
    constexpr SymbolType symbolType() const { return mSymbolType; }

    bool empty() const;
    bool beginsWith(const Name &prefix) const;

    void emit(TInfoSinkBase &out) const;

  private:
    ImmutableString mRawName;
    SymbolType mSymbolType;
    template <typename T>
    void emitImpl(T &out) const;
    friend std::ostream &operator<<(std::ostream &os, const sh::Name &name);
};

constexpr Name kBaseInstanceName = Name("baseInstance");

[[nodiscard]] bool ExpressionContainsName(const Name &name, TIntermTyped &node);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_NAME_H_
