//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexDeclarationCache.cpp: Implements a helper class to construct and cache vertex declarations.

#include "libANGLE/renderer/d3d/d3d9/VertexDeclarationCache.h"

#include "libANGLE/Context.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/ProgramExecutableD3D.h"
#include "libANGLE/renderer/d3d/d3d9/Context9.h"
#include "libANGLE/renderer/d3d/d3d9/VertexBuffer9.h"
#include "libANGLE/renderer/d3d/d3d9/formatutils9.h"

namespace rx
{

VertexDeclarationCache::VertexDeclarationCache() : mMaxLru(0)
{
    for (int i = 0; i < NUM_VERTEX_DECL_CACHE_ENTRIES; i++)
    {
        mVertexDeclCache[i].vertexDeclaration = nullptr;
        mVertexDeclCache[i].lruCount          = 0;
    }

    for (VBData &vb : mAppliedVBs)
    {
        vb.serial = 0;
    }

    mLastSetVDecl      = nullptr;
    mInstancingEnabled = true;
}

VertexDeclarationCache::~VertexDeclarationCache()
{
    for (int i = 0; i < NUM_VERTEX_DECL_CACHE_ENTRIES; i++)
    {
        SafeRelease(mVertexDeclCache[i].vertexDeclaration);
    }
}

angle::Result VertexDeclarationCache::applyDeclaration(
    const gl::Context *context,
    IDirect3DDevice9 *device,
    const std::vector<TranslatedAttribute> &attributes,
    gl::ProgramExecutable *executable,
    GLint start,
    GLsizei instances,
    GLsizei *repeatDraw)
{
    ASSERT(gl::MAX_VERTEX_ATTRIBS >= attributes.size());

    *repeatDraw = 1;

    const size_t invalidAttribIndex = attributes.size();
    size_t indexedAttribute         = invalidAttribIndex;
    size_t instancedAttribute       = invalidAttribIndex;

    if (instances == 0)
    {
        for (size_t i = 0; i < attributes.size(); ++i)
        {
            if (attributes[i].divisor != 0)
            {
                // If a divisor is set, it still applies even if an instanced draw was not used, so
                // treat as a single-instance draw.
                instances = 1;
                break;
            }
        }
    }

    if (instances > 0)
    {
        // Find an indexed attribute to be mapped to D3D stream 0
        for (size_t i = 0; i < attributes.size(); i++)
        {
            if (attributes[i].active)
            {
                if (indexedAttribute == invalidAttribIndex && attributes[i].divisor == 0)
                {
                    indexedAttribute = i;
                }
                else if (instancedAttribute == invalidAttribIndex && attributes[i].divisor != 0)
                {
                    instancedAttribute = i;
                }
                if (indexedAttribute != invalidAttribIndex &&
                    instancedAttribute != invalidAttribIndex)
                    break;  // Found both an indexed and instanced attribute
            }
        }

        // The validation layer checks that there is at least one active attribute with a zero
        // divisor as per the GL_ANGLE_instanced_arrays spec.
        ASSERT(indexedAttribute != invalidAttribIndex);
    }

    D3DCAPS9 caps;
    device->GetDeviceCaps(&caps);

    D3DVERTEXELEMENT9 elements[gl::MAX_VERTEX_ATTRIBS + 1];
    D3DVERTEXELEMENT9 *element = &elements[0];

    ProgramExecutableD3D *executableD3D = GetImplAs<ProgramExecutableD3D>(executable);
    const auto &semanticIndexes         = executableD3D->getAttribLocationToD3DSemantics();

    for (size_t i = 0; i < attributes.size(); i++)
    {
        if (attributes[i].active)
        {
            // Directly binding the storage buffer is not supported for d3d9
            ASSERT(attributes[i].storage == nullptr);

            int stream = static_cast<int>(i);

            if (instances > 0)
            {
                // Due to a bug on ATI cards we can't enable instancing when none of the attributes
                // are instanced.
                if (instancedAttribute == invalidAttribIndex)
                {
                    *repeatDraw = instances;
                }
                else
                {
                    if (i == indexedAttribute)
                    {
                        stream = 0;
                    }
                    else if (i == 0)
                    {
                        stream = static_cast<int>(indexedAttribute);
                    }

                    UINT frequency = 1;

                    if (attributes[i].divisor == 0)
                    {
                        frequency = D3DSTREAMSOURCE_INDEXEDDATA | instances;
                    }
                    else
                    {
                        frequency = D3DSTREAMSOURCE_INSTANCEDATA | attributes[i].divisor;
                    }

                    device->SetStreamSourceFreq(stream, frequency);
                    mInstancingEnabled = true;
                }
            }

            VertexBuffer9 *vertexBuffer = GetAs<VertexBuffer9>(attributes[i].vertexBuffer.get());

            unsigned int offset = 0;
            ANGLE_TRY(attributes[i].computeOffset(context, start, &offset));

            if (mAppliedVBs[stream].serial != attributes[i].serial ||
                mAppliedVBs[stream].stride != attributes[i].stride ||
                mAppliedVBs[stream].offset != offset)
            {
                device->SetStreamSource(stream, vertexBuffer->getBuffer(), offset,
                                        attributes[i].stride);
                mAppliedVBs[stream].serial = attributes[i].serial;
                mAppliedVBs[stream].stride = attributes[i].stride;
                mAppliedVBs[stream].offset = offset;
            }

            angle::FormatID vertexformatID =
                gl::GetVertexFormatID(*attributes[i].attribute, gl::VertexAttribType::Float);
            const d3d9::VertexFormat &d3d9VertexInfo =
                d3d9::GetVertexFormatInfo(caps.DeclTypes, vertexformatID);

            element->Stream     = static_cast<WORD>(stream);
            element->Offset     = 0;
            element->Type       = static_cast<BYTE>(d3d9VertexInfo.nativeFormat);
            element->Method     = D3DDECLMETHOD_DEFAULT;
            element->Usage      = D3DDECLUSAGE_TEXCOORD;
            element->UsageIndex = static_cast<BYTE>(semanticIndexes[i]);
            element++;
        }
    }

    if (instances == 0 || instancedAttribute == invalidAttribIndex)
    {
        if (mInstancingEnabled)
        {
            for (int i = 0; i < gl::MAX_VERTEX_ATTRIBS; i++)
            {
                device->SetStreamSourceFreq(i, 1);
            }

            mInstancingEnabled = false;
        }
    }

    static const D3DVERTEXELEMENT9 end = D3DDECL_END();
    *(element++)                       = end;

    for (int i = 0; i < NUM_VERTEX_DECL_CACHE_ENTRIES; i++)
    {
        VertexDeclCacheEntry *entry = &mVertexDeclCache[i];
        if (memcmp(entry->cachedElements, elements,
                   (element - elements) * sizeof(D3DVERTEXELEMENT9)) == 0 &&
            entry->vertexDeclaration)
        {
            entry->lruCount = ++mMaxLru;
            if (entry->vertexDeclaration != mLastSetVDecl)
            {
                device->SetVertexDeclaration(entry->vertexDeclaration);
                mLastSetVDecl = entry->vertexDeclaration;
            }

            return angle::Result::Continue;
        }
    }

    VertexDeclCacheEntry *lastCache = mVertexDeclCache;

    for (int i = 0; i < NUM_VERTEX_DECL_CACHE_ENTRIES; i++)
    {
        if (mVertexDeclCache[i].lruCount < lastCache->lruCount)
        {
            lastCache = &mVertexDeclCache[i];
        }
    }

    if (lastCache->vertexDeclaration != nullptr)
    {
        SafeRelease(lastCache->vertexDeclaration);
        // mLastSetVDecl is set to the replacement, so we don't have to worry
        // about it.
    }

    memcpy(lastCache->cachedElements, elements, (element - elements) * sizeof(D3DVERTEXELEMENT9));
    HRESULT result = device->CreateVertexDeclaration(elements, &lastCache->vertexDeclaration);
    ANGLE_TRY_HR(GetImplAs<Context9>(context), result,
                 "Failed to create internal vertex declaration");

    device->SetVertexDeclaration(lastCache->vertexDeclaration);
    mLastSetVDecl       = lastCache->vertexDeclaration;
    lastCache->lruCount = ++mMaxLru;

    return angle::Result::Continue;
}

void VertexDeclarationCache::markStateDirty()
{
    for (VBData &vb : mAppliedVBs)
    {
        vb.serial = 0;
    }

    mLastSetVDecl      = nullptr;
    mInstancingEnabled = true;  // Forces it to be disabled when not used
}

}  // namespace rx
