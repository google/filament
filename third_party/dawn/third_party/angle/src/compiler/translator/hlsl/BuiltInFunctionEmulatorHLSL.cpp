//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/hlsl/BuiltInFunctionEmulatorHLSL.h"
#include "angle_gl.h"
#include "compiler/translator/BuiltInFunctionEmulator.h"
#include "compiler/translator/tree_util/BuiltIn.h"

namespace sh
{

// Defined in emulated_builtin_functions_hlsl_autogen.cpp.
const char *FindHLSLFunction(int uniqueId);

void InitBuiltInIsnanFunctionEmulatorForHLSLWorkarounds(BuiltInFunctionEmulator *emu,
                                                        int targetGLSLVersion)
{
    if (targetGLSLVersion < 130)
        return;

    emu->addEmulatedFunction(BuiltInId::isnan_Float1,
                             "bool isnan_emu(float x)\n"
                             "{\n"
                             "    return (x > 0.0 || x < 0.0) ? false : x != 0.0;\n"
                             "}\n"
                             "\n");

    emu->addEmulatedFunction(
        BuiltInId::isnan_Float2,
        "bool2 isnan_emu(float2 x)\n"
        "{\n"
        "    bool2 isnan;\n"
        "    for (int i = 0; i < 2; i++)\n"
        "    {\n"
        "        isnan[i] = (x[i] > 0.0 || x[i] < 0.0) ? false : x[i] != 0.0;\n"
        "    }\n"
        "    return isnan;\n"
        "}\n");

    emu->addEmulatedFunction(
        BuiltInId::isnan_Float3,
        "bool3 isnan_emu(float3 x)\n"
        "{\n"
        "    bool3 isnan;\n"
        "    for (int i = 0; i < 3; i++)\n"
        "    {\n"
        "        isnan[i] = (x[i] > 0.0 || x[i] < 0.0) ? false : x[i] != 0.0;\n"
        "    }\n"
        "    return isnan;\n"
        "}\n");

    emu->addEmulatedFunction(
        BuiltInId::isnan_Float4,
        "bool4 isnan_emu(float4 x)\n"
        "{\n"
        "    bool4 isnan;\n"
        "    for (int i = 0; i < 4; i++)\n"
        "    {\n"
        "        isnan[i] = (x[i] > 0.0 || x[i] < 0.0) ? false : x[i] != 0.0;\n"
        "    }\n"
        "    return isnan;\n"
        "}\n");
}

void InitBuiltInFunctionEmulatorForHLSL(BuiltInFunctionEmulator *emu)
{
    emu->addFunctionMap(FindHLSLFunction);

    // (a + b2^16) * (c + d2^16) = ac + (ad + bc) * 2^16 + bd * 2^32
    // Also note that below, a * d + ((a * c) >> 16) is guaranteed not to overflow, because:
    // a <= 0xffff, d <= 0xffff, ((a * c) >> 16) <= 0xffff and 0xffff * 0xffff + 0xffff = 0xffff0000
    emu->addEmulatedFunction(BuiltInId::umulExtended_UInt1_UInt1_UInt1_UInt1,
                             "void umulExtended_emu(uint x, uint y, out uint msb, out uint lsb)\n"
                             "{\n"
                             "    lsb = x * y;\n"
                             "    uint a = (x & 0xffffu);\n"
                             "    uint b = (x >> 16);\n"
                             "    uint c = (y & 0xffffu);\n"
                             "    uint d = (y >> 16);\n"
                             "    uint ad = a * d + ((a * c) >> 16);\n"
                             "    uint bc = b * c;\n"
                             "    uint carry = uint(ad > (0xffffffffu - bc));\n"
                             "    msb = ((ad + bc) >> 16) + (carry << 16) + b * d;\n"
                             "}\n");
    emu->addEmulatedFunctionWithDependency(
        BuiltInId::umulExtended_UInt1_UInt1_UInt1_UInt1,
        BuiltInId::umulExtended_UInt2_UInt2_UInt2_UInt2,
        "void umulExtended_emu(uint2 x, uint2 y, out uint2 msb, out uint2 lsb)\n"
        "{\n"
        "    umulExtended_emu(x.x, y.x, msb.x, lsb.x);\n"
        "    umulExtended_emu(x.y, y.y, msb.y, lsb.y);\n"
        "}\n");
    emu->addEmulatedFunctionWithDependency(
        BuiltInId::umulExtended_UInt1_UInt1_UInt1_UInt1,
        BuiltInId::umulExtended_UInt3_UInt3_UInt3_UInt3,
        "void umulExtended_emu(uint3 x, uint3 y, out uint3 msb, out uint3 lsb)\n"
        "{\n"
        "    umulExtended_emu(x.x, y.x, msb.x, lsb.x);\n"
        "    umulExtended_emu(x.y, y.y, msb.y, lsb.y);\n"
        "    umulExtended_emu(x.z, y.z, msb.z, lsb.z);\n"
        "}\n");
    emu->addEmulatedFunctionWithDependency(
        BuiltInId::umulExtended_UInt1_UInt1_UInt1_UInt1,
        BuiltInId::umulExtended_UInt4_UInt4_UInt4_UInt4,
        "void umulExtended_emu(uint4 x, uint4 y, out uint4 msb, out uint4 lsb)\n"
        "{\n"
        "    umulExtended_emu(x.x, y.x, msb.x, lsb.x);\n"
        "    umulExtended_emu(x.y, y.y, msb.y, lsb.y);\n"
        "    umulExtended_emu(x.z, y.z, msb.z, lsb.z);\n"
        "    umulExtended_emu(x.w, y.w, msb.w, lsb.w);\n"
        "}\n");

    // The imul emulation does two's complement negation on the lsb and msb manually in case the
    // result needs to be negative.
    // TODO(oetuaho): Note that this code doesn't take one edge case into account, where x or y is
    // -2^31. abs(-2^31) is undefined.
    emu->addEmulatedFunctionWithDependency(
        BuiltInId::umulExtended_UInt1_UInt1_UInt1_UInt1,
        BuiltInId::imulExtended_Int1_Int1_Int1_Int1,
        "void imulExtended_emu(int x, int y, out int msb, out int lsb)\n"
        "{\n"
        "    uint unsignedMsb;\n"
        "    uint unsignedLsb;\n"
        "    bool negative = (x < 0) != (y < 0);\n"
        "    umulExtended_emu(uint(abs(x)), uint(abs(y)), unsignedMsb, unsignedLsb);\n"
        "    lsb = asint(unsignedLsb);\n"
        "    msb = asint(unsignedMsb);\n"
        "    if (negative)\n"
        "    {\n"
        "        lsb = ~lsb;\n"
        "        msb = ~msb;\n"
        "        if (lsb == 0xffffffff)\n"
        "        {\n"
        "            lsb = 0;\n"
        "            msb += 1;\n"
        "        }\n"
        "        else\n"
        "        {\n"
        "            lsb += 1;\n"
        "        }\n"
        "    }\n"
        "}\n");
    emu->addEmulatedFunctionWithDependency(
        BuiltInId::imulExtended_Int1_Int1_Int1_Int1, BuiltInId::imulExtended_Int2_Int2_Int2_Int2,
        "void imulExtended_emu(int2 x, int2 y, out int2 msb, out int2 lsb)\n"
        "{\n"
        "    imulExtended_emu(x.x, y.x, msb.x, lsb.x);\n"
        "    imulExtended_emu(x.y, y.y, msb.y, lsb.y);\n"
        "}\n");
    emu->addEmulatedFunctionWithDependency(
        BuiltInId::imulExtended_Int1_Int1_Int1_Int1, BuiltInId::imulExtended_Int3_Int3_Int3_Int3,
        "void imulExtended_emu(int3 x, int3 y, out int3 msb, out int3 lsb)\n"
        "{\n"
        "    imulExtended_emu(x.x, y.x, msb.x, lsb.x);\n"
        "    imulExtended_emu(x.y, y.y, msb.y, lsb.y);\n"
        "    imulExtended_emu(x.z, y.z, msb.z, lsb.z);\n"
        "}\n");
    emu->addEmulatedFunctionWithDependency(
        BuiltInId::imulExtended_Int1_Int1_Int1_Int1, BuiltInId::imulExtended_Int4_Int4_Int4_Int4,
        "void imulExtended_emu(int4 x, int4 y, out int4 msb, out int4 lsb)\n"
        "{\n"
        "    imulExtended_emu(x.x, y.x, msb.x, lsb.x);\n"
        "    imulExtended_emu(x.y, y.y, msb.y, lsb.y);\n"
        "    imulExtended_emu(x.z, y.z, msb.z, lsb.z);\n"
        "    imulExtended_emu(x.w, y.w, msb.w, lsb.w);\n"
        "}\n");
}

}  // namespace sh
