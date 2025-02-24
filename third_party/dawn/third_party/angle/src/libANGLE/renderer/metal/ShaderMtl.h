//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderMtl.h:
//    Defines the class interface for ShaderMtl, implementing ShaderImpl.
//
#ifndef LIBANGLE_RENDERER_METAL_SHADERMTL_H_
#define LIBANGLE_RENDERER_METAL_SHADERMTL_H_

#include <map>

#include "libANGLE/renderer/ShaderImpl.h"
#include "libANGLE/renderer/metal/mtl_msl_utils.h"

namespace rx
{
class ShaderMtl : public ShaderImpl
{
  public:
    ShaderMtl(const gl::ShaderState &state);
    ~ShaderMtl() override;

    std::shared_ptr<ShaderTranslateTask> compile(const gl::Context *context,
                                                 ShCompileOptions *options) override;
    std::shared_ptr<ShaderTranslateTask> load(const gl::Context *context,
                                              gl::BinaryInputStream *stream) override;

    const SharedCompiledShaderStateMtl &getCompiledState() const { return mCompiledState; }

    std::string getDebugInfo() const override;

  private:
    SharedCompiledShaderStateMtl mCompiledState;
};

}  // namespace rx

#endif /* LIBANGLE_RENDERER_METAL_SHADERMTL_H_ */
