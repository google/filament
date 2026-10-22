/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team


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

/** @file  BlenderTessellator.cpp
 *  @brief A simple tessellation wrapper
 */


#ifndef ASSIMP_BUILD_NO_BLEND_IMPORTER

#include "BlenderDNA.h"
#include "BlenderScene.h"
#include "BlenderBMesh.h"
#include "BlenderTessellator.h"

#include <stddef.h>

static const unsigned int BLEND_TESS_MAGIC = 0x83ed9ac3;

#if ASSIMP_BLEND_WITH_GLU_TESSELLATE

namspace Assimp
{
    template< > const char* LogFunctions< BlenderTessellatorGL >::Prefix()
    {
        static auto prefix = "BLEND_TESS_GL: ";
        return prefix;
    }
}

using namespace Assimp;
using namespace Assimp::Blender;

#ifndef CALLBACK
#define CALLBACK
#endif

// ------------------------------------------------------------------------------------------------
BlenderTessellatorGL::BlenderTessellatorGL( BlenderBMeshConverter& converter ):
    converter( &converter )
{
}

// ------------------------------------------------------------------------------------------------
BlenderTessellatorGL::~BlenderTessellatorGL( )
{
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorGL::Tessellate( const MLoop* polyLoop, int vertexCount, const std::vector< MVert >& vertices )
{
    AssertVertexCount( vertexCount );

    std::vector< VertexGL > polyLoopGL;
    GenerateLoopVerts( polyLoopGL, polyLoop, vertexCount, vertices );

    TessDataGL tessData;
    Tesssellate( polyLoopGL, tessData );

    TriangulateDrawCalls( tessData );
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorGL::AssertVertexCount( int vertexCount )
{
    if ( vertexCount <= 4 )
    {
        ThrowException( "Expected more than 4 vertices for tessellation" );
    }
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorGL::GenerateLoopVerts( std::vector< VertexGL >& polyLoopGL, const MLoop* polyLoop, int vertexCount, const std::vector< MVert >& vertices )
{
    for ( int i = 0; i < vertexCount; ++i )
    {
        const MLoop& loopItem = polyLoop[ i ];
        const MVert& vertex = vertices[ loopItem.v ];
        polyLoopGL.push_back( VertexGL( vertex.co[ 0 ], vertex.co[ 1 ], vertex.co[ 2 ], loopItem.v, BLEND_TESS_MAGIC ) );
    }
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorGL::Tesssellate( std::vector< VertexGL >& polyLoopGL, TessDataGL& tessData )
{
    GLUtesselator* tessellator = gluNewTess( );
    gluTessCallback( tessellator, GLU_TESS_BEGIN_DATA, reinterpret_cast< void ( CALLBACK * )( ) >( TessellateBegin ) );
    gluTessCallback( tessellator, GLU_TESS_END_DATA, reinterpret_cast< void ( CALLBACK * )( ) >( TessellateEnd ) );
    gluTessCallback( tessellator, GLU_TESS_VERTEX_DATA, reinterpret_cast< void ( CALLBACK * )( ) >( TessellateVertex ) );
    gluTessCallback( tessellator, GLU_TESS_COMBINE_DATA, reinterpret_cast< void ( CALLBACK * )( ) >( TessellateCombine ) );
    gluTessCallback( tessellator, GLU_TESS_EDGE_FLAG_DATA, reinterpret_cast< void ( CALLBACK * )( ) >( TessellateEdgeFlag ) );
    gluTessCallback( tessellator, GLU_TESS_ERROR_DATA, reinterpret_cast< void ( CALLBACK * )( ) >( TessellateError ) );
    gluTessProperty( tessellator, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO );

    gluTessBeginPolygon( tessellator, &tessData );
    gluTessBeginContour( tessellator );

    for ( unsigned int i = 0; i < polyLoopGL.size( ); ++i )
    {
        gluTessVertex( tessellator, reinterpret_cast< GLdouble* >( &polyLoopGL[ i ] ), &polyLoopGL[ i ] );
    }

    gluTessEndContour( tessellator );
    gluTessEndPolygon( tessellator );
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorGL::TriangulateDrawCalls( const TessDataGL& tessData )
{
    // NOTE - Because we are supplying a callback to GLU_TESS_EDGE_FLAG_DATA we don't technically
    //        need support for GL_TRIANGLE_STRIP and GL_TRIANGLE_FAN but we'll keep it here in case
    //        GLU tessellate changes or tri-strips and fans are wanted.
    //        See: http://www.opengl.org/sdk/docs/man2/xhtml/gluTessCallback.xml
    for ( unsigned int i = 0; i < tessData.drawCalls.size( ); ++i )
    {
        const DrawCallGL& drawCallGL = tessData.drawCalls[ i ];
        const VertexGL* vertices = &tessData.vertices[ drawCallGL.baseVertex ];
        if ( drawCallGL.drawMode == GL_TRIANGLES )
        {
            MakeFacesFromTris( vertices, drawCallGL.vertexCount );
        }
        else if ( drawCallGL.drawMode == GL_TRIANGLE_STRIP )
        {
            MakeFacesFromTriStrip( vertices, drawCallGL.vertexCount );
        }
        else if ( drawCallGL.drawMode == GL_TRIANGLE_FAN )
        {
            MakeFacesFromTriFan( vertices, drawCallGL.vertexCount );
        }
    }
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorGL::MakeFacesFromTris( const VertexGL* vertices, int vertexCount )
{
    const int triangleCount = vertexCount / 3;
    for ( int i = 0; i < triangleCount; ++i )
    {
        int vertexBase = i * 3;
        converter->AddFace( vertices[ vertexBase + 0 ].index, vertices[ vertexBase + 1 ].index, vertices[ vertexBase + 2 ].index );
    }
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorGL::MakeFacesFromTriStrip( const VertexGL* vertices, int vertexCount )
{
    const int triangleCount = vertexCount - 2;
    for ( int i = 0; i < triangleCount; ++i )
    {
        int vertexBase = i;
        converter->AddFace( vertices[ vertexBase + 0 ].index, vertices[ vertexBase + 1 ].index, vertices[ vertexBase + 2 ].index );
    }
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorGL::MakeFacesFromTriFan( const VertexGL* vertices, int vertexCount )
{
    const int triangleCount = vertexCount - 2;
    for ( int i = 0; i < triangleCount; ++i )
    {
        int vertexBase = i;
        converter->AddFace( vertices[ 0 ].index, vertices[ vertexBase + 1 ].index, vertices[ vertexBase + 2 ].index );
    }
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorGL::TessellateBegin( GLenum drawModeGL, void* userData )
{
    TessDataGL& tessData = *reinterpret_cast< TessDataGL* >( userData );
    tessData.drawCalls.push_back( DrawCallGL( drawModeGL, tessData.vertices.size( ) ) );
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorGL::TessellateEnd( void* )
{
    // Do nothing
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorGL::TessellateVertex( const void* vtxData, void* userData )
{
    TessDataGL& tessData = *reinterpret_cast< TessDataGL* >( userData );

    const VertexGL& vertex = *reinterpret_cast< const VertexGL* >( vtxData );
    if ( vertex.magic != BLEND_TESS_MAGIC )
    {
        ThrowException( "Point returned by GLU Tessellate was probably not one of ours. This indicates we need a new way to store vertex information" );
    }
    tessData.vertices.push_back( vertex );
    if ( tessData.drawCalls.size( ) == 0 )
    {
        ThrowException( "\"Vertex\" callback received before \"Begin\"" );
    }
    ++( tessData.drawCalls.back( ).vertexCount );
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorGL::TessellateCombine( const GLdouble intersection[ 3 ], const GLdouble* [ 4 ], const GLfloat [ 4 ], GLdouble** out, void* userData )
{
    ThrowException( "Intersected polygon loops are not yet supported" );
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorGL::TessellateEdgeFlag( GLboolean, void* )
{
    // Do nothing
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorGL::TessellateError( GLenum errorCode, void* )
{
    ThrowException( reinterpret_cast< const char* >( gluErrorString( errorCode ) ) );
}

#endif // ASSIMP_BLEND_WITH_GLU_TESSELLATE

#if ASSIMP_BLEND_WITH_POLY_2_TRI

namespace Assimp
{
    template< > const char* LogFunctions< BlenderTessellatorP2T >::Prefix()
    {
        static auto prefix = "BLEND_TESS_P2T: ";
        return prefix;
    }
}

using namespace Assimp;
using namespace Assimp::Blender;

// ------------------------------------------------------------------------------------------------
BlenderTessellatorP2T::BlenderTessellatorP2T( BlenderBMeshConverter& converter ):
    converter( &converter )
{
}

// ------------------------------------------------------------------------------------------------
BlenderTessellatorP2T::~BlenderTessellatorP2T( )
{
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorP2T::Tessellate( const MLoop* polyLoop, int vertexCount, const std::vector< MVert >& vertices )
{
    AssertVertexCount( vertexCount );

    // NOTE - We have to hope that points in a Blender polygon are roughly on the same plane.
    //        There may be some triangulation artifacts if they are wildly different.

    std::vector< PointP2T > points;
    Copy3DVertices( polyLoop, vertexCount, vertices, points );

    PlaneP2T plane = FindLLSQPlane( points );

    aiMatrix4x4 transform = GeneratePointTransformMatrix( plane );

    TransformAndFlattenVectices( transform, points );

    std::vector< p2t::Point* > pointRefs;
    ReferencePoints( points, pointRefs );

    p2t::CDT cdt( pointRefs );

    cdt.Triangulate( );
    std::vector< p2t::Triangle* > triangles = cdt.GetTriangles( );

    MakeFacesFromTriangles( triangles );
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorP2T::AssertVertexCount( int vertexCount )
{
    if ( vertexCount <= 4 )
    {
        ThrowException( "Expected more than 4 vertices for tessellation" );
    }
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorP2T::Copy3DVertices( const MLoop* polyLoop, int vertexCount, const std::vector< MVert >& vertices, std::vector< PointP2T >& points ) const
{
    points.resize( vertexCount );
    for ( int i = 0; i < vertexCount; ++i )
    {
        const MLoop& loop = polyLoop[ i ];
        const MVert& vert = vertices[ loop.v ];

        PointP2T& point = points[ i ];
        point.point3D.Set( vert.co[ 0 ], vert.co[ 1 ], vert.co[ 2 ] );
        point.index = loop.v;
        point.magic = BLEND_TESS_MAGIC;
    }
}

// ------------------------------------------------------------------------------------------------
aiMatrix4x4 BlenderTessellatorP2T::GeneratePointTransformMatrix( const Blender::PlaneP2T& plane ) const
{
    aiVector3D sideA( 1.0f, 0.0f, 0.0f );
    if ( std::fabs( plane.normal * sideA ) > 0.999f )
    {
        sideA = aiVector3D( 0.0f, 1.0f, 0.0f );
    }

    aiVector3D sideB( plane.normal ^ sideA );
    sideB.Normalize( );
    sideA = sideB ^ plane.normal;

    aiMatrix4x4 result;
    result.a1 = sideA.x;
    result.a2 = sideA.y;
    result.a3 = sideA.z;
    result.b1 = sideB.x;
    result.b2 = sideB.y;
    result.b3 = sideB.z;
    result.c1 = plane.normal.x;
    result.c2 = plane.normal.y;
    result.c3 = plane.normal.z;
    result.a4 = plane.centre.x;
    result.b4 = plane.centre.y;
    result.c4 = plane.centre.z;
    result.Inverse( );

    return result;
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorP2T::TransformAndFlattenVectices( const aiMatrix4x4& transform, std::vector< Blender::PointP2T >& vertices ) const
{
    for ( size_t i = 0; i < vertices.size( ); ++i )
    {
        PointP2T& point = vertices[ i ];
        point.point3D = transform * point.point3D;
        point.point2D.set( point.point3D.y, point.point3D.z );
    }
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorP2T::ReferencePoints( std::vector< Blender::PointP2T >& points, std::vector< p2t::Point* >& pointRefs ) const
{
    pointRefs.resize( points.size( ) );
    for ( size_t i = 0; i < points.size( ); ++i )
    {
        pointRefs[ i ] = &points[ i ].point2D;
    }
}

// ------------------------------------------------------------------------------------------------
inline PointP2T& BlenderTessellatorP2T::GetActualPointStructure( p2t::Point& point ) const
{
    unsigned int pointOffset = offsetof( PointP2T, point2D );
    PointP2T& pointStruct = *reinterpret_cast< PointP2T* >( reinterpret_cast< char* >( &point ) - pointOffset );
    if ( pointStruct.magic != static_cast<int>( BLEND_TESS_MAGIC ) )
    {
        ThrowException( "Point returned by poly2tri was probably not one of ours. This indicates we need a new way to store vertex information" );
    }
    return pointStruct;
}

// ------------------------------------------------------------------------------------------------
void BlenderTessellatorP2T::MakeFacesFromTriangles( std::vector< p2t::Triangle* >& triangles ) const
{
    for ( size_t i = 0; i < triangles.size( ); ++i )
    {
        p2t::Triangle& Triangle = *triangles[ i ];

        PointP2T& pointA = GetActualPointStructure( *Triangle.GetPoint( 0 ) );
        PointP2T& pointB = GetActualPointStructure( *Triangle.GetPoint( 1 ) );
        PointP2T& pointC = GetActualPointStructure( *Triangle.GetPoint( 2 ) );

        converter->AddFace( pointA.index, pointB.index, pointC.index );
    }
}

// ------------------------------------------------------------------------------------------------
inline float p2tMax( float a, float b )
{
    return a > b ? a : b;
}

// ------------------------------------------------------------------------------------------------
// Adapted from: http://missingbytes.blogspot.co.uk/2012/06/fitting-plane-to-point-cloud.html
float BlenderTessellatorP2T::FindLargestMatrixElem( const aiMatrix3x3& mtx ) const
{
    float result = 0.0f;

    for ( unsigned int x = 0; x < 3; ++x )
    {
        for ( unsigned int y = 0; y < 3; ++y )
        {
            result = p2tMax( std::fabs( mtx[ x ][ y ] ), result );
        }
    }

    return result;
}

// ------------------------------------------------------------------------------------------------
// Apparently Assimp doesn't have matrix scaling
aiMatrix3x3 BlenderTessellatorP2T::ScaleMatrix( const aiMatrix3x3& mtx, float scale ) const
{
    aiMatrix3x3 result;

    for ( unsigned int x = 0; x < 3; ++x )
    {
        for ( unsigned int y = 0; y < 3; ++y )
        {
            result[ x ][ y ] = mtx[ x ][ y ] * scale;
        }
    }

    return result;
}


// ------------------------------------------------------------------------------------------------
// Adapted from: http://missingbytes.blogspot.co.uk/2012/06/fitting-plane-to-point-cloud.html
aiVector3D BlenderTessellatorP2T::GetEigenVectorFromLargestEigenValue( const aiMatrix3x3& mtx ) const
{
    const float scale = FindLargestMatrixElem( mtx );
    aiMatrix3x3 mc = ScaleMatrix( mtx, 1.0f / scale );
    mc = mc * mc * mc;

    aiVector3D v( 1.0f );
    aiVector3D lastV = v;
    for ( int i = 0; i < 100; ++i )
    {
        v = mc * v;
        v.Normalize( );
        if ( ( v - lastV ).SquareLength( ) < 1e-16f )
        {
            break;
        }
        lastV = v;
    }
    return v;
}

// ------------------------------------------------------------------------------------------------
// Adapted from: http://missingbytes.blogspot.co.uk/2012/06/fitting-plane-to-point-cloud.html
PlaneP2T BlenderTessellatorP2T::FindLLSQPlane( const std::vector< PointP2T >& points ) const
{
    PlaneP2T result;

    aiVector3D sum( 0.0 );
    for ( size_t i = 0; i < points.size( ); ++i )
    {
        sum += points[ i ].point3D;
    }
    result.centre = sum * (ai_real)( 1.0 / points.size( ) );

    ai_real sumXX = 0.0;
    ai_real sumXY = 0.0;
    ai_real sumXZ = 0.0;
    ai_real sumYY = 0.0;
    ai_real sumYZ = 0.0;
    ai_real sumZZ = 0.0;
    for ( size_t i = 0; i < points.size( ); ++i )
    {
        aiVector3D offset = points[ i ].point3D - result.centre;
        sumXX += offset.x * offset.x;
        sumXY += offset.x * offset.y;
        sumXZ += offset.x * offset.z;
        sumYY += offset.y * offset.y;
        sumYZ += offset.y * offset.z;
        sumZZ += offset.z * offset.z;
    }

    aiMatrix3x3 mtx( sumXX, sumXY, sumXZ, sumXY, sumYY, sumYZ, sumXZ, sumYZ, sumZZ );

    const ai_real det = mtx.Determinant( );
    if ( det == 0.0f )
    {
        result.normal = aiVector3D( 0.0f );
    }
    else
    {
        aiMatrix3x3 invMtx = mtx;
        invMtx.Inverse( );
        result.normal = GetEigenVectorFromLargestEigenValue( invMtx );
    }

    return result;
}

#endif // ASSIMP_BLEND_WITH_POLY_2_TRI

#endif // ASSIMP_BUILD_NO_BLEND_IMPORTER
