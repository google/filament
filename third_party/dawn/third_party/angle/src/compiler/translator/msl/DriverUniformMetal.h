//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DriverUniformMetal:
//   Struct defining the default driver uniforms for direct and SpirV based ANGLE translation
//

#ifndef COMPILER_TRANSLATOR_MSL_DRIVERUNIFORMMETAL_H_
#define COMPILER_TRANSLATOR_MSL_DRIVERUNIFORMMETAL_H_

#include "compiler/translator/tree_util/DriverUniform.h"

namespace sh
{

class DriverUniformMetal : public DriverUniformExtended
{
  public:
    DriverUniformMetal(DriverUniformMode mode) : DriverUniformExtended(mode) {}
    DriverUniformMetal() : DriverUniformExtended(DriverUniformMode::InterfaceBlock) {}
    ~DriverUniformMetal() override {}

    TIntermTyped *getCoverageMaskField() const;

  protected:
    TFieldList *createUniformFields(TSymbolTable *symbolTable) override;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_DRIVERUNIFORMMETAL_H_
