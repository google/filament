//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/tree_ops/RewritePixelLocalStorage.h"

#include "common/angleutils.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_ops/MonomorphizeUnsupportedFunctions.h"
#include "compiler/translator/tree_util/BuiltIn.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{
namespace
{
constexpr static TBasicType DataTypeOfPLSType(TBasicType plsType)
{
    switch (plsType)
    {
        case EbtPixelLocalANGLE:
            return EbtFloat;
        case EbtIPixelLocalANGLE:
            return EbtInt;
        case EbtUPixelLocalANGLE:
            return EbtUInt;
        default:
            UNREACHABLE();
            return EbtVoid;
    }
}

constexpr static TBasicType DataTypeOfImageType(TBasicType imageType)
{
    switch (imageType)
    {
        case EbtImage2D:
            return EbtFloat;
        case EbtIImage2D:
            return EbtInt;
        case EbtUImage2D:
            return EbtUInt;
        default:
            UNREACHABLE();
            return EbtVoid;
    }
}

// Maps PLS symbols to a backing store.
template <typename T>
class PLSBackingStoreMap
{
  public:
    // Sets the given variable as the backing storage for the plsSymbol's binding point. An entry
    // must not already exist in the map for this binding point.
    void insertNew(TIntermSymbol *plsSymbol, const T &backingStore)
    {
        ASSERT(plsSymbol);
        ASSERT(IsPixelLocal(plsSymbol->getBasicType()));
        int binding = plsSymbol->getType().getLayoutQualifier().binding;
        ASSERT(binding >= 0);
        auto result = mMap.insert({binding, backingStore});
        ASSERT(result.second);  // Ensure an image didn't already exist for this symbol.
    }

    // Looks up the backing store for the given plsSymbol's binding point. An entry must already
    // exist in the map for this binding point.
    const T &find(TIntermSymbol *plsSymbol)
    {
        ASSERT(plsSymbol);
        ASSERT(IsPixelLocal(plsSymbol->getBasicType()));
        int binding = plsSymbol->getType().getLayoutQualifier().binding;
        ASSERT(binding >= 0);
        auto iter = mMap.find(binding);
        ASSERT(iter != mMap.end());  // Ensure PLSImages already exist for this symbol.
        return iter->second;
    }

    const std::map<int, T> &bindingOrderedMap() const { return mMap; }

  private:
    // Use std::map so the backing stores are ordered by binding when we iterate.
    std::map<int, T> mMap;
};

// Base class for rewriting high level PLS operations to AST operations specified by
// ShPixelLocalStorageType.
class RewritePLSTraverser : public TIntermTraverser
{
  public:
    RewritePLSTraverser(TCompiler *compiler,
                        TSymbolTable &symbolTable,
                        const ShCompileOptions &compileOptions,
                        int shaderVersion)
        : TIntermTraverser(true, false, false, &symbolTable),
          mCompiler(compiler),
          mCompileOptions(&compileOptions),
          mShaderVersion(shaderVersion)
    {}

    bool visitDeclaration(Visit, TIntermDeclaration *decl) override
    {
        TIntermTyped *declVariable = (decl->getSequence())->front()->getAsTyped();
        ASSERT(declVariable);

        if (!IsPixelLocal(declVariable->getBasicType()))
        {
            return true;
        }

        // PLS is not allowed in arrays.
        ASSERT(!declVariable->isArray());

        // This visitDeclaration doesn't get called for function arguments, and opaque types can
        // otherwise only be uniforms.
        ASSERT(declVariable->getQualifier() == EvqUniform);

        TIntermSymbol *plsSymbol = declVariable->getAsSymbolNode();
        ASSERT(plsSymbol);

        visitPLSDeclaration(plsSymbol);

        return false;
    }

