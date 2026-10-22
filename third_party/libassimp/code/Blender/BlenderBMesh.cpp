/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2013, assimp team
All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the
following conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----------------------------------------------------------------------
*/

/** @file  BlenderBMesh.cpp
 *  @brief Conversion of Blender's new BMesh stuff
 */


#ifndef ASSIMP_BUILD_NO_BLEND_IMPORTER

#include "BlenderDNA.h"
#include "BlenderScene.h"
#include "BlenderBMesh.h"
#include "BlenderTessellator.h"

namespace Assimp
{
    template< > const char* LogFunctions< BlenderBMeshConverter >::Prefix()
    {
        static auto prefix = "BLEND_BMESH: ";
        return prefix;
    }
}

using namespace Assimp;
using namespace Assimp::Blender;
using namespace Assimp::Formatter;

// ------------------------------------------------------------------------------------------------
BlenderBMeshConverter::BlenderBMeshConverter( const Mesh* mesh ):
    BMesh( mesh ),
    triMesh( NULL )
{
}

// ------------------------------------------------------------------------------------------------
BlenderBMeshConverter::~BlenderBMeshConverter( )
{
    DestroyTriMesh( );
}

// ------------------------------------------------------------------------------------------------
bool BlenderBMeshConverter::ContainsBMesh( ) const
{
    // TODO - Should probably do some additional verification here
    return BMesh->totpoly && BMesh->totloop && BMesh->totvert;
}

// ------------------------------------------------------------------------------------------------
const Mesh* BlenderBMeshConverter::TriangulateBMesh( )
{
    AssertValidMesh( );
    AssertValidSizes( );
    PrepareTriMesh( );

    for ( int i = 0; i < BMesh->totpoly; ++i )
    {
        const MPoly& poly = BMesh->mpoly[ i ];
        ConvertPolyToFaces( poly );
    }

    return triMesh;
}

// ------------------------------------------------------------------------------------------------
void BlenderBMeshConverter::AssertValidMesh( )
{
    if ( !ContainsBMesh( ) )
    {
        ThrowException( "BlenderBMeshConverter requires a BMesh with \"polygons\" - please call BlenderBMeshConverter::ContainsBMesh to check this first" );
    }
}

// ------------------------------------------------------------------------------------------------
void BlenderBMeshConverter::AssertValidSizes( )
{
    if ( BMesh->totpoly != static_cast<int>( BMesh->mpoly.size( ) ) )
    {
        ThrowException( "BMesh poly array has incorrect size" );
    }
    if ( BMesh->totloop != static_cast<int>( BMesh->mloop.size( ) ) )
    {
        ThrowException( "BMesh loop array has incorrect size" );
    }
}

// ------------------------------------------------------------------------------------------------
void BlenderBMeshConverter::PrepareTriMesh( )
{
    if ( triMesh )
    {
        DestroyTriMesh( );
    }

    triMesh = new Mesh( *BMesh );
    triMesh->totface = 0;
    triMesh->mface.clear( );
}

// ------------------------------------------------------------------------------------------------
void BlenderBMeshConverter::DestroyTriMesh( )
{
    delete triMesh;
    triMesh = NULL;
}

// ------------------------------------------------------------------------------------------------
void BlenderBMeshConverter::ConvertPolyToFaces( const MPoly& poly )
{
    const MLoop* polyLoop = &BMesh->mloop[ poly.loopstart ];

    if ( poly.totloop == 3 || poly.totloop == 4 )
    {
        AddFace( polyLoop[ 0 ].v, polyLoop[ 1 ].v, polyLoop[ 2 ].v, poly.totloop == 4 ? polyLoop[ 3 ].v : 0 );

        // UVs are optional, so only convert when present.
        if ( BMesh->mloopuv.size() )
        {
            if ( (poly.loopstart + poly.totloop ) > static_cast<int>( BMesh->mloopuv.size() ) )
            {
                ThrowException( "BMesh uv loop array has incorrect size" );
            }
            const MLoopUV* loopUV = &BMesh->mloopuv[ poly.loopstart ];
            AddTFace( loopUV[ 0 ].uv, loopUV[ 1 ].uv, loopUV[ 2 ].uv, poly.totloop == 4 ? loopUV[ 3 ].uv : 0 );
        }
    }
    else if ( poly.totloop > 4 )
    {
#if ASSIMP_BLEND_WITH_GLU_TESSELLATE
        BlenderTessellatorGL tessGL( *this );
        tessGL.Tessellate( polyLoop, poly.totloop, triMesh->mvert );
#elif ASSIMP_BLEND_WITH_POLY_2_TRI
        BlenderTessellatorP2T tessP2T( *this );
        tessP2T.Tessellate( polyLoop, poly.totloop, triMesh->mvert );
#endif
    }
}

// ------------------------------------------------------------------------------------------------
void BlenderBMeshConverter::AddFace( int v1, int v2, int v3, int v4 )
{
    MFace face;
    face.v1 = v1;
    face.v2 = v2;
    face.v3 = v3;
    face.v4 = v4;
    // TODO - Work out how materials work
    face.mat_nr = 0;
    triMesh->mface.push_back( face );
    triMesh->totface = static_cast<int>(triMesh->mface.size( ));
}

// ------------------------------------------------------------------------------------------------
void BlenderBMeshConverter::AddTFace( const float* uv1, const float *uv2, const float *uv3, const float* uv4 )
{
    MTFace mtface;
    memcpy( &mtface.uv[ 0 ], uv1, sizeof(float) * 2 );
    memcpy( &mtface.uv[ 1 ], uv2, sizeof(float) * 2 );
    memcpy( &mtface.uv[ 2 ], uv3, sizeof(float) * 2 );

    if ( uv4 )
    {
        memcpy( &mtface.uv[ 3 ], uv4, sizeof(float) * 2 );
    }

    triMesh->mtface.push_back( mtface );
}

#endif // ASSIMP_BUILD_NO_BLEND_IMPORTER
