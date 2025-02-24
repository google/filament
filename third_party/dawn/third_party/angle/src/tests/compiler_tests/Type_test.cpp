//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Type_test.cpp:
//   Tests for StaticType, TType and BasicType.
//

#include "angle_gl.h"
#include "compiler/translator/PoolAlloc.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/Types.h"
#include "gtest/gtest.h"

namespace sh
{

// Verify that mangled name matches between a vector/matrix TType and a corresponding StaticType.
TEST(Type, VectorAndMatrixMangledNameConsistent)
{
    angle::PoolAllocator allocator;
    allocator.push();
    SetGlobalPoolAllocator(&allocator);

    const TType *staticTypeScalar = StaticType::Get<EbtFloat, EbpMedium, EvqGlobal, 1, 1>();
    const TType *staticTypeVec2   = StaticType::Get<EbtFloat, EbpMedium, EvqGlobal, 2, 1>();
    const TType *staticTypeMat2x4 = StaticType::Get<EbtFloat, EbpMedium, EvqGlobal, 2, 4>();
    TType *typeScalar             = new TType(EbtFloat, EbpMedium, EvqGlobal, 1, 1);
    TType *typeVec2               = new TType(EbtFloat, EbpMedium, EvqGlobal, 2, 1);
    TType *typeMat2x4             = new TType(EbtFloat, EbpMedium, EvqGlobal, 2, 4);
    EXPECT_EQ(std::string(staticTypeScalar->getMangledName()),
              std::string(typeScalar->getMangledName()));
    EXPECT_EQ(std::string(staticTypeVec2->getMangledName()),
              std::string(typeVec2->getMangledName()));
    EXPECT_EQ(std::string(staticTypeMat2x4->getMangledName()),
              std::string(typeMat2x4->getMangledName()));

    SetGlobalPoolAllocator(nullptr);
    allocator.pop();
}

// Verify that basic type mangled names are unique.
TEST(Type, BaseTypeMangledNamesUnique)
{
    // Types have either a 0-prefix or a 1-prefix
    std::set<char> uniqueNames0;
    std::set<char> uniqueNames1;
    for (int i = static_cast<int>(EbtVoid); i < static_cast<int>(EbtLast); ++i)
    {
        if (i == static_cast<int>(EbtStruct) || i == static_cast<int>(EbtInterfaceBlock))
        {
            continue;
        }
        TBasicMangledName typeName(static_cast<TBasicType>(i));
        char *mangledName = typeName.getName();
        static_assert(TBasicMangledName::mangledNameSize == 2, "Mangled name size is not 2");
        if (mangledName[0] != '{')
        {
            if (mangledName[0] == '0')
                ASSERT_TRUE(uniqueNames0.insert(mangledName[1]).second);
            if (mangledName[0] == '1')
                ASSERT_TRUE(uniqueNames1.insert(mangledName[1]).second);
        }
    }
}

}  // namespace sh