    bool visitAggregate(Visit, TIntermAggregate *aggregate) override
    {
        if (!BuiltInGroup::IsPixelLocal(aggregate->getOp()))
        {
            return true;
        }

        const TIntermSequence &args = *aggregate->getSequence();
        ASSERT(args.size() >= 1);
        TIntermSymbol *plsSymbol = args[0]->getAsSymbolNode();

        // Rewrite pixelLocalLoadANGLE -> imageLoad.
        if (aggregate->getOp() == EOpPixelLocalLoadANGLE)
        {
            visitPLSLoad(plsSymbol);
            return false;  // No need to recurse since this node is being dropped.
        }

        // Rewrite pixelLocalStoreANGLE -> imageStore.
        if (aggregate->getOp() == EOpPixelLocalStoreANGLE)
        {
            // Also hoist the 'value' expression into a temp. In the event of
            // "pixelLocalStoreANGLE(..., pixelLocalLoadANGLE(...))", this ensures the load occurs
            // _before_ any potential barriers required by the subclass.
            //
            // NOTE: It is generally unsafe to hoist function arguments due to short circuiting,
            // e.g., "if (false && function(...))", but pixelLocalStoreANGLE returns type void, so
            // it is safe in this particular case.
            TType *valueType    = new TType(DataTypeOfPLSType(plsSymbol->getBasicType()),
                                            plsSymbol->getPrecision(), EvqTemporary, 4);
            TVariable *valueVar = CreateTempVariable(mSymbolTable, valueType);
            TIntermDeclaration *valueDecl =
                CreateTempInitDeclarationNode(valueVar, args[1]->getAsTyped());
            valueDecl->traverse(this);  // Rewrite any potential pixelLocalLoadANGLEs in valueDecl.
            insertStatementInParentBlock(valueDecl);

            visitPLSStore(plsSymbol, valueVar);
            return false;  // No need to recurse since this node is being dropped.
        }

        return true;
    }

    // Called after rewrite. Injects one-time setup code that needs to run before any PLS accesses.
    virtual void injectPrePLSCode(TCompiler *,
                                  TSymbolTable &,
                                  const ShCompileOptions &,
                                  TIntermBlock *mainBody,
                                  size_t plsBeginPosition)
    {}

    // Called after rewrite. Injects one-time finalization code that needs to run after all PLS.
    virtual void injectPostPLSCode(TCompiler *,
                                   TSymbolTable &,
                                   const ShCompileOptions &,
                                   TIntermBlock *mainBody,
                                   size_t plsEndPosition)
    {}

    // Called after all other operations have completed.
    void injectPixelCoordInitializationCodeIfNeeded(TCompiler *compiler,
                                                    TIntermBlock *root,
                                                    TIntermBlock *mainBody)
    {
        if (mGlobalPixelCoord)
        {
            // Initialize the global pixel coord at the beginning of main():
            //
            //     pixelCoord = ivec2(floor(gl_FragCoord.xy));
            //
            TIntermTyped *exp;
            exp = ReferenceBuiltInVariable(ImmutableString("gl_FragCoord"), *mSymbolTable,
                                           mShaderVersion);
            exp = CreateSwizzle(exp, 0, 1);
            exp = CreateBuiltInFunctionCallNode("floor", {exp}, *mSymbolTable, mShaderVersion);
            exp = TIntermAggregate::CreateConstructor(TType(EbtInt, 2), {exp});
            exp = CreateTempAssignmentNode(mGlobalPixelCoord, exp);
            mainBody->insertStatement(0, exp);
        }
    }

  protected:
    virtual void visitPLSDeclaration(TIntermSymbol *plsSymbol)             = 0;
    virtual void visitPLSLoad(TIntermSymbol *plsSymbol)                    = 0;
    virtual void visitPLSStore(TIntermSymbol *plsSymbol, TVariable *value) = 0;

    // Inserts a global to hold the pixel coordinate as soon as we see PLS declared. This will be
    // initialized at the beginning of main().
    void ensureGlobalPixelCoordDeclared()
    {
        if (!mGlobalPixelCoord)
        {
            TType *coordType  = new TType(EbtInt, EbpHigh, EvqGlobal, 2);
            mGlobalPixelCoord = CreateTempVariable(mSymbolTable, coordType);
            insertStatementInParentBlock(CreateTempDeclarationNode(mGlobalPixelCoord));
        }
    }

    // anglebug.com/42265993: Storing to integer formats with larger-than-representable values has
    // different behavior on the various APIs.
    //
    // This method clamps sub-32-bit integers to the min/max representable values of their format.
    void clampPLSVarIfNeeded(TVariable *plsVar, TLayoutImageInternalFormat plsFormat)
    {
        switch (plsFormat)
        {
            case EiifRGBA8I:
            {
                // Clamp r,g,b,a to their min/max 8-bit values:
                //
                //     plsVar = clamp(plsVar, -128, 127) & 0xff
                //
                TIntermTyped *newPLSValue = CreateBuiltInFunctionCallNode(
                    "clamp",
                    {new TIntermSymbol(plsVar), CreateIndexNode(-128), CreateIndexNode(127)},
                    *mSymbolTable, mShaderVersion);
                insertStatementInParentBlock(CreateTempAssignmentNode(plsVar, newPLSValue));
                break;
            }
            case EiifRGBA8UI:
            {
                // Clamp r,g,b,a to their max 8-bit values:
                //
                //     plsVar = min(plsVar, 255)
                //
                TIntermTyped *newPLSValue = CreateBuiltInFunctionCallNode(
                    "min", {new TIntermSymbol(plsVar), CreateUIntNode(255)}, *mSymbolTable,
                    mShaderVersion);
                insertStatementInParentBlock(CreateTempAssignmentNode(plsVar, newPLSValue));
                break;
            }
            default:
                break;
        }
    }

