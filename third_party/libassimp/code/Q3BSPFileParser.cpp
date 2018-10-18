/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2018, assimp team


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


#ifndef ASSIMP_BUILD_NO_Q3BSP_IMPORTER

#include "Q3BSPFileParser.h"
#include "Q3BSPFileData.h"
#include "Q3BSPZipArchive.h"
#include <vector>
#include <assimp/DefaultIOSystem.h>
#include <assimp/ai_assert.h>

namespace Assimp {

using namespace Q3BSP;

// ------------------------------------------------------------------------------------------------
Q3BSPFileParser::Q3BSPFileParser( const std::string &mapName, Q3BSPZipArchive *pZipArchive ) :
    m_sOffset( 0 ),
    m_Data(),
    m_pModel(nullptr),
    m_pZipArchive( pZipArchive )
{
    ai_assert(nullptr != m_pZipArchive );
    ai_assert( !mapName.empty() );

    if ( !readData( mapName ) )
        return;

    m_pModel = new Q3BSPModel;
    m_pModel->m_ModelName = mapName;
    if ( !parseFile() ) {
        delete m_pModel;
        m_pModel = nullptr;
    }
}

// ------------------------------------------------------------------------------------------------
Q3BSPFileParser::~Q3BSPFileParser() {
    delete m_pModel;
    m_pModel = nullptr;
}

// ------------------------------------------------------------------------------------------------
Q3BSP::Q3BSPModel *Q3BSPFileParser::getModel() const {
    return m_pModel;
}

// ------------------------------------------------------------------------------------------------
bool Q3BSPFileParser::readData( const std::string &rMapName ) {
    if ( !m_pZipArchive->Exists( rMapName.c_str() ) )
        return false;

    IOStream *pMapFile = m_pZipArchive->Open( rMapName.c_str() );
    if ( nullptr == pMapFile )
        return false;

    const size_t size = pMapFile->FileSize();
    m_Data.resize( size );

    const size_t readSize = pMapFile->Read( &m_Data[0], sizeof( char ), size );
    if ( readSize != size ) {
        m_Data.clear();
        return false;
    }
    m_pZipArchive->Close( pMapFile );

    return true;
}

// ------------------------------------------------------------------------------------------------
bool Q3BSPFileParser::parseFile() {
    if ( m_Data.empty() ) {
        return false;
    }

    if ( !validateFormat() )
    {
        return false;
    }

    // Imports the dictionary of the level
    getLumps();

    // Count data and prepare model data
    countLumps();

    // Read in Vertices
    getVertices();

    // Read in Indices
    getIndices();

    // Read Faces
    getFaces();

    // Read Textures
    getTextures();

    // Read Lightmaps
    getLightMaps();

    // Load the entities
    getEntities();

    return true;
}

// ------------------------------------------------------------------------------------------------
bool Q3BSPFileParser::validateFormat()
{
    sQ3BSPHeader *pHeader = (sQ3BSPHeader*) &m_Data[ 0 ];
    m_sOffset += sizeof( sQ3BSPHeader );

    // Version and identify string validation
    if (pHeader->strID[ 0 ] != 'I' || pHeader->strID[ 1 ] != 'B' || pHeader->strID[ 2 ] != 'S'
        || pHeader->strID[ 3 ] != 'P')
    {
        return false;
    }

    return true;
}

// ------------------------------------------------------------------------------------------------
void Q3BSPFileParser::getLumps()
{
    size_t Offset = m_sOffset;
    m_pModel->m_Lumps.resize( kMaxLumps );
    for ( size_t idx=0; idx < kMaxLumps; idx++ )
    {
        sQ3BSPLump *pLump = new sQ3BSPLump;
        memcpy( pLump, &m_Data[ Offset ], sizeof( sQ3BSPLump ) );
        Offset += sizeof( sQ3BSPLump );
        m_pModel->m_Lumps[ idx ] = pLump;
    }
}

// ------------------------------------------------------------------------------------------------
void Q3BSPFileParser::countLumps()
{
    m_pModel->m_Vertices.resize( m_pModel->m_Lumps[ kVertices ]->iSize / sizeof( sQ3BSPVertex ) );
    m_pModel->m_Indices.resize( m_pModel->m_Lumps[ kMeshVerts ]->iSize  / sizeof( int ) );
    m_pModel->m_Faces.resize( m_pModel->m_Lumps[ kFaces ]->iSize / sizeof( sQ3BSPFace ) );
    m_pModel->m_Textures.resize( m_pModel->m_Lumps[ kTextures ]->iSize / sizeof( sQ3BSPTexture ) );
    m_pModel->m_Lightmaps.resize( m_pModel->m_Lumps[ kLightmaps ]->iSize / sizeof( sQ3BSPLightmap ) );
}

// ------------------------------------------------------------------------------------------------
void Q3BSPFileParser::getVertices()
{
    size_t Offset = m_pModel->m_Lumps[ kVertices ]->iOffset;
    for ( size_t idx = 0; idx < m_pModel->m_Vertices.size(); idx++ )
    {
        sQ3BSPVertex *pVertex = new sQ3BSPVertex;
        memcpy( pVertex, &m_Data[ Offset ], sizeof( sQ3BSPVertex ) );
        Offset += sizeof( sQ3BSPVertex );
        m_pModel->m_Vertices[ idx ] = pVertex;
    }
}

// ------------------------------------------------------------------------------------------------
void Q3BSPFileParser::getIndices()
{
    ai_assert(nullptr != m_pModel );

    sQ3BSPLump *lump = m_pModel->m_Lumps[ kMeshVerts ];
    size_t Offset = (size_t) lump->iOffset;
    const size_t nIndices = lump->iSize / sizeof( int );
    m_pModel->m_Indices.resize( nIndices );
    memcpy( &m_pModel->m_Indices[ 0 ], &m_Data[ Offset ], lump->iSize );
}

// ------------------------------------------------------------------------------------------------
void Q3BSPFileParser::getFaces()
{
    ai_assert(nullptr != m_pModel );

    size_t Offset = m_pModel->m_Lumps[ kFaces ]->iOffset;
    for ( size_t idx = 0; idx < m_pModel->m_Faces.size(); idx++ )
    {
        sQ3BSPFace *pFace = new sQ3BSPFace;
        memcpy( pFace, &m_Data[ Offset ], sizeof( sQ3BSPFace ) );
        m_pModel->m_Faces[ idx ] = pFace;
        Offset += sizeof( sQ3BSPFace );
    }
}

// ------------------------------------------------------------------------------------------------
void Q3BSPFileParser::getTextures()
{
    ai_assert(nullptr != m_pModel );

    size_t Offset = m_pModel->m_Lumps[ kTextures ]->iOffset;
    for ( size_t idx=0; idx < m_pModel->m_Textures.size(); idx++ )
    {
        sQ3BSPTexture *pTexture = new sQ3BSPTexture;
        memcpy( pTexture, &m_Data[ Offset ], sizeof(sQ3BSPTexture) );
        m_pModel->m_Textures[ idx ] = pTexture;
        Offset += sizeof(sQ3BSPTexture);
    }
}

// ------------------------------------------------------------------------------------------------
void Q3BSPFileParser::getLightMaps()
{
    ai_assert(nullptr != m_pModel );

    size_t Offset = m_pModel->m_Lumps[kLightmaps]->iOffset;
    for ( size_t idx=0; idx < m_pModel->m_Lightmaps.size(); idx++ )
    {
        sQ3BSPLightmap *pLightmap = new sQ3BSPLightmap;
        memcpy( pLightmap, &m_Data[ Offset ], sizeof( sQ3BSPLightmap ) );
        Offset += sizeof( sQ3BSPLightmap );
        m_pModel->m_Lightmaps[ idx ] = pLightmap;
    }
}

// ------------------------------------------------------------------------------------------------
void Q3BSPFileParser::getEntities() {
    const int size = m_pModel->m_Lumps[ kEntities ]->iSize;
    m_pModel->m_EntityData.resize( size );
    if ( size > 0 ) {
        size_t Offset = m_pModel->m_Lumps[ kEntities ]->iOffset;
        memcpy( &m_pModel->m_EntityData[ 0 ], &m_Data[ Offset ], sizeof( char ) * size );
    }
}

// ------------------------------------------------------------------------------------------------

} // Namespace Assimp

#endif // ASSIMP_BUILD_NO_Q3BSP_IMPORTER
