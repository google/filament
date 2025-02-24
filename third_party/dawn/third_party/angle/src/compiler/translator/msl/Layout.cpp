//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <algorithm>
#include <cctype>
#include <cstring>
#include <limits>
#include <unordered_map>
#include <unordered_set>

#include "compiler/translator/Symbol.h"
#include "compiler/translator/msl/Layout.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

enum class Language
{
    Metal,
    GLSL,
};

static size_t RoundUpToMultipleOf(size_t x, size_t multiple)
{
    const size_t rem = x % multiple;
    return rem == 0 ? x : x + (multiple - rem);
}

void Layout::requireAlignment(size_t align, bool pad)
{
    alignOf = std::max(alignOf, align);
    if (pad)
    {
        sizeOf = RoundUpToMultipleOf(sizeOf, align);
    }
}

bool Layout::operator==(const Layout &other) const
{
    return sizeOf == other.sizeOf && alignOf == other.alignOf;
}

void Layout::operator+=(const Layout &other)
{
    requireAlignment(other.alignOf, true);
    sizeOf += other.sizeOf;
}

void Layout::operator*=(size_t scale)
{
    sizeOf *= scale;
}

Layout Layout::operator+(const Layout &other) const
{
    auto self = *this;
    self += other;
    return self;
}

Layout Layout::operator*(size_t scale) const
{
    auto self = *this;
    self *= scale;
    return self;
}

static Layout ScalarLayoutOf(const TType &type, Language language)
{
    const TBasicType basicType = type.getBasicType();
    switch (basicType)
    {
        case TBasicType::EbtBool:
            return {1, 1};
        case TBasicType::EbtInt:
        case TBasicType::EbtUInt:
        case TBasicType::EbtFloat:
            return {4, 4};
        default:
            if (IsSampler(basicType))
            {
                switch (language)
                {
                    case Language::Metal:
                        return {8, 8};
                    case Language::GLSL:
                        return {4, 4};
                }
            }
            UNIMPLEMENTED();
            return Layout::Invalid();
    }
}

static const size_t innerScalesPacked[]   = {0, 1, 2, 3, 4};
static const size_t innerScalesUnpacked[] = {0, 1, 2, 4, 4};

Layout sh::MetalLayoutOf(const TType &type, MetalLayoutOfConfig config)
{
    ASSERT(type.getNominalSize() <= 4);
    ASSERT(type.getSecondarySize() <= 4);

    const TLayoutBlockStorage storage = type.getLayoutQualifier().blockStorage;

    const bool isPacked = !config.disablePacking && (storage == TLayoutBlockStorage::EbsPacked ||
                                                     storage == TLayoutBlockStorage::EbsShared);

    if (type.isArray() && !config.maskArray)
    {
        config.maskArray    = true;
        const Layout layout = MetalLayoutOf(type, config);
        config.maskArray    = false;
        const size_t vol    = type.getArraySizeProduct();
        return layout * vol;
    }

    if (const TStructure *structure = type.getStruct())
    {
        ASSERT(type.getNominalSize() == 1);
        ASSERT(type.getSecondarySize() == 1);
        auto config2             = config;
        config2.maskArray        = false;
        auto layout              = Layout::Identity();
        const TFieldList &fields = structure->fields();
        for (const TField *field : fields)
        {
            layout += MetalLayoutOf(*field->type(), config2);
        }
        if (config.assumeStructsAreTailPadded)
        {
            size_t pad =
                (kDefaultStructAlignmentSize - layout.sizeOf) % kDefaultStructAlignmentSize;
            layout.sizeOf += pad;
        }
        layout.sizeOf = RoundUpToMultipleOf(layout.sizeOf, layout.alignOf);
        return layout;
    }

    if (config.treatSamplersAsTextureEnv && IsSampler(type.getBasicType()))
    {
        return {16, 8};  // pointer{8,8} and metal::sampler{8,8}
    }

    const Layout scalarLayout = ScalarLayoutOf(type, Language::Metal);

    if (type.isRank0())
    {
        return scalarLayout;
    }
    else if (type.isVector())
    {
        if (isPacked)
        {
            const size_t innerScale = innerScalesPacked[type.getNominalSize()];
            auto layout = Layout{scalarLayout.sizeOf * innerScale, scalarLayout.alignOf};
            return layout;
        }
        else
        {
            const size_t innerScale = innerScalesUnpacked[type.getNominalSize()];
            auto layout             = Layout::Both(scalarLayout.sizeOf * innerScale);
            return layout;
        }
    }
    else
    {
        ASSERT(type.isMatrix());
        ASSERT(type.getBasicType() == TBasicType::EbtFloat);
        // typeCxR <=> typeR[C]
        const size_t innerScale = innerScalesUnpacked[type.getRows()];
        const size_t outerScale = static_cast<size_t>(type.getCols());
        const size_t n          = scalarLayout.sizeOf * innerScale;
        return {n * outerScale, n};
    }
}