    // Expands an expression to 4 components, filling in the missing components with [0, 0, 0, 1].
    static TIntermTyped *Expand(TIntermTyped *expr)
    {
        const TType &type = expr->getType();
        ASSERT(type.getNominalSize() == 1 || type.getNominalSize() == 4);
        if (type.getNominalSize() == 1)
        {
            switch (type.getBasicType())
            {
                case EbtFloat:
                    expr = TIntermAggregate::CreateConstructor(  // "vec4(r, 0, 0, 1)"
                        TType(EbtFloat, 4),
                        {expr, CreateFloatNode(0, EbpLow), CreateFloatNode(0, EbpLow),
                         CreateFloatNode(1, EbpLow)});
                    break;
                case EbtUInt:
                    expr = TIntermAggregate::CreateConstructor(  // "uvec4(r, 0, 0, 1)"
                        TType(EbtUInt, 4),
                        {expr, CreateUIntNode(0), CreateUIntNode(0), CreateUIntNode(1)});
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        }
        return expr;
    }

    static TIntermTyped *Expand(TVariable *var) { return Expand(new TIntermSymbol(var)); }

    // Returns an expression that swizzles a variable down to 'n' components.
    static TIntermTyped *Swizzle(TVariable *var, int n)
    {
        TIntermTyped *swizzled = new TIntermSymbol(var);
        if (var->getType().getNominalSize() != n)
        {
            ASSERT(var->getType().getNominalSize() > n);
            TVector swizzleOffsets{0, 1, 2, 3};
            swizzleOffsets.resize(n);
            swizzled = new TIntermSwizzle(swizzled, swizzleOffsets);
        }
        return swizzled;
    }

    const TCompiler *const mCompiler;
    const ShCompileOptions *const mCompileOptions;
    const int mShaderVersion;

    // Stores the shader invocation's pixel coordinate as "ivec2(floor(gl_FragCoord.xy))".
    TVariable *mGlobalPixelCoord = nullptr;
};

// Rewrites high level PLS operations to shader image operations.
class RewritePLSToImagesTraverser : public RewritePLSTraverser
{
  public:
    RewritePLSToImagesTraverser(TCompiler *compiler,
                                TSymbolTable &symbolTable,
                                const ShCompileOptions &compileOptions,
                                int shaderVersion)
        : RewritePLSTraverser(compiler, symbolTable, compileOptions, shaderVersion)
    {}

  private:
    void visitPLSDeclaration(TIntermSymbol *plsSymbol) override
    {
        // Replace the PLS declaration with an image2D.
        ensureGlobalPixelCoordDeclared();
        TVariable *image2D = createPLSImageReplacement(plsSymbol);
        mImages.insertNew(plsSymbol, image2D);
        queueReplacement(new TIntermDeclaration({new TIntermSymbol(image2D)}),
                         OriginalNode::IS_DROPPED);
    }

