/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2017, assimp team

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

/** @file  BlenderTessellator.h
 *  @brief A simple tessellation wrapper
 */
#ifndef INCLUDED_AI_BLEND_TESSELLATOR_H
#define INCLUDED_AI_BLEND_TESSELLATOR_H

// Use these to toggle between GLU Tessellate or poly2tri
// Note (acg) keep GLU Tesselate disabled by default - if it is turned on,
// assimp needs to be linked against GLU, which is currently not yet
// made configurable in CMake and potentially not wanted by most users
// as it requires a Gl environment.
#ifndef ASSIMP_BLEND_WITH_GLU_TESSELLATE
#   define ASSIMP_BLEND_WITH_GLU_TESSELLATE 0
#endif

#ifndef ASSIMP_BLEND_WITH_POLY_2_TRI
#   define ASSIMP_BLEND_WITH_POLY_2_TRI 1
#endif

#include "LogAux.h"

#if ASSIMP_BLEND_WITH_GLU_TESSELLATE

#if defined( WIN32 ) || defined( _WIN32 ) || defined( _MSC_VER )
#include <windows.h>
#endif
#include <GL/glu.h>

namespace Assimp
{
    class BlenderBMeshConverter;

    // TinyFormatter.h
    namespace Formatter
    {
        template < typename T,typename TR, typename A > class basic_formatter;
        typedef class basic_formatter< char, std::char_traits< char >, std::allocator< char > > format;
    }

    // BlenderScene.h
    namespace Blender
    {
        struct MLoop;
        struct MVert;

        struct VertexGL
        {
            GLdouble X;
            GLdouble Y;
            GLdouble Z;
            int index;
            int magic;

            VertexGL( GLdouble X, GLdouble Y, GLdouble Z, int index, int magic ): X( X ), Y( Y ), Z( Z ), index( index ), magic( magic ) { }
        };

        struct DrawCallGL
        {
            GLenum drawMode;
            int baseVertex;
            int vertexCount;

            DrawCallGL( GLenum drawMode, int baseVertex ): drawMode( drawMode ), baseVertex( baseVertex ), vertexCount( 0 ) { }
        };

        struct TessDataGL
        {
            std::vector< DrawCallGL > drawCalls;
            std::vector< VertexGL > vertices;
        };
    }

    class BlenderTessellatorGL: public LogFunctions< BlenderTessellatorGL >
    {
    public:
        BlenderTessellatorGL( BlenderBMeshConverter& converter );
        ~BlenderTessellatorGL( );

        void Tessellate( const Blender::MLoop* polyLoop, int vertexCount, const std::vector< Blender::MVert >& vertices );

    private:
        void AssertVertexCount( int vertexCount );
        void GenerateLoopVerts( std::vector< Blender::VertexGL >& polyLoopGL, const Blender::MLoop* polyLoop, int vertexCount, const std::vector< Blender::MVert >& vertices );
        void Tesssellate( std::vector< Blender::VertexGL >& polyLoopGL, Blender::TessDataGL& tessData );
        void TriangulateDrawCalls( const Blender::TessDataGL& tessData );
        void MakeFacesFromTris( const Blender::VertexGL* vertices, int vertexCount );
        void MakeFacesFromTriStrip( const Blender::VertexGL* vertices, int vertexCount );
        void MakeFacesFromTriFan( const Blender::VertexGL* vertices, int vertexCount );

        static void TessellateBegin( GLenum drawModeGL, void* userData );
        static void TessellateEnd( void* userData );
        static void TessellateVertex( const void* vtxData, void* userData );
        static void TessellateCombine( const GLdouble intersection[ 3 ], const GLdouble* [ 4 ], const GLfloat [ 4 ], GLdouble** out, void* userData );
        static void TessellateEdgeFlag( GLboolean edgeFlag, void* userData );
        static void TessellateError( GLenum errorCode, void* userData );

        BlenderBMeshConverter* converter;
    };
} // end of namespace Assimp

#endif // ASSIMP_BLEND_WITH_GLU_TESSELLATE

#if ASSIMP_BLEND_WITH_POLY_2_TRI

#include "../contrib/poly2tri/poly2tri/poly2tri.h"

namespace Assimp
{
    class BlenderBMeshConverter;

    // TinyFormatter.h
    namespace Formatter
    {
        template < typename T,typename TR, typename A > class basic_formatter;
        typedef class basic_formatter< char, std::char_traits< char >, std::allocator< char > > format;
    }

    // BlenderScene.h
    namespace Blender
    {
        struct MLoop;
        struct MVert;

        struct PointP2T
        {
            aiVector3D point3D;
            p2t::Point point2D;
            int magic;
            int index;
        };

        struct PlaneP2T
        {
            aiVector3D centre;
            aiVector3D normal;
        };
    }

    class BlenderTessellatorP2T: public LogFunctions< BlenderTessellatorP2T >
    {
    public:
        BlenderTessellatorP2T( BlenderBMeshConverter& converter );
        ~BlenderTessellatorP2T( );

        void Tessellate( const Blender::MLoop* polyLoop, int vertexCount, const std::vector< Blender::MVert >& vertices );

    private:
        void AssertVertexCount( int vertexCount );
        void Copy3DVertices( const Blender::MLoop* polyLoop, int vertexCount, const std::vector< Blender::MVert >& vertices, std::vector< Blender::PointP2T >& targetVertices ) const;
        aiMatrix4x4 GeneratePointTransformMatrix( const Blender::PlaneP2T& plane ) const;
        void TransformAndFlattenVectices( const aiMatrix4x4& transform, std::vector< Blender::PointP2T >& vertices ) const;
        void ReferencePoints( std::vector< Blender::PointP2T >& points, std::vector< p2t::Point* >& pointRefs ) const;
        inline Blender::PointP2T& GetActualPointStructure( p2t::Point& point ) const;
        void MakeFacesFromTriangles( std::vector< p2t::Triangle* >& triangles ) const;

        // Adapted from: http://missingbytes.blogspot.co.uk/2012/06/fitting-plane-to-point-cloud.html
        float FindLargestMatrixElem( const aiMatrix3x3& mtx ) const;
        aiMatrix3x3 ScaleMatrix( const aiMatrix3x3& mtx, float scale ) const;
        aiVector3D GetEigenVectorFromLargestEigenValue( const aiMatrix3x3& mtx ) const;
        Blender::PlaneP2T FindLLSQPlane( const std::vector< Blender::PointP2T >& points ) const;

        BlenderBMeshConverter* converter;
    };
} // end of namespace Assimp

#endif // ASSIMP_BLEND_WITH_POLY_2_TRI

#endif // INCLUDED_AI_BLEND_TESSELLATOR_H