TLayoutBlockStorage sh::Overlay(TLayoutBlockStorage oldStorage, const TType &type)
{
    const TLayoutBlockStorage newStorage = type.getLayoutQualifier().blockStorage;
    switch (newStorage)
    {
        case TLayoutBlockStorage::EbsUnspecified:
            return oldStorage == TLayoutBlockStorage::EbsUnspecified ? kDefaultLayoutBlockStorage
                                                                     : oldStorage;
        default:
            return newStorage;
    }
}

TLayoutMatrixPacking sh::Overlay(TLayoutMatrixPacking oldPacking, const TType &type)
{
    const TLayoutMatrixPacking newPacking = type.getLayoutQualifier().matrixPacking;
    switch (newPacking)
    {
        case TLayoutMatrixPacking::EmpUnspecified:
            return oldPacking == TLayoutMatrixPacking::EmpUnspecified ? kDefaultLayoutMatrixPacking
                                                                      : oldPacking;
        default:
            return newPacking;
    }
}

bool sh::CanBePacked(TLayoutBlockStorage storage)
{
    switch (storage)
    {
        case TLayoutBlockStorage::EbsPacked:
        case TLayoutBlockStorage::EbsShared:
            return true;
        case TLayoutBlockStorage::EbsStd140:
        case TLayoutBlockStorage::EbsStd430:
            return false;
        case TLayoutBlockStorage::EbsUnspecified:
            UNREACHABLE();
            return false;
    }
}

bool sh::CanBePacked(TLayoutQualifier layoutQualifier)
{
    return CanBePacked(layoutQualifier.blockStorage);
}

bool sh::CanBePacked(const TType &type)
{
    if (!type.isVector())
    {
        return false;
    }
    return CanBePacked(type.getLayoutQualifier());
}

void sh::SetBlockStorage(TType &type, TLayoutBlockStorage storage)
{
    auto qual         = type.getLayoutQualifier();
    qual.blockStorage = storage;
    type.setLayoutQualifier(qual);
}

static Layout CommonGlslStructLayoutOf(TField const *const *begin,
                                       TField const *const *end,
                                       const TLayoutBlockStorage storage,
                                       const TLayoutMatrixPacking matrixPacking,
                                       const bool maskArray,
                                       const size_t baseAlignment)
{
    const bool isPacked =
        storage == TLayoutBlockStorage::EbsPacked || storage == TLayoutBlockStorage::EbsShared;

    auto layout = Layout::Identity();
    for (auto iter = begin; iter != end; ++iter)
    {
        layout += GlslLayoutOf(*(*iter)->type(), storage, matrixPacking, false);
    }
    if (!isPacked)  // XXX: Correct?
    {
        layout.sizeOf = RoundUpToMultipleOf(layout.sizeOf, layout.alignOf);
    }
    layout.requireAlignment(baseAlignment, true);
    return layout;
}