    // Creates an image2D that replaces a pixel local storage handle.
    TVariable *createPLSImageReplacement(const TIntermSymbol *plsSymbol)
    {
        ASSERT(plsSymbol);
        ASSERT(IsPixelLocal(plsSymbol->getBasicType()));

        TType *imageType = new TType(plsSymbol->getType());

        TLayoutQualifier layoutQualifier = imageType->getLayoutQualifier();
        switch (layoutQualifier.imageInternalFormat)
        {
            case TLayoutImageInternalFormat::EiifRGBA8:
                if (!mCompileOptions->pls.supportsNativeRGBA8ImageFormats)
                {
                    layoutQualifier.imageInternalFormat = EiifR32UI;
                    imageType->setPrecision(EbpHigh);
                    imageType->setBasicType(EbtUImage2D);
                }
                else
                {
                    imageType->setBasicType(EbtImage2D);
                }
                break;
            case TLayoutImageInternalFormat::EiifRGBA8I:
                if (!mCompileOptions->pls.supportsNativeRGBA8ImageFormats)
                {
                    layoutQualifier.imageInternalFormat = EiifR32I;
                    imageType->setPrecision(EbpHigh);
                }
                imageType->setBasicType(EbtIImage2D);
                break;
            case TLayoutImageInternalFormat::EiifRGBA8UI:
                if (!mCompileOptions->pls.supportsNativeRGBA8ImageFormats)
                {
                    layoutQualifier.imageInternalFormat = EiifR32UI;
                    imageType->setPrecision(EbpHigh);
                }
                imageType->setBasicType(EbtUImage2D);
                break;
            case TLayoutImageInternalFormat::EiifR32F:
                imageType->setBasicType(EbtImage2D);
                break;
            case TLayoutImageInternalFormat::EiifR32UI:
                imageType->setBasicType(EbtUImage2D);
                break;
            default:
                UNREACHABLE();
        }
        layoutQualifier.rasterOrdered =
            mCompileOptions->pls.fragmentSyncType ==
                ShFragmentSynchronizationType::RasterizerOrderViews_D3D ||
            mCompileOptions->pls.fragmentSyncType ==
                ShFragmentSynchronizationType::RasterOrderGroups_Metal;
        imageType->setLayoutQualifier(layoutQualifier);

        TMemoryQualifier memoryQualifier{};
        memoryQualifier.coherent          = true;
        memoryQualifier.restrictQualifier = true;
        memoryQualifier.volatileQualifier = false;
        // TODO(anglebug.com/40096838): Maybe we could walk the tree first and see which PLS is used
        // how. If the PLS is never loaded, we could add a writeonly qualifier, for example.
        memoryQualifier.readonly  = false;
        memoryQualifier.writeonly = false;
        imageType->setMemoryQualifier(memoryQualifier);

        const TVariable &plsVar = plsSymbol->variable();
        return new TVariable(plsVar.uniqueId(), plsVar.name(), plsVar.symbolType(),
                             plsVar.extensions(), imageType);
    }

    void visitPLSLoad(TIntermSymbol *plsSymbol) override
    {
        // Replace the pixelLocalLoadANGLE with imageLoad.
        TVariable *image2D = mImages.find(plsSymbol);
        ASSERT(mGlobalPixelCoord);
        TIntermTyped *pls = CreateBuiltInFunctionCallNode(
            "imageLoad", {new TIntermSymbol(image2D), new TIntermSymbol(mGlobalPixelCoord)},
            *mSymbolTable, 310);
        pls = unpackImageDataIfNecessary(pls, plsSymbol, image2D);
        queueReplacement(pls, OriginalNode::IS_DROPPED);
    }

    // Unpacks the raw PLS data if the output shader language needs r32* packing.
    TIntermTyped *unpackImageDataIfNecessary(TIntermTyped *data,
                                             TIntermSymbol *plsSymbol,
                                             TVariable *image2D)
    {
        TLayoutImageInternalFormat plsFormat =
            plsSymbol->getType().getLayoutQualifier().imageInternalFormat;
        TLayoutImageInternalFormat imageFormat =
            image2D->getType().getLayoutQualifier().imageInternalFormat;
        if (plsFormat == imageFormat)
        {
            return data;  // This PLS storage isn't packed.
        }
        switch (plsFormat)
        {
            case EiifRGBA8:
                ASSERT(!mCompileOptions->pls.supportsNativeRGBA8ImageFormats);
                // Unpack and normalize r,g,b,a from a single 32-bit unsigned int:
                //
                //     unpackUnorm4x8(data.r)
                //
                data = CreateBuiltInFunctionCallNode("unpackUnorm4x8", {CreateSwizzle(data, 0)},
                                                     *mSymbolTable, 310);
                break;
            case EiifRGBA8I:
            case EiifRGBA8UI:
            {
                ASSERT(!mCompileOptions->pls.supportsNativeRGBA8ImageFormats);
                constexpr unsigned shifts[] = {24, 16, 8, 0};
                // Unpack r,g,b,a form a single (signed or unsigned) 32-bit int. Shift left,
                // then right, to preserve the sign for ints. (highp integers are exactly
                // 32-bit, two's compliment.)
                //
                //     data.rrrr << uvec4(24, 16, 8, 0) >> 24u
                //
                data = CreateSwizzle(data, 0, 0, 0, 0);
                data = new TIntermBinary(EOpBitShiftLeft, data, CreateUVecNode(shifts, 4, EbpLow));
                data = new TIntermBinary(EOpBitShiftRight, data, CreateUIntNode(24));
                break;
            }
            default:
                UNREACHABLE();
        }
        return data;
    }

