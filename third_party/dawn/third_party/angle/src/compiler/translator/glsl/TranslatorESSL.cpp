//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/glsl/TranslatorESSL.h"

#include "angle_gl.h"
#include "common/utilities.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/glsl/BuiltInFunctionEmulatorGLSL.h"
#include "compiler/translator/glsl/OutputESSL.h"
#include "compiler/translator/tree_ops/DeclarePerVertexBlocks.h"
#include "compiler/translator/tree_ops/RecordConstantPrecision.h"
#include "compiler/translator/tree_util/FindSymbolNode.h"
#include "compiler/translator/tree_util/ReplaceClipCullDistanceVariable.h"
#include "compiler/translator/tree_util/RunAtTheEndOfShader.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

bool EmulateClipOrigin(TCompiler *compiler, TIntermBlock *root, TSymbolTable *symbolTable)
{
    // Skip the operation if gl_Position is not used.
    const TIntermSymbol *positionSymbol = FindSymbolNode(root, ImmutableString("gl_Position"));
    if (!positionSymbol)
    {
        return true;
    }

    const TType *type           = StaticType::Get<EbtFloat, EbpHigh, EvqUniform, 1, 1>();
    const TVariable *clipOrigin = new TVariable(symbolTable, ImmutableString("angle_ClipOrigin"),
                                                type, SymbolType::AngleInternal);

    DeclareGlobalVariable(root, clipOrigin);

    // gl_Position.y *= angle_clipOrigin;
    TIntermSwizzle *positionY =
        new TIntermSwizzle(new TIntermSymbol(&positionSymbol->variable()), {1});
    TIntermBinary *applyOrigin =
        new TIntermBinary(EOpMulAssign, positionY, new TIntermSymbol(clipOrigin));

    return RunAtTheEndOfShader(compiler, root, applyOrigin, symbolTable);
}

}  // namespace

TranslatorESSL::TranslatorESSL(sh::GLenum type, ShShaderSpec spec)
    : TCompiler(type, spec, SH_ESSL_OUTPUT)
{}

void TranslatorESSL::initBuiltInFunctionEmulator(BuiltInFunctionEmulator *emu,
                                                 const ShCompileOptions &compileOptions)
{
    if (compileOptions.emulateAtan2FloatFunction)
    {
        InitBuiltInAtanFunctionEmulatorForGLSLWorkarounds(emu);
    }
}