static Layout CommonGlslLayoutOf(const TType &type,
                                 const TLayoutBlockStorage storage,
                                 const TLayoutMatrixPacking matrixPacking,
                                 const bool maskArray,
                                 const size_t baseAlignment)
{
    ASSERT(storage != TLayoutBlockStorage::EbsUnspecified);

    const bool isPacked =
        storage == TLayoutBlockStorage::EbsPacked || storage == TLayoutBlockStorage::EbsShared;

    if (type.isArray() && !type.isMatrix() && !maskArray)
    {
        auto layout = GlslLayoutOf(type, storage, matrixPacking, true);
        layout *= type.getArraySizeProduct();
        layout.requireAlignment(baseAlignment, true);
        return layout;
    }

    if (const TStructure *structure = type.getStruct())
    {
        ASSERT(type.getNominalSize() == 1);
        ASSERT(type.getSecondarySize() == 1);
        const TFieldList &fields = structure->fields();
        return CommonGlslStructLayoutOf(fields.data(), fields.data() + fields.size(), storage,
                                        matrixPacking, maskArray, baseAlignment);
    }

    const auto scalarLayout = ScalarLayoutOf(type, Language::GLSL);

    if (type.isRank0())
    {
        return scalarLayout;
    }
    else if (type.isVector())
    {
        if (isPacked)
        {
            const size_t sizeScale  = innerScalesPacked[type.getNominalSize()];
            const size_t alignScale = innerScalesUnpacked[type.getNominalSize()];
            auto layout =
                Layout{scalarLayout.sizeOf * sizeScale, scalarLayout.alignOf * alignScale};
            return layout;
        }
        else
        {
            const size_t innerScale = innerScalesUnpacked[type.getNominalSize()];
            auto layout             = Layout::Both(scalarLayout.sizeOf * innerScale);
            return layout;
        }
    }
    else
    {
        ASSERT(type.isMatrix());

        size_t innerDim;
        size_t outerDim;

        switch (matrixPacking)
        {
            case TLayoutMatrixPacking::EmpColumnMajor:
                innerDim = static_cast<size_t>(type.getRows());
                outerDim = static_cast<size_t>(type.getCols());
                break;
            case TLayoutMatrixPacking::EmpRowMajor:
                innerDim = static_cast<size_t>(type.getCols());
                outerDim = static_cast<size_t>(type.getRows());
                break;
            case TLayoutMatrixPacking::EmpUnspecified:
                UNREACHABLE();
                innerDim = 0;
                outerDim = 0;
        }

        outerDim *= type.getArraySizeProduct();

        const size_t innerScale = innerScalesUnpacked[innerDim];
        const size_t n          = innerScale * scalarLayout.sizeOf;
        Layout layout           = {outerDim * n, n};
        layout.requireAlignment(baseAlignment, true);
        return layout;
    }
}

Layout sh::GlslLayoutOf(const TType &type,
                        TLayoutBlockStorage storage,
                        TLayoutMatrixPacking matrixPacking,
                        bool maskArray)
{
    ASSERT(type.getNominalSize() <= 4);
    ASSERT(type.getSecondarySize() <= 4);

    storage       = Overlay(storage, type);
    matrixPacking = Overlay(matrixPacking, type);

    switch (storage)
    {
        case TLayoutBlockStorage::EbsPacked:
            return CommonGlslLayoutOf(type, storage, matrixPacking, maskArray, 1);
        case TLayoutBlockStorage::EbsShared:
            return CommonGlslLayoutOf(type, storage, matrixPacking, maskArray, 16);
        case TLayoutBlockStorage::EbsStd140:
            return CommonGlslLayoutOf(type, storage, matrixPacking, maskArray, 16);
        case TLayoutBlockStorage::EbsStd430:
            return CommonGlslLayoutOf(type, storage, matrixPacking, maskArray, 1);
        case TLayoutBlockStorage::EbsUnspecified:
            UNREACHABLE();
            return Layout::Invalid();
    }
}

[[nodiscard]] Layout sh::GlslStructLayoutOf(TField const *const *begin,
                                            TField const *const *end,
                                            TLayoutBlockStorage storage,
                                            TLayoutMatrixPacking matrixPacking,
                                            bool maskArray)
{
    ASSERT(storage != TLayoutBlockStorage::EbsUnspecified);
    ASSERT(matrixPacking != TLayoutMatrixPacking::EmpUnspecified);

    switch (storage)
    {
        case TLayoutBlockStorage::EbsPacked:
            return CommonGlslStructLayoutOf(begin, end, storage, matrixPacking, maskArray, 1);
        case TLayoutBlockStorage::EbsShared:
            return CommonGlslStructLayoutOf(begin, end, storage, matrixPacking, maskArray, 16);
        case TLayoutBlockStorage::EbsStd140:
            return CommonGlslStructLayoutOf(begin, end, storage, matrixPacking, maskArray, 16);
        case TLayoutBlockStorage::EbsStd430:
            return CommonGlslStructLayoutOf(begin, end, storage, matrixPacking, maskArray, 1);
        case TLayoutBlockStorage::EbsUnspecified:
            UNREACHABLE();
            return Layout::Invalid();
    }
}