    void visitPLSStore(TIntermSymbol *plsSymbol, TVariable *value) override
    {
        TVariable *image2D       = mImages.find(plsSymbol);
        TIntermTyped *packedData = clampAndPackPLSDataIfNecessary(value, plsSymbol, image2D);

        // Surround the store with memoryBarrierImage calls in order to ensure dependent stores and
        // loads in a single shader invocation are coherent. From the ES 3.1 spec:
        //
        //   Using variables declared as "coherent" guarantees only that the results of stores will
        //   be immediately visible to shader invocations using similarly-declared variables;
        //   calling MemoryBarrier is required to ensure that the stores are visible to other
        //   operations.
        //
        insertStatementsInParentBlock(
            {CreateBuiltInFunctionCallNode("memoryBarrierImage", {}, *mSymbolTable,
                                           310)},  // Before.
            {CreateBuiltInFunctionCallNode("memoryBarrierImage", {}, *mSymbolTable,
                                           310)});  // After.

        // Rewrite the pixelLocalStoreANGLE with imageStore.
        ASSERT(mGlobalPixelCoord);
        queueReplacement(
            CreateBuiltInFunctionCallNode(
                "imageStore",
                {new TIntermSymbol(image2D), new TIntermSymbol(mGlobalPixelCoord), packedData},
                *mSymbolTable, 310),
            OriginalNode::IS_DROPPED);
    }

    // Packs the PLS to raw data if the output shader language needs r32* packing.
    TIntermTyped *clampAndPackPLSDataIfNecessary(TVariable *plsVar,
                                                 TIntermSymbol *plsSymbol,
                                                 TVariable *image2D)
    {
        TLayoutImageInternalFormat plsFormat =
            plsSymbol->getType().getLayoutQualifier().imageInternalFormat;
        clampPLSVarIfNeeded(plsVar, plsFormat);
        TIntermTyped *result = new TIntermSymbol(plsVar);
        TLayoutImageInternalFormat imageFormat =
            image2D->getType().getLayoutQualifier().imageInternalFormat;
        if (plsFormat == imageFormat)
        {
            return result;  // This PLS storage isn't packed.
        }
        switch (plsFormat)
        {
            case EiifRGBA8:
            {
                ASSERT(!mCompileOptions->pls.supportsNativeRGBA8ImageFormats);
                if (mCompileOptions->passHighpToPackUnormSnormBuiltins)
                {
                    // anglebug.com/42265995: unpackUnorm4x8 doesn't work on Pixel 4 when passed
                    // a mediump vec4. Use an intermediate highp vec4.
                    //
                    // It's safe to inject a variable here because it happens right before
                    // pixelLocalStoreANGLE, which returns type void. (See visitAggregate.)
                    TType *highpType              = new TType(EbtFloat, EbpHigh, EvqTemporary, 4);
                    TVariable *workaroundHighpVar = CreateTempVariable(mSymbolTable, highpType);
                    insertStatementInParentBlock(
                        CreateTempInitDeclarationNode(workaroundHighpVar, result));
                    result = new TIntermSymbol(workaroundHighpVar);
                }

                // Denormalize and pack r,g,b,a into a single 32-bit unsigned int:
                //
                //     packUnorm4x8(workaroundHighpVar)
                //
                result =
                    CreateBuiltInFunctionCallNode("packUnorm4x8", {result}, *mSymbolTable, 310);
                break;
            }
            case EiifRGBA8I:
            case EiifRGBA8UI:
            {
                ASSERT(!mCompileOptions->pls.supportsNativeRGBA8ImageFormats);
                if (plsFormat == EiifRGBA8I)
                {
                    // Mask off extra sign bits beyond 8.
                    //
                    //     plsVar &= 0xff
                    //
                    insertStatementInParentBlock(new TIntermBinary(
                        EOpBitwiseAndAssign, new TIntermSymbol(plsVar), CreateIndexNode(0xff)));
                }
                // Pack r,g,b,a into a single 32-bit (signed or unsigned) int:
                //
                //     r | (g << 8) | (b << 16) | (a << 24)
                //
                auto shiftComponent = [=](int componentIdx) {
                    return new TIntermBinary(EOpBitShiftLeft,
                                             CreateSwizzle(new TIntermSymbol(plsVar), componentIdx),
                                             CreateUIntNode(componentIdx * 8));
                };
                result = CreateSwizzle(result, 0);
                result = new TIntermBinary(EOpBitwiseOr, result, shiftComponent(1));
                result = new TIntermBinary(EOpBitwiseOr, result, shiftComponent(2));
                result = new TIntermBinary(EOpBitwiseOr, result, shiftComponent(3));
                break;
            }
            default:
                UNREACHABLE();
        }
        // Convert the packed data to a {u,i}vec4 for imageStore.
        TType imageStoreType(DataTypeOfImageType(image2D->getType().getBasicType()), 4);
        return TIntermAggregate::CreateConstructor(imageStoreType, {result});
    }

