//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/msl/MapSymbols.h"
#include "compiler/translator/IntermRebuild.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

namespace
{

class Rewriter : public TIntermRebuild
{
  private:
    std::function<TIntermNode &(const TFunction *, TIntermSymbol &)> mMap;

  public:
    Rewriter(TCompiler &compiler,
             std::function<TIntermNode &(const TFunction *, TIntermSymbol &)> map)
        : TIntermRebuild(compiler, false, true), mMap(map)
    {}

    PostResult visitSymbolPost(TIntermSymbol &symbolNode) override
    {
        return mMap(getParentFunction(), symbolNode);
    }
};

}  // namespace

bool sh::MapSymbols(TCompiler &compiler,
                    TIntermBlock &root,
                    std::function<TIntermNode &(const TFunction *, TIntermSymbol &)> map)
{
    Rewriter rewriter(compiler, std::move(map));
    if (!rewriter.rebuildRoot(root))
    {
        return false;
    }
    return true;
}