bool TranslatorESSL::translate(TIntermBlock *root,
                               const ShCompileOptions &compileOptions,
                               PerformanceDiagnostics * /*perfDiagnostics*/)
{
    TInfoSinkBase &sink = getInfoSink().obj;

    int shaderVer = getShaderVersion();  // Frontend shader version.
    if (shaderVer == 300)
    {
        // Although ANGLE supports all these extensions with ESSL 3.00,
        // some drivers may support the required functionality only
        // with ESSL 3.10.
        const bool hasExtensionsThatMayRequireES31 =
            getResources().EXT_clip_cull_distance || getResources().ANGLE_clip_cull_distance ||
            getResources().NV_shader_noperspective_interpolation ||
            getResources().OES_shader_multisample_interpolation ||
            getResources().ANGLE_texture_multisample ||
            getResources().OES_texture_storage_multisample_2d_array;

        // When PLS is implemented with shader images,
        // ESSL 3.10 output is required.
        const bool usesShaderImagesForPLS =
            hasPixelLocalStorageUniforms() &&
            compileOptions.pls.type == ShPixelLocalStorageType::ImageLoadStore;

        if (hasExtensionsThatMayRequireES31 || usesShaderImagesForPLS)
        {
            shaderVer = 310;
        }
    }
    if (shaderVer > 100)
    {
        sink << "#version " << shaderVer << " es\n";
    }

    // Write built-in extension behaviors.
    writeExtensionBehavior(compileOptions);

    // Write pragmas after extensions because some drivers consider pragmas
    // like non-preprocessor tokens.
    WritePragma(sink, compileOptions, getPragma());

    if (!RecordConstantPrecision(this, root, &getSymbolTable()))
    {
        return false;
    }

    // Write emulated built-in functions if needed.
    if (!getBuiltInFunctionEmulator().isOutputEmpty())
    {
        sink << "// BEGIN: Generated code for built-in function emulation\n\n";
        if (getShaderType() == GL_FRAGMENT_SHADER)
        {
            sink << "#if defined(GL_FRAGMENT_PRECISION_HIGH)\n"
                 << "#define emu_precision highp\n"
                 << "#else\n"
                 << "#define emu_precision mediump\n"
                 << "#endif\n\n";
        }
        else
        {
            sink << "#define emu_precision highp\n";
        }

        getBuiltInFunctionEmulator().outputEmulatedFunctions(sink);
        sink << "// END: Generated code for built-in function emulation\n\n";
    }

    if (getShaderType() == GL_VERTEX_SHADER)
    {
        // Emulate GL_CLIP_DISTANCEi_EXT state if needed
        if (mMetadataFlags[MetadataFlags::HasClipDistance] &&
            compileOptions.emulateClipDistanceState)
        {
            constexpr const ImmutableString kClipDistanceEnabledName("angle_ClipDistanceEnabled");

            const TType *type = StaticType::Get<EbtUInt, EbpLow, EvqUniform, 1, 1>();
            const TVariable *clipDistanceEnabled = new TVariable(
                &getSymbolTable(), kClipDistanceEnabledName, type, SymbolType::AngleInternal);
            const TIntermSymbol *clipDistanceEnabledSymbol = new TIntermSymbol(clipDistanceEnabled);

            // AngleInternal variables don't get collected
            ShaderVariable uniform;
            uniform.name          = kClipDistanceEnabledName.data();
            uniform.mappedName    = kClipDistanceEnabledName.data();
            uniform.type          = GLVariableType(*type);
            uniform.precision     = GLVariablePrecision(*type);
            uniform.staticUse     = true;
            uniform.active        = true;
            uniform.binding       = type->getLayoutQualifier().binding;
            uniform.location      = type->getLayoutQualifier().location;
            uniform.offset        = type->getLayoutQualifier().offset;
            uniform.rasterOrdered = type->getLayoutQualifier().rasterOrdered;
            uniform.readonly      = type->getMemoryQualifier().readonly;
            uniform.writeonly     = type->getMemoryQualifier().writeonly;
            mUniforms.push_back(uniform);

            DeclareGlobalVariable(root, clipDistanceEnabled);
            if (!ZeroDisabledClipDistanceAssignments(this, root, &getSymbolTable(), getShaderType(),
                                                     clipDistanceEnabledSymbol))
                return false;

            // The previous operation always redeclares gl_ClipDistance
            if (!DeclarePerVertexBlocks(this, root, &getSymbolTable(), nullptr, nullptr))
                return false;
        }
        else if (areClipDistanceOrCullDistanceUsed() &&
                 (IsExtensionEnabled(getExtensionBehavior(), TExtension::EXT_clip_cull_distance) ||
                  IsExtensionEnabled(getExtensionBehavior(), TExtension::ANGLE_clip_cull_distance)))
        {
            // When clip distance state emulation is not needed,
            // the redeclared extension built-ins still should be moved to gl_PerVertex
            if (!DeclarePerVertexBlocks(this, root, &getSymbolTable(), nullptr, nullptr))
                return false;
        }

        if (compileOptions.emulateClipOrigin)
        {
            if (!EmulateClipOrigin(this, root, &getSymbolTable()))
            {
                return false;
            }
        }
    }

    if (getShaderType() == GL_FRAGMENT_SHADER)
    {
        EmitEarlyFragmentTestsGLSL(*this, sink);
        WriteFragmentShaderLayoutQualifiers(sink, getAdvancedBlendEquations());
    }

    if (getShaderType() == GL_COMPUTE_SHADER)
    {
        EmitWorkGroupSizeGLSL(*this, sink);
    }

    if (getShaderType() == GL_GEOMETRY_SHADER_EXT)
    {
        WriteGeometryShaderLayoutQualifiers(
            sink, getGeometryShaderInputPrimitiveType(), getGeometryShaderInvocations(),
            getGeometryShaderOutputPrimitiveType(), getGeometryShaderMaxVertices());
    }

    // Write translated shader.
    TOutputESSL outputESSL(this, sink, compileOptions);

    root->traverse(&outputESSL);

    return true;
}

bool TranslatorESSL::shouldFlattenPragmaStdglInvariantAll()
{
    // If following the spec to the letter, we should not flatten this pragma.
    // However, the spec's wording means that the pragma applies only to outputs.
    // This contradicts the spirit of using the pragma,
    // because if the pragma is used in a vertex shader,
    // the only way to be able to link it to a fragment shader
    // is to manually qualify each of fragment shader's inputs as invariant.
    // Which defeats the purpose of this pragma - temporarily make all varyings
    // invariant for debugging.
    // Thus, we should be non-conformant to spec's letter here and flatten.
    return true;
}

