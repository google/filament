//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FunctionLookup.cpp: Used for storing function calls that have not yet been resolved during
// parsing.
//

#include "compiler/translator/FunctionLookup.h"
#include "compiler/translator/ImmutableStringBuilder.h"

namespace sh
{

namespace
{

const char kFunctionMangledNameSeparator = '(';

constexpr const ImmutableString kEmptyName("");

}  // anonymous namespace

TFunctionLookup::TFunctionLookup(const ImmutableString &name,
                                 const TType *constructorType,
                                 const TSymbol *symbol)
    : mName(name), mConstructorType(constructorType), mThisNode(nullptr), mSymbol(symbol)
{}

// static
TFunctionLookup *TFunctionLookup::CreateConstructor(const TType *type)
{
    ASSERT(type != nullptr);
    return new TFunctionLookup(kEmptyName, type, nullptr);
}

// static
TFunctionLookup *TFunctionLookup::CreateFunctionCall(const ImmutableString &name,
                                                     const TSymbol *symbol)
{
    ASSERT(name != "");
    return new TFunctionLookup(name, nullptr, symbol);
}

const ImmutableString &TFunctionLookup::name() const
{
    return mName;
}

ImmutableString TFunctionLookup::getMangledName() const
{
    return GetMangledName(mName.data(), mArguments);
}

ImmutableString TFunctionLookup::GetMangledName(const char *functionName,
                                                const TIntermSequence &arguments)
{
    std::string newName(functionName);
    newName += kFunctionMangledNameSeparator;

    for (TIntermNode *argument : arguments)
    {
        newName += argument->getAsTyped()->getType().getMangledName();
    }
    return ImmutableString(newName);
}

bool TFunctionLookup::isConstructor() const
{
    return mConstructorType != nullptr;
}

const TType &TFunctionLookup::constructorType() const
{
    return *mConstructorType;
}

void TFunctionLookup::setThisNode(TIntermTyped *thisNode)
{
    mThisNode = thisNode;
}

TIntermTyped *TFunctionLookup::thisNode() const
{
    return mThisNode;
}

void TFunctionLookup::addArgument(TIntermTyped *argument)
{
    mArguments.push_back(argument);
}

TIntermSequence &TFunctionLookup::arguments()
{
    return mArguments;
}

const TSymbol *TFunctionLookup::symbol() const
{
    return mSymbol;
}

}  // namespace sh