    void injectPrePLSCode(TCompiler *compiler,
                          TSymbolTable &symbolTable,
                          const ShCompileOptions &compileOptions,
                          TIntermBlock *mainBody,
                          size_t plsBeginPosition) override
    {
        // When PLS is implemented with images, early_fragment_tests ensure that depth/stencil
        // can also block stores to PLS.
        compiler->specifyEarlyFragmentTests();

        // Delimit the beginning of a per-pixel critical section, if supported. This makes pixel
        // local storage coherent.
        //
        // Either: GL_NV_fragment_shader_interlock
        //         GL_INTEL_fragment_shader_ordering
        //         GL_ARB_fragment_shader_interlock (may compile to
        //                                           SPV_EXT_fragment_shader_interlock)
        switch (compileOptions.pls.fragmentSyncType)
        {
            // Raster ordered resources don't need explicit synchronization calls.
            case ShFragmentSynchronizationType::RasterizerOrderViews_D3D:
            case ShFragmentSynchronizationType::RasterOrderGroups_Metal:
            case ShFragmentSynchronizationType::NotSupported:
                break;
            case ShFragmentSynchronizationType::FragmentShaderInterlock_NV_GL:
                mainBody->insertStatement(
                    plsBeginPosition,
                    CreateBuiltInFunctionCallNode("beginInvocationInterlockNV", {}, symbolTable,
                                                  kESSLInternalBackendBuiltIns));
                break;
            case ShFragmentSynchronizationType::FragmentShaderOrdering_INTEL_GL:
                mainBody->insertStatement(
                    plsBeginPosition,
                    CreateBuiltInFunctionCallNode("beginFragmentShaderOrderingINTEL", {},
                                                  symbolTable, kESSLInternalBackendBuiltIns));
                break;
            case ShFragmentSynchronizationType::FragmentShaderInterlock_ARB_GL:
                mainBody->insertStatement(
                    plsBeginPosition,
                    CreateBuiltInFunctionCallNode("beginInvocationInterlockARB", {}, symbolTable,
                                                  kESSLInternalBackendBuiltIns));
                break;
            default:
                UNREACHABLE();
        }
    }

    void injectPostPLSCode(TCompiler *,
                           TSymbolTable &symbolTable,
                           const ShCompileOptions &compileOptions,
                           TIntermBlock *mainBody,
                           size_t plsEndPosition) override
    {
        // Delimit the end of the PLS critical section, if required.
        //
        // Either: GL_NV_fragment_shader_interlock
        //         GL_ARB_fragment_shader_interlock (may compile to
        //                                           SPV_EXT_fragment_shader_interlock)
        switch (compileOptions.pls.fragmentSyncType)
        {
            // Raster ordered resources don't need explicit synchronization calls.
            case ShFragmentSynchronizationType::RasterizerOrderViews_D3D:
            case ShFragmentSynchronizationType::RasterOrderGroups_Metal:
            // GL_INTEL_fragment_shader_ordering doesn't have an "end()" call.
            case ShFragmentSynchronizationType::FragmentShaderOrdering_INTEL_GL:
            case ShFragmentSynchronizationType::NotSupported:
                break;
            case ShFragmentSynchronizationType::FragmentShaderInterlock_NV_GL:

                mainBody->insertStatement(
                    plsEndPosition,
                    CreateBuiltInFunctionCallNode("endInvocationInterlockNV", {}, symbolTable,
                                                  kESSLInternalBackendBuiltIns));
                break;
            case ShFragmentSynchronizationType::FragmentShaderInterlock_ARB_GL:
                mainBody->insertStatement(
                    plsEndPosition,
                    CreateBuiltInFunctionCallNode("endInvocationInterlockARB", {}, symbolTable,
                                                  kESSLInternalBackendBuiltIns));
                break;
            default:
                UNREACHABLE();
        }
    }

    PLSBackingStoreMap<TVariable *> mImages;
};

// Rewrites high level PLS operations to framebuffer fetch operations.
class RewritePLSToFramebufferFetchTraverser : public RewritePLSTraverser
{
  public:
    RewritePLSToFramebufferFetchTraverser(TCompiler *compiler,
                                          TSymbolTable &symbolTable,
                                          const ShCompileOptions &compileOptions,
                                          int shaderVersion)
        : RewritePLSTraverser(compiler, symbolTable, compileOptions, shaderVersion)
    {}

