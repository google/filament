//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// blocklayout.h:
//   Methods and classes related to uniform layout and packing in GLSL and HLSL.
//

#ifndef COMMON_BLOCKLAYOUTHLSL_H_
#define COMMON_BLOCKLAYOUTHLSL_H_

#include <cstddef>
#include <vector>

#include <GLSLANG/ShaderLang.h>
#include "angle_gl.h"
#include "compiler/translator/blocklayout.h"

namespace sh
{
// Block layout packed according to the D3D9 or default D3D10+ register packing rules
// See http://msdn.microsoft.com/en-us/library/windows/desktop/bb509632(v=vs.85).aspx
// The strategy should be ENCODE_LOOSE for D3D9 constant blocks, and ENCODE_PACKED
// for everything else (D3D10+ constant blocks and all attributes/varyings).

class HLSLBlockEncoder : public BlockLayoutEncoder
{
  public:
    enum HLSLBlockEncoderStrategy
    {
        ENCODE_PACKED,
        ENCODE_LOOSE
    };

    HLSLBlockEncoder(HLSLBlockEncoderStrategy strategy, bool transposeMatrices);

    void enterAggregateType(const ShaderVariable &structVar) override;
    void exitAggregateType(const ShaderVariable &structVar) override;
    void skipRegisters(unsigned int numRegisters);

    bool isPacked() const { return mEncoderStrategy == ENCODE_PACKED; }

    static HLSLBlockEncoderStrategy GetStrategyFor(ShShaderOutput outputType);

  protected:
    void getBlockLayoutInfo(GLenum type,
                            const std::vector<unsigned int> &arraySizes,
                            bool isRowMajorMatrix,
                            int *arrayStrideOut,
                            int *matrixStrideOut) override;
    void advanceOffset(GLenum type,
                       const std::vector<unsigned int> &arraySizes,
                       bool isRowMajorMatrix,
                       int arrayStride,
                       int matrixStride) override;

    HLSLBlockEncoderStrategy mEncoderStrategy;
    bool mTransposeMatrices;
};

// This method returns the number of used registers for a ShaderVariable. It is dependent on the
// HLSLBlockEncoder class to count the number of used registers in a struct (which are individually
// packed according to the same rules).
unsigned int HLSLVariableRegisterCount(const ShaderVariable &variable, ShShaderOutput outputType);
}  // namespace sh

#endif  // COMMON_BLOCKLAYOUTHLSL_H_
