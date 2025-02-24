//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_MSL_LAYOUT_H_
#define COMPILER_TRANSLATOR_MSL_LAYOUT_H_

#include "common/angleutils.h"
#include "compiler/translator/Types.h"

namespace sh
{

constexpr const auto kDefaultLayoutBlockStorage  = TLayoutBlockStorage::EbsShared;
constexpr const auto kDefaultLayoutMatrixPacking = TLayoutMatrixPacking::EmpColumnMajor;
constexpr const auto kDefaultStructAlignmentSize = 16;

// Returns `oldStorage` if `type` has unspecified block storage.
// Otherwise returns block storage of `type`.
TLayoutBlockStorage Overlay(TLayoutBlockStorage oldStorage, const TType &type);

// Returns `oldPacking` if `type` has unspecified matrix packing.
// Otherwise returns matrix packing of `type`.
TLayoutMatrixPacking Overlay(TLayoutMatrixPacking oldPacking, const TType &type);

// Returns whether or not it is permissable for the block storage to use a packed representation.
bool CanBePacked(TLayoutBlockStorage storage);

// Returns whether or not it is permissable for the layout qualifier to use a packed representation.
bool CanBePacked(TLayoutQualifier layoutQualifier);

// Returns whether or not it is permissable for the type to use a packed representation.
bool CanBePacked(const TType &type);

// Sets the block storage for a type.
void SetBlockStorage(TType &type, TLayoutBlockStorage storage);

// Contains `sizeof` and `alignof` information.
struct Layout
{
    size_t sizeOf;
    size_t alignOf;

    static Layout Identity() { return {0, 1}; }
    static Layout Invalid() { return {0, 0}; }
    static Layout Both(size_t n) { return {n, n}; }

    void requireAlignment(size_t align, bool pad);

    bool operator==(const Layout &other) const;

    void operator+=(const Layout &other);
    void operator*=(size_t scale);

    Layout operator+(const Layout &other) const;
    Layout operator*(size_t scale) const;
};

struct MetalLayoutOfConfig
{
    bool disablePacking             = false;
    bool maskArray                  = false;
    bool treatSamplersAsTextureEnv  = false;
    bool assumeStructsAreTailPadded = false;  // Pad to multiple of 16
};

// Returns the layout of a type if it were to be represented in a Metal program.
// This deliberately ignores the TLayoutBlockStorage and TLayoutMatrixPacking of any type.
[[nodiscard]] Layout MetalLayoutOf(const TType &type, MetalLayoutOfConfig config = {});

// Returns the layout of a type if it were to be represented in a GLSL program.
[[nodiscard]] Layout GlslLayoutOf(
    const TType &type,
    TLayoutBlockStorage storage        = TLayoutBlockStorage::EbsUnspecified,
    TLayoutMatrixPacking matrixPacking = TLayoutMatrixPacking::EmpUnspecified,
    bool maskArray                     = false);

// Returns the layout of a structure if it were to be represented in a GLSL program.
[[nodiscard]] Layout GlslStructLayoutOf(TField const *const *begin,
                                        TField const *const *end,
                                        TLayoutBlockStorage storage,
                                        TLayoutMatrixPacking matrixPacking,
                                        bool maskArray = false);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_LAYOUT_H_