    void visitPLSDeclaration(TIntermSymbol *plsSymbol) override
    {
        // Replace the PLS declaration with a framebuffer attachment.
        PLSAttachment attachment(mCompiler, mSymbolTable, *mCompileOptions, plsSymbol->variable());
        mPLSAttachments.insertNew(plsSymbol, attachment);
        insertStatementInParentBlock(
            new TIntermDeclaration({new TIntermSymbol(attachment.fragmentVar)}));
        queueReplacement(CreateTempDeclarationNode(attachment.accessVar), OriginalNode::IS_DROPPED);
    }

    void visitPLSLoad(TIntermSymbol *plsSymbol) override
    {
        // Read our temporary accessVar.
        const PLSAttachment &attachment = mPLSAttachments.find(plsSymbol);
        queueReplacement(Expand(attachment.accessVar), OriginalNode::IS_DROPPED);
    }

    void visitPLSStore(TIntermSymbol *plsSymbol, TVariable *value) override
    {
        // Set our temporary accessVar.
        const PLSAttachment &attachment = mPLSAttachments.find(plsSymbol);
        queueReplacement(CreateTempAssignmentNode(attachment.accessVar, attachment.swizzle(value)),
                         OriginalNode::IS_DROPPED);
    }

    void injectPrePLSCode(TCompiler *compiler,
                          TSymbolTable &symbolTable,
                          const ShCompileOptions &compileOptions,
                          TIntermBlock *mainBody,
                          size_t plsBeginPosition) override
    {
        // [OpenGL ES Version 3.0.6, 3.9.2.3 "Shader Output"]: Any colors, or color components,
        // associated with a fragment that are not written by the fragment shader are undefined.
        //
        // [EXT_shader_framebuffer_fetch]: Prior to fragment shading, fragment outputs declared
        // inout are populated with the value last written to the framebuffer at the same(x, y,
        // sample) position.
        //
        // It's unclear from the EXT_shader_framebuffer_fetch spec whether inout fragment variables
        // become undefined if not explicitly written, but either way, when this compiles to subpass
        // loads in Vulkan, we definitely get undefined behavior if PLS variables are not written.
        //
        // To make sure every PLS variable gets written, we read them all before PLS operations,
        // then write them all back out after all PLS is complete.
        TIntermSequence plsPreloads;
        plsPreloads.reserve(mPLSAttachments.bindingOrderedMap().size());
        for (const auto &entry : mPLSAttachments.bindingOrderedMap())
        {
            const PLSAttachment &attachment = entry.second;
            plsPreloads.push_back(
                CreateTempAssignmentNode(attachment.accessVar, attachment.swizzleFragmentVar()));
        }
        mainBody->getSequence()->insert(mainBody->getSequence()->begin() + plsBeginPosition,
                                        plsPreloads.begin(), plsPreloads.end());
    }

    void injectPostPLSCode(TCompiler *,
                           TSymbolTable &symbolTable,
                           const ShCompileOptions &compileOptions,
                           TIntermBlock *mainBody,
                           size_t plsEndPosition) override
    {
        TIntermSequence plsWrites;
        plsWrites.reserve(mPLSAttachments.bindingOrderedMap().size());
        for (const auto &entry : mPLSAttachments.bindingOrderedMap())
        {
            const PLSAttachment &attachment = entry.second;
            plsWrites.push_back(new TIntermBinary(EOpAssign, attachment.swizzleFragmentVar(),
                                                  new TIntermSymbol(attachment.accessVar)));
        }
        mainBody->getSequence()->insert(mainBody->getSequence()->begin() + plsEndPosition,
                                        plsWrites.begin(), plsWrites.end());
    }

