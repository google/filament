/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team



All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the following
conditions are met:

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
---------------------------------------------------------------------------
*/
#ifndef ASSIMP_BUILD_NO_OBJ_IMPORTER

#include "ObjFileParser.h"
#include "ObjFileMtlImporter.h"
#include "ObjTools.h"
#include "ObjFileData.h"
#include <assimp/ParsingUtils.h>
#include <assimp/BaseImporter.h>
#include <assimp/DefaultIOSystem.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/material.h>
#include <assimp/Importer.hpp>
#include <cstdlib>

namespace Assimp {

const std::string ObjFileParser::DEFAULT_MATERIAL = AI_DEFAULT_MATERIAL_NAME;

ObjFileParser::ObjFileParser()
: m_DataIt()
, m_DataItEnd()
, m_pModel( nullptr )
, m_uiLine( 0 )
, m_pIO( nullptr )
, m_progress( nullptr )
, m_originalObjFileName() {
    // empty
}

ObjFileParser::ObjFileParser( IOStreamBuffer<char> &streamBuffer, const std::string &modelName,
                              IOSystem *io, ProgressHandler* progress,
                              const std::string &originalObjFileName) :
    m_DataIt(),
    m_DataItEnd(),
    m_pModel(nullptr),
    m_uiLine(0),
    m_pIO( io ),
    m_progress(progress),
    m_originalObjFileName(originalObjFileName)
{
    std::fill_n(m_buffer,Buffersize,0);

    // Create the model instance to store all the data
    m_pModel.reset(new ObjFile::Model());
    m_pModel->m_ModelName = modelName;

    // create default material and store it
    m_pModel->m_pDefaultMaterial = new ObjFile::Material;
    m_pModel->m_pDefaultMaterial->MaterialName.Set( DEFAULT_MATERIAL );
    m_pModel->m_MaterialLib.push_back( DEFAULT_MATERIAL );
    m_pModel->m_MaterialMap[ DEFAULT_MATERIAL ] = m_pModel->m_pDefaultMaterial;

    // Start parsing the file
    parseFile( streamBuffer );
}

ObjFileParser::~ObjFileParser() {
}

void ObjFileParser::setBuffer( std::vector<char> &buffer ) {
    m_DataIt = buffer.begin();
    m_DataItEnd = buffer.end();
}

ObjFile::Model *ObjFileParser::GetModel() const {
    return m_pModel.get();
}

void ObjFileParser::parseFile( IOStreamBuffer<char> &streamBuffer ) {
    // only update every 100KB or it'll be too slow
    //const unsigned int updateProgressEveryBytes = 100 * 1024;
    unsigned int progressCounter = 0;
    const unsigned int bytesToProcess = static_cast<unsigned int>(streamBuffer.size());
    const unsigned int progressTotal = bytesToProcess;
    unsigned int processed = 0;
    size_t lastFilePos( 0 );

    std::vector<char> buffer;
    while ( streamBuffer.getNextDataLine( buffer, '\\' ) ) {
        m_DataIt = buffer.begin();
        m_DataItEnd = buffer.end();

        // Handle progress reporting
        const size_t filePos( streamBuffer.getFilePos() );
        if ( lastFilePos < filePos ) {
            processed = static_cast<unsigned int>(filePos);
            lastFilePos = filePos;
            progressCounter++;
            m_progress->UpdateFileRead( processed, progressTotal );
        }

        // parse line
        switch (*m_DataIt) {
        case 'v': // Parse a vertex texture coordinate
            {
                ++m_DataIt;
                if (*m_DataIt == ' ' || *m_DataIt == '\t') {
                    size_t numComponents = getNumComponentsInDataDefinition();
                    if (numComponents == 3) {
                        // read in vertex definition
                        getVector3(m_pModel->m_Vertices);
                    } else if (numComponents == 4) {
                        // read in vertex definition (homogeneous coords)
                        getHomogeneousVector3(m_pModel->m_Vertices);
                    } else if (numComponents == 6) {
                        // read vertex and vertex-color
                        getTwoVectors3(m_pModel->m_Vertices, m_pModel->m_VertexColors);
                    }
                } else if (*m_DataIt == 't') {
                    // read in texture coordinate ( 2D or 3D )
                    ++m_DataIt;
                    size_t dim = getTexCoordVector(m_pModel->m_TextureCoord);
                    m_pModel->m_TextureCoordDim = std::max(m_pModel->m_TextureCoordDim, (unsigned int)dim);
                } else if (*m_DataIt == 'n') {
                    // Read in normal vector definition
                    ++m_DataIt;
                    getVector3( m_pModel->m_Normals );
                }
            }
            break;

        case 'p': // Parse a face, line or point statement
        case 'l':
        case 'f':
            {
                getFace(*m_DataIt == 'f' ? aiPrimitiveType_POLYGON : (*m_DataIt == 'l'
                    ? aiPrimitiveType_LINE : aiPrimitiveType_POINT));
            }
            break;

        case '#': // Parse a comment
            {
                getComment();
            }
            break;

        case 'u': // Parse a material desc. setter
            {
                std::string name;

                getNameNoSpace(m_DataIt, m_DataItEnd, name);

                size_t nextSpace = name.find(" ");
                if (nextSpace != std::string::npos)
                    name = name.substr(0, nextSpace);

                if(name == "usemtl")
                {
                    getMaterialDesc();
                }
            }
            break;

        case 'm': // Parse a material library or merging group ('mg')
            {
                std::string name;

                getNameNoSpace(m_DataIt, m_DataItEnd, name);

                size_t nextSpace = name.find(" ");
                if (nextSpace != std::string::npos)
                    name = name.substr(0, nextSpace);

                if (name == "mg")
                    getGroupNumberAndResolution();
                else if(name == "mtllib")
                    getMaterialLib();
				else
					goto pf_skip_line;
            }
            break;

        case 'g': // Parse group name
            {
                getGroupName();
            }
            break;

        case 's': // Parse group number
            {
                getGroupNumber();
            }
            break;

        case 'o': // Parse object name
            {
                getObjectName();
            }
            break;

        default:
            {
pf_skip_line:
                m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
            }
            break;
        }
    }
}

void ObjFileParser::copyNextWord(char *pBuffer, size_t length) {
    size_t index = 0;
    m_DataIt = getNextWord<DataArrayIt>(m_DataIt, m_DataItEnd);
    if ( *m_DataIt == '\\' ) {
        ++m_DataIt;
        ++m_DataIt;
        m_DataIt = getNextWord<DataArrayIt>( m_DataIt, m_DataItEnd );
    }
    while( m_DataIt != m_DataItEnd && !IsSpaceOrNewLine( *m_DataIt ) ) {
        pBuffer[index] = *m_DataIt;
        index++;
        if( index == length - 1 ) {
            break;
        }
        ++m_DataIt;
    }

    ai_assert(index < length);
    pBuffer[index] = '\0';
}

static bool isDataDefinitionEnd( const char *tmp ) {
    if ( *tmp == '\\' ) {
        tmp++;
        if ( IsLineEnd( *tmp ) ) {
            tmp++;
            return true;
        }
    }
    return false;
}

static bool isNanOrInf(const char * in) {
    // Look for "nan" or "inf", case insensitive
    if ((in[0] == 'N' || in[0] == 'n') && ASSIMP_strincmp(in, "nan", 3) == 0) {
        return true;
    }
    else if ((in[0] == 'I' || in[0] == 'i') && ASSIMP_strincmp(in, "inf", 3) == 0) {
        return true;
    }
    return false;
}

size_t ObjFileParser::getNumComponentsInDataDefinition() {
    size_t numComponents( 0 );
    const char* tmp( &m_DataIt[0] );
    bool end_of_definition = false;
    while ( !end_of_definition ) {
        if ( isDataDefinitionEnd( tmp ) ) {
            tmp += 2;
        } else if ( IsLineEnd( *tmp ) ) {
            end_of_definition = true;
        }
        if ( !SkipSpaces( &tmp ) ) {
            break;
        }
        const bool isNum( IsNumeric( *tmp ) || isNanOrInf(tmp));
        SkipToken( tmp );
        if ( isNum ) {
            ++numComponents;
        }
        if ( !SkipSpaces( &tmp ) ) {
            break;
        }
    }
    return numComponents;
}

size_t ObjFileParser::getTexCoordVector( std::vector<aiVector3D> &point3d_array ) {
    size_t numComponents = getNumComponentsInDataDefinition();
    ai_real x, y, z;
    if( 2 == numComponents ) {
        copyNextWord( m_buffer, Buffersize );
        x = ( ai_real ) fast_atof( m_buffer );

        copyNextWord( m_buffer, Buffersize );
        y = ( ai_real ) fast_atof( m_buffer );
        z = 0.0;
    } else if( 3 == numComponents ) {
        copyNextWord( m_buffer, Buffersize );
        x = ( ai_real ) fast_atof( m_buffer );

        copyNextWord( m_buffer, Buffersize );
        y = ( ai_real ) fast_atof( m_buffer );

        copyNextWord( m_buffer, Buffersize );
        z = ( ai_real ) fast_atof( m_buffer );
    } else {
        throw DeadlyImportError( "OBJ: Invalid number of components" );
    }

    // Coerce nan and inf to 0 as is the OBJ default value
    if (!std::isfinite(x))
        x = 0;

    if (!std::isfinite(y))
        y = 0;

    if (!std::isfinite(z))
        z = 0;

    point3d_array.push_back( aiVector3D( x, y, z ) );
    m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
    return numComponents;
}

void ObjFileParser::getVector3( std::vector<aiVector3D> &point3d_array ) {
    ai_real x, y, z;
    copyNextWord(m_buffer, Buffersize);
    x = (ai_real) fast_atof(m_buffer);

    copyNextWord(m_buffer, Buffersize);
    y = (ai_real) fast_atof(m_buffer);

    copyNextWord( m_buffer, Buffersize );
    z = ( ai_real ) fast_atof( m_buffer );

    point3d_array.push_back( aiVector3D( x, y, z ) );
    m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
}

void ObjFileParser::getHomogeneousVector3( std::vector<aiVector3D> &point3d_array ) {
    ai_real x, y, z, w;
    copyNextWord(m_buffer, Buffersize);
    x = (ai_real) fast_atof(m_buffer);

    copyNextWord(m_buffer, Buffersize);
    y = (ai_real) fast_atof(m_buffer);

    copyNextWord( m_buffer, Buffersize );
    z = ( ai_real ) fast_atof( m_buffer );

    copyNextWord( m_buffer, Buffersize );
    w = ( ai_real ) fast_atof( m_buffer );

    if (w == 0)
      throw DeadlyImportError("OBJ: Invalid component in homogeneous vector (Division by zero)");

    point3d_array.push_back( aiVector3D( x/w, y/w, z/w ) );
    m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
}

void ObjFileParser::getTwoVectors3( std::vector<aiVector3D> &point3d_array_a, std::vector<aiVector3D> &point3d_array_b ) {
    ai_real x, y, z;
    copyNextWord(m_buffer, Buffersize);
    x = (ai_real) fast_atof(m_buffer);

    copyNextWord(m_buffer, Buffersize);
    y = (ai_real) fast_atof(m_buffer);

    copyNextWord( m_buffer, Buffersize );
    z = ( ai_real ) fast_atof( m_buffer );

    point3d_array_a.push_back( aiVector3D( x, y, z ) );

    copyNextWord(m_buffer, Buffersize);
    x = (ai_real) fast_atof(m_buffer);

    copyNextWord(m_buffer, Buffersize);
    y = (ai_real) fast_atof(m_buffer);

    copyNextWord( m_buffer, Buffersize );
    z = ( ai_real ) fast_atof( m_buffer );

    point3d_array_b.push_back( aiVector3D( x, y, z ) );

    m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
}

void ObjFileParser::getVector2( std::vector<aiVector2D> &point2d_array ) {
    ai_real x, y;
    copyNextWord(m_buffer, Buffersize);
    x = (ai_real) fast_atof(m_buffer);

    copyNextWord(m_buffer, Buffersize);
    y = (ai_real) fast_atof(m_buffer);

    point2d_array.push_back(aiVector2D(x, y));

    m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
}

static const std::string DefaultObjName = "defaultobject";

void ObjFileParser::getFace( aiPrimitiveType type ) {
    m_DataIt = getNextToken<DataArrayIt>( m_DataIt, m_DataItEnd );
    if ( m_DataIt == m_DataItEnd || *m_DataIt == '\0' ) {
        return;
    }

    ObjFile::Face *face = new ObjFile::Face( type );
    bool hasNormal = false;

    const int vSize = static_cast<unsigned int>(m_pModel->m_Vertices.size());
    const int vtSize = static_cast<unsigned int>(m_pModel->m_TextureCoord.size());
    const int vnSize = static_cast<unsigned int>(m_pModel->m_Normals.size());

    const bool vt = (!m_pModel->m_TextureCoord.empty());
    const bool vn = (!m_pModel->m_Normals.empty());
    int iStep = 0, iPos = 0;
    while ( m_DataIt != m_DataItEnd ) {
        iStep = 1;

        if ( IsLineEnd( *m_DataIt ) ) {
            break;
        }

        if ( *m_DataIt =='/' ) {
            if (type == aiPrimitiveType_POINT) {
                ASSIMP_LOG_ERROR("Obj: Separator unexpected in point statement");
            }
            iPos++;
        } else if( IsSpaceOrNewLine( *m_DataIt ) ) {
            iPos = 0;
        } else {
            //OBJ USES 1 Base ARRAYS!!!!
            const int iVal( ::atoi( & ( *m_DataIt ) ) );

            // increment iStep position based off of the sign and # of digits
            int tmp = iVal;
            if ( iVal < 0 ) {
                ++iStep;
            }
            while ( ( tmp = tmp / 10 ) != 0 ) {
                ++iStep;
            }

            if (iPos == 1 && !vt && vn)
                iPos = 2; // skip texture coords for normals if there are no tex coords

            if ( iVal > 0 ) {
                // Store parsed index
                if ( 0 == iPos ) {
                    face->m_vertices.push_back( iVal - 1 );
                } else if ( 1 == iPos ) {
                    face->m_texturCoords.push_back( iVal - 1 );
                } else if ( 2 == iPos ) {
                    face->m_normals.push_back( iVal - 1 );
                    hasNormal = true;
                } else {
                    reportErrorTokenInFace();
                }
            } else if ( iVal < 0 ) {
                // Store relatively index
                if ( 0 == iPos ) {
                    face->m_vertices.push_back( vSize + iVal );
                } else if ( 1 == iPos ) {
                    face->m_texturCoords.push_back( vtSize + iVal );
                } else if ( 2 == iPos ) {
                    face->m_normals.push_back( vnSize + iVal );
                    hasNormal = true;
                } else {
                    reportErrorTokenInFace();
                }
            } else {
                //On error, std::atoi will return 0 which is not a valid value
                delete face;
                throw DeadlyImportError("OBJ: Invalid face indice");
            }

        }
        m_DataIt += iStep;
    }

    if ( face->m_vertices.empty() ) {
        ASSIMP_LOG_ERROR("Obj: Ignoring empty face");
        // skip line and clean up
        m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
        delete face;
        return;
    }

    // Set active material, if one set
    if( NULL != m_pModel->m_pCurrentMaterial ) {
        face->m_pMaterial = m_pModel->m_pCurrentMaterial;
    } else {
        face->m_pMaterial = m_pModel->m_pDefaultMaterial;
    }

    // Create a default object, if nothing is there
    if( NULL == m_pModel->m_pCurrent ) {
        createObject( DefaultObjName );
    }

    // Assign face to mesh
    if ( NULL == m_pModel->m_pCurrentMesh ) {
        createMesh( DefaultObjName );
    }

    // Store the face
    m_pModel->m_pCurrentMesh->m_Faces.push_back( face );
    m_pModel->m_pCurrentMesh->m_uiNumIndices += (unsigned int) face->m_vertices.size();
    m_pModel->m_pCurrentMesh->m_uiUVCoordinates[ 0 ] += (unsigned int) face->m_texturCoords.size();
    if( !m_pModel->m_pCurrentMesh->m_hasNormals && hasNormal ) {
        m_pModel->m_pCurrentMesh->m_hasNormals = true;
    }
    // Skip the rest of the line
    m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
}

void ObjFileParser::getMaterialDesc() {
    // Get next data for material data
    m_DataIt = getNextToken<DataArrayIt>(m_DataIt, m_DataItEnd);
    if (m_DataIt == m_DataItEnd) {
        return;
    }

    char *pStart = &(*m_DataIt);
    while( m_DataIt != m_DataItEnd && !IsLineEnd( *m_DataIt ) ) {
        ++m_DataIt;
    }

    // In some cases we should ignore this 'usemtl' command, this variable helps us to do so
    bool skip = false;

    // Get name
    std::string strName(pStart, &(*m_DataIt));
    strName = trim_whitespaces(strName);
    if (strName.empty())
        skip = true;

    // If the current mesh has the same material, we simply ignore that 'usemtl' command
    // There is no need to create another object or even mesh here
    if ( m_pModel->m_pCurrentMaterial && m_pModel->m_pCurrentMaterial->MaterialName == aiString( strName ) ) {
        skip = true;
    }

    if (!skip) {
        // Search for material
        std::map<std::string, ObjFile::Material*>::iterator it = m_pModel->m_MaterialMap.find(strName);
        if (it == m_pModel->m_MaterialMap.end()) {
			// Not found, so we don't know anything about the material except for its name.
			// This may be the case if the material library is missing. We don't want to lose all
			// materials if that happens, so create a new named material instead of discarding it
			// completely.
            ASSIMP_LOG_ERROR("OBJ: failed to locate material " + strName + ", creating new material");
			m_pModel->m_pCurrentMaterial = new ObjFile::Material();
			m_pModel->m_pCurrentMaterial->MaterialName.Set(strName);
			m_pModel->m_MaterialLib.push_back(strName);
			m_pModel->m_MaterialMap[strName] = m_pModel->m_pCurrentMaterial;
        } else {
            // Found, using detected material
            m_pModel->m_pCurrentMaterial = (*it).second;
        }

        if ( needsNewMesh( strName ) ) {
            createMesh( strName );
        }

        m_pModel->m_pCurrentMesh->m_uiMaterialIndex = getMaterialIndex(strName);
    }

    // Skip rest of line
    m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
}

// -------------------------------------------------------------------
//  Get a comment, values will be skipped
void ObjFileParser::getComment() {
    m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
}

// -------------------------------------------------------------------
//  Get material library from file.
void ObjFileParser::getMaterialLib() {
    // Translate tuple
    m_DataIt = getNextToken<DataArrayIt>(m_DataIt, m_DataItEnd);
    if( m_DataIt == m_DataItEnd ) {
        return;
    }

    char *pStart = &(*m_DataIt);
    while( m_DataIt != m_DataItEnd && !IsLineEnd( *m_DataIt ) ) {
        ++m_DataIt;
    }

    // Check for existence
    const std::string strMatName(pStart, &(*m_DataIt));
    std::string absName;

	// Check if directive is valid.
    if ( 0 == strMatName.length() ) {
        ASSIMP_LOG_WARN( "OBJ: no name for material library specified." );
        return;
    }

    if ( m_pIO->StackSize() > 0 ) {
        std::string path = m_pIO->CurrentDirectory();
        if ( '/' != *path.rbegin() ) {
          path += '/';
        }
        absName += path;
        absName += strMatName;
    } else {
        absName = strMatName;
    }

    IOStream *pFile = m_pIO->Open( absName );
    if ( nullptr == pFile ) {
        ASSIMP_LOG_ERROR("OBJ: Unable to locate material file " + strMatName);
        std::string strMatFallbackName = m_originalObjFileName.substr(0, m_originalObjFileName.length() - 3) + "mtl";
        ASSIMP_LOG_INFO("OBJ: Opening fallback material file " + strMatFallbackName);
        pFile = m_pIO->Open(strMatFallbackName);
        if (!pFile) {
            ASSIMP_LOG_ERROR("OBJ: Unable to locate fallback material file " + strMatFallbackName);
            m_DataIt = skipLine<DataArrayIt>(m_DataIt, m_DataItEnd, m_uiLine);
            return;
        }
    }

    // Import material library data from file.
    // Some exporters (e.g. Silo) will happily write out empty
    // material files if the model doesn't use any materials, so we
    // allow that.
    std::vector<char> buffer;
    BaseImporter::TextFileToBuffer( pFile, buffer, BaseImporter::ALLOW_EMPTY );
    m_pIO->Close( pFile );

    // Importing the material library
    ObjFileMtlImporter mtlImporter( buffer, strMatName, m_pModel.get() );
}

// -------------------------------------------------------------------
//  Set a new material definition as the current material.
void ObjFileParser::getNewMaterial() {
    m_DataIt = getNextToken<DataArrayIt>(m_DataIt, m_DataItEnd);
    m_DataIt = getNextWord<DataArrayIt>(m_DataIt, m_DataItEnd);
    if( m_DataIt == m_DataItEnd ) {
        return;
    }

    char *pStart = &(*m_DataIt);
    std::string strMat( pStart, *m_DataIt );
    while( m_DataIt != m_DataItEnd && IsSpaceOrNewLine( *m_DataIt ) ) {
        ++m_DataIt;
    }
    std::map<std::string, ObjFile::Material*>::iterator it = m_pModel->m_MaterialMap.find( strMat );
    if ( it == m_pModel->m_MaterialMap.end() ) {
        // Show a warning, if material was not found
        ASSIMP_LOG_WARN("OBJ: Unsupported material requested: " + strMat);
        m_pModel->m_pCurrentMaterial = m_pModel->m_pDefaultMaterial;
    } else {
        // Set new material
        if ( needsNewMesh( strMat ) ) {
            createMesh( strMat );
        }
        m_pModel->m_pCurrentMesh->m_uiMaterialIndex = getMaterialIndex( strMat );
    }

    m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
}

// -------------------------------------------------------------------
int ObjFileParser::getMaterialIndex( const std::string &strMaterialName )
{
    int mat_index = -1;
    if( strMaterialName.empty() ) {
        return mat_index;
    }
    for (size_t index = 0; index < m_pModel->m_MaterialLib.size(); ++index)
    {
        if ( strMaterialName == m_pModel->m_MaterialLib[ index ])
        {
            mat_index = (int)index;
            break;
        }
    }
    return mat_index;
}

// -------------------------------------------------------------------
//  Getter for a group name.
void ObjFileParser::getGroupName() {
    std::string groupName;

    // here we skip 'g ' from line
    m_DataIt = getNextToken<DataArrayIt>(m_DataIt, m_DataItEnd);
    m_DataIt = getName<DataArrayIt>(m_DataIt, m_DataItEnd, groupName);
    if( isEndOfBuffer( m_DataIt, m_DataItEnd ) ) {
        return;
    }

    // Change active group, if necessary
    if ( m_pModel->m_strActiveGroup != groupName ) {
        // Search for already existing entry
        ObjFile::Model::ConstGroupMapIt it = m_pModel->m_Groups.find(groupName);

        // We are mapping groups into the object structure
        createObject( groupName );

        // New group name, creating a new entry
        if (it == m_pModel->m_Groups.end())
        {
            std::vector<unsigned int> *pFaceIDArray = new std::vector<unsigned int>;
            m_pModel->m_Groups[ groupName ] = pFaceIDArray;
            m_pModel->m_pGroupFaceIDs = (pFaceIDArray);
        }
        else
        {
            m_pModel->m_pGroupFaceIDs = (*it).second;
        }
        m_pModel->m_strActiveGroup = groupName;
    }
    m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
}

// -------------------------------------------------------------------
//  Not supported
void ObjFileParser::getGroupNumber()
{
    // Not used

    m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
}

// -------------------------------------------------------------------
//  Not supported
void ObjFileParser::getGroupNumberAndResolution()
{
    // Not used

    m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
}

// -------------------------------------------------------------------
//  Stores values for a new object instance, name will be used to
//  identify it.
void ObjFileParser::getObjectName()
{
    m_DataIt = getNextToken<DataArrayIt>(m_DataIt, m_DataItEnd);
    if( m_DataIt == m_DataItEnd ) {
        return;
    }
    char *pStart = &(*m_DataIt);
    while( m_DataIt != m_DataItEnd && !IsSpaceOrNewLine( *m_DataIt ) ) {
        ++m_DataIt;
    }

    std::string strObjectName(pStart, &(*m_DataIt));
    if (!strObjectName.empty())
    {
        // Reset current object
        m_pModel->m_pCurrent = NULL;

        // Search for actual object
        for (std::vector<ObjFile::Object*>::const_iterator it = m_pModel->m_Objects.begin();
            it != m_pModel->m_Objects.end();
            ++it)
        {
            if ((*it)->m_strObjName == strObjectName)
            {
                m_pModel->m_pCurrent = *it;
                break;
            }
        }

        // Allocate a new object, if current one was not found before
        if( NULL == m_pModel->m_pCurrent ) {
            createObject( strObjectName );
        }
    }
    m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
}
// -------------------------------------------------------------------
//  Creates a new object instance
void ObjFileParser::createObject(const std::string &objName)
{
    ai_assert( NULL != m_pModel );

    m_pModel->m_pCurrent = new ObjFile::Object;
    m_pModel->m_pCurrent->m_strObjName = objName;
    m_pModel->m_Objects.push_back( m_pModel->m_pCurrent );

    createMesh( objName  );

    if( m_pModel->m_pCurrentMaterial )
    {
        m_pModel->m_pCurrentMesh->m_uiMaterialIndex =
            getMaterialIndex( m_pModel->m_pCurrentMaterial->MaterialName.data );
        m_pModel->m_pCurrentMesh->m_pMaterial = m_pModel->m_pCurrentMaterial;
    }
}
// -------------------------------------------------------------------
//  Creates a new mesh
void ObjFileParser::createMesh( const std::string &meshName )
{
    ai_assert( NULL != m_pModel );
    m_pModel->m_pCurrentMesh = new ObjFile::Mesh( meshName );
    m_pModel->m_Meshes.push_back( m_pModel->m_pCurrentMesh );
    unsigned int meshId = static_cast<unsigned int>(m_pModel->m_Meshes.size()-1);
    if ( NULL != m_pModel->m_pCurrent )
    {
        m_pModel->m_pCurrent->m_Meshes.push_back( meshId );
    }
    else
    {
        ASSIMP_LOG_ERROR("OBJ: No object detected to attach a new mesh instance.");
    }
}

// -------------------------------------------------------------------
//  Returns true, if a new mesh must be created.
bool ObjFileParser::needsNewMesh( const std::string &materialName )
{
    // If no mesh data yet
    if(m_pModel->m_pCurrentMesh == 0)
    {
        return true;
    }
    bool newMat = false;
    int matIdx = getMaterialIndex( materialName );
    int curMatIdx = m_pModel->m_pCurrentMesh->m_uiMaterialIndex;
    if ( curMatIdx != int(ObjFile::Mesh::NoMaterial)
        && curMatIdx != matIdx
        // no need create a new mesh if no faces in current
        // lets say 'usemtl' goes straight after 'g'
        && m_pModel->m_pCurrentMesh->m_Faces.size() > 0 )
    {
        // New material -> only one material per mesh, so we need to create a new
        // material
        newMat = true;
    }
    return newMat;
}

// -------------------------------------------------------------------
//  Shows an error in parsing process.
void ObjFileParser::reportErrorTokenInFace()
{
    m_DataIt = skipLine<DataArrayIt>( m_DataIt, m_DataItEnd, m_uiLine );
    ASSIMP_LOG_ERROR("OBJ: Not supported token in face description detected");
}

// -------------------------------------------------------------------

}   // Namespace Assimp

#endif // !! ASSIMP_BUILD_NO_OBJ_IMPORTER
