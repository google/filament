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

/** @file  BlenderBMesh.h
 *  @brief Conversion of Blender's new BMesh stuff
 */
#ifndef INCLUDED_AI_BLEND_BMESH_H
#define INCLUDED_AI_BLEND_BMESH_H

#include "LogAux.h"

namespace Assimp
{
    // TinyFormatter.h
    namespace Formatter
    {
        template < typename T,typename TR, typename A > class basic_formatter;
        typedef class basic_formatter< char, std::char_traits< char >, std::allocator< char > > format;
    }

    // BlenderScene.h
    namespace Blender
    {
        struct Mesh;
        struct MPoly;
        struct MLoop;
    }

    class BlenderBMeshConverter: public LogFunctions< BlenderBMeshConverter >
    {
    public:
        BlenderBMeshConverter( const Blender::Mesh* mesh );
        ~BlenderBMeshConverter( );

        bool ContainsBMesh( ) const;

        const Blender::Mesh* TriangulateBMesh( );

    private:
        void AssertValidMesh( );
        void AssertValidSizes( );
        void PrepareTriMesh( );
        void DestroyTriMesh( );
        void ConvertPolyToFaces( const Blender::MPoly& poly );
        void AddFace( int v1, int v2, int v3, int v4 = 0 );
        void AddTFace( const float* uv1, const float* uv2, const float *uv3, const float* uv4 = 0 );

        const Blender::Mesh* BMesh;
        Blender::Mesh* triMesh;

        friend class BlenderTessellatorGL;
        friend class BlenderTessellatorP2T;
    };

} // end of namespace Assimp

#endif // INCLUDED_AI_BLEND_BMESH_H