  private:
    struct PLSAttachment
    {
        PLSAttachment(const TCompiler *compiler,
                      TSymbolTable *symbolTable,
                      const ShCompileOptions &compileOptions,
                      const TVariable &plsVar)
        {
            const TType &plsType = plsVar.getType();

            TType *accessVarType;
            switch (plsType.getLayoutQualifier().imageInternalFormat)
            {
                default:
                    UNREACHABLE();
                    [[fallthrough]];
                case EiifRGBA8:
                    accessVarType = new TType(EbtFloat, 4);
                    break;
                case EiifRGBA8I:
                    accessVarType = new TType(EbtInt, 4);
                    break;
                case EiifRGBA8UI:
                    accessVarType = new TType(EbtUInt, 4);
                    break;
                case EiifR32F:
                    accessVarType = new TType(EbtFloat, 1);
                    break;
                case EiifR32UI:
                    accessVarType = new TType(EbtUInt, 1);
                    break;
            }
            accessVarType->setPrecision(plsType.getPrecision());
            accessVar = CreateTempVariable(symbolTable, accessVarType);

            // Qualcomm seems to want fragment outputs to be 4-component vectors, and produces a
            // compile error from "inout uint". Our Metal translator also saturates color outputs to
            // 4 components. And since the spec also seems silent on how many components an output
            // must have, we always use 4.
            TType *fragmentVarType = new TType(accessVarType->getBasicType(), 4);
            fragmentVarType->setPrecision(plsType.getPrecision());
            fragmentVarType->setQualifier(EvqFragmentInOut);

            // PLS attachments are bound in reverse order from the rear.
            TLayoutQualifier layoutQualifier = TLayoutQualifier::Create();
            layoutQualifier.location =
                compiler->getResources().MaxCombinedDrawBuffersAndPixelLocalStoragePlanes -
                plsType.getLayoutQualifier().binding - 1;
            layoutQualifier.locationsSpecified = 1;
            if (compileOptions.pls.fragmentSyncType == ShFragmentSynchronizationType::NotSupported)
            {
                // We're using EXT_shader_framebuffer_fetch_non_coherent, which requires the
                // "noncoherent" qualifier.
                layoutQualifier.noncoherent = true;
            }
            fragmentVarType->setLayoutQualifier(layoutQualifier);

            fragmentVar = new TVariable(plsVar.uniqueId(), plsVar.name(), plsVar.symbolType(),
                                        plsVar.extensions(), fragmentVarType);
        }

        // Swizzles a variable down to the same number of components as the PLS internalformat.
        TIntermTyped *swizzle(TVariable *var) const
        {
            return Swizzle(var, accessVar->getType().getNominalSize());
        }

        TIntermTyped *swizzleFragmentVar() const { return swizzle(fragmentVar); }

        TVariable *fragmentVar;
        TVariable *accessVar;
    };

    PLSBackingStoreMap<PLSAttachment> mPLSAttachments;
};

}  // anonymous namespace

bool RewritePixelLocalStorage(TCompiler *compiler,
                              TIntermBlock *root,
                              TSymbolTable &symbolTable,
                              const ShCompileOptions &compileOptions,
                              int shaderVersion)
{
    // If any functions take PLS arguments, monomorphize the functions by removing said parameters
    // and making the PLS calls from main() instead, using the global uniform from the call site
    // instead of the function argument. This is necessary because function arguments don't carry
    // the necessary "binding" or "format" layout qualifiers.
    if (!MonomorphizeUnsupportedFunctions(
            compiler, root, &symbolTable,
            UnsupportedFunctionArgsBitSet{UnsupportedFunctionArgs::PixelLocalStorage}))
    {
        return false;
    }

    TIntermBlock *mainBody = FindMainBody(root);

    std::unique_ptr<RewritePLSTraverser> traverser;
    switch (compileOptions.pls.type)
    {
        case ShPixelLocalStorageType::ImageLoadStore:
            traverser = std::make_unique<RewritePLSToImagesTraverser>(
                compiler, symbolTable, compileOptions, shaderVersion);
            break;
        case ShPixelLocalStorageType::FramebufferFetch:
            traverser = std::make_unique<RewritePLSToFramebufferFetchTraverser>(
                compiler, symbolTable, compileOptions, shaderVersion);
            break;
        case ShPixelLocalStorageType::NotSupported:
            UNREACHABLE();
            return false;
    }

    // Rewrite PLS operations.
    root->traverse(traverser.get());
    if (!traverser->updateTree(compiler, root))
    {
        return false;
    }

    // Inject the code that needs to run before and after all PLS operations.
    // TODO(anglebug.com/40096838): Inject these functions in a tight critical section, instead of
    // just locking the entire main() function:
    //   - Monomorphize all PLS calls into main().
    //   - Insert begin/end calls around the first/last PLS calls (and outside of flow control).
    traverser->injectPrePLSCode(compiler, symbolTable, compileOptions, mainBody, 0);
    traverser->injectPostPLSCode(compiler, symbolTable, compileOptions, mainBody,
                                 mainBody->getChildCount());

    // Assign the global pixel coord at the beginning of main(), if used.
    traverser->injectPixelCoordInitializationCodeIfNeeded(compiler, root, mainBody);

    return compiler->validateAST(root);
}
}  // namespace sh