void TranslatorESSL::writeExtensionBehavior(const ShCompileOptions &compileOptions)
{
    TInfoSinkBase &sink                   = getInfoSink().obj;
    const TExtensionBehavior &extBehavior = getExtensionBehavior();
    for (TExtensionBehavior::const_iterator iter = extBehavior.begin(); iter != extBehavior.end();
         ++iter)
    {
        if (iter->second != EBhUndefined)
        {
            const bool isMultiview = (iter->first == TExtension::OVR_multiview) ||
                                     (iter->first == TExtension::OVR_multiview2);
            if (getResources().NV_shader_framebuffer_fetch &&
                iter->first == TExtension::EXT_shader_framebuffer_fetch)
            {
                sink << "#extension GL_NV_shader_framebuffer_fetch : "
                     << GetBehaviorString(iter->second) << "\n";
            }
            else if (getResources().NV_draw_buffers && iter->first == TExtension::EXT_draw_buffers)
            {
                sink << "#extension GL_NV_draw_buffers : " << GetBehaviorString(iter->second)
                     << "\n";
            }
            else if (isMultiview)
            {
                // Only either OVR_multiview OR OVR_multiview2 should be emitted.
                if ((iter->first != TExtension::OVR_multiview) ||
                    !IsExtensionEnabled(extBehavior, TExtension::OVR_multiview2))
                {
                    EmitMultiviewGLSL(*this, compileOptions, iter->first, iter->second, sink);
                }
            }
            else if (iter->first == TExtension::EXT_geometry_shader ||
                     iter->first == TExtension::OES_geometry_shader)
            {
                sink << "#ifdef GL_EXT_geometry_shader\n"
                     << "#extension GL_EXT_geometry_shader : " << GetBehaviorString(iter->second)
                     << "\n"
                     << "#elif defined GL_OES_geometry_shader\n"
                     << "#extension GL_OES_geometry_shader : " << GetBehaviorString(iter->second)
                     << "\n";
                if (iter->second == EBhRequire)
                {
                    sink << "#else\n"
                         << "#error \"No geometry shader extensions available.\" // Only generate "
                            "this if the extension is \"required\"\n";
                }
                sink << "#endif\n";
            }
            else if (iter->first == TExtension::ANGLE_multi_draw)
            {
                // Don't emit anything. This extension is emulated
                ASSERT(compileOptions.emulateGLDrawID);
                continue;
            }
            else if (iter->first == TExtension::ANGLE_base_vertex_base_instance_shader_builtin)
            {
                // Don't emit anything. This extension is emulated
                ASSERT(compileOptions.emulateGLBaseVertexBaseInstance);
                continue;
            }
            else if (iter->first == TExtension::EXT_clip_cull_distance ||
                     iter->first == TExtension::ANGLE_clip_cull_distance)
            {
                sink << "#extension GL_EXT_clip_cull_distance : " << GetBehaviorString(iter->second)
                     << "\n";
                if (areClipDistanceOrCullDistanceUsed())
                {
                    sink << "#extension GL_EXT_shader_io_blocks : "
                         << GetBehaviorString(iter->second) << "\n";
                }
            }
            else if (iter->first == TExtension::ANGLE_shader_pixel_local_storage)
            {
                if (compileOptions.pls.type == ShPixelLocalStorageType::FramebufferFetch)
                {
                    // Just enable the extension. Appropriate warnings will be generated by the
                    // frontend compiler for GL_ANGLE_shader_pixel_local_storage, if desired.
                    sink << "#extension GL_EXT_shader_framebuffer_fetch : enable\n";
                }
                continue;
            }
            else if (iter->first == TExtension::EXT_shader_framebuffer_fetch)
            {
                sink << "#extension GL_EXT_shader_framebuffer_fetch : "
                     << GetBehaviorString(iter->second) << "\n";
                continue;
            }
            else if (iter->first == TExtension::EXT_shader_framebuffer_fetch_non_coherent)
            {
                sink << "#extension GL_EXT_shader_framebuffer_fetch_non_coherent : "
                     << GetBehaviorString(iter->second) << "\n";
                continue;
            }
            else if (iter->first == TExtension::ANGLE_texture_multisample)
            {
                // Don't emit anything. This functionality is core in ESSL 3.10.
                continue;
            }
            else if (iter->first == TExtension::WEBGL_video_texture)
            {
                // Don't emit anything. This extension is emulated
                // TODO(crbug.com/776222): support external image.
                continue;
            }
            else
            {
                sink << "#extension " << GetExtensionNameString(iter->first) << " : "
                     << GetBehaviorString(iter->second) << "\n";
            }
        }
    }
}

}  // namespace sh
