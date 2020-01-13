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

/** @file  IFCUtil.cpp
 *  @brief Implementation of conversion routines for some common Ifc helper entities.
 */

#ifndef ASSIMP_BUILD_NO_IFC_IMPORTER

#include "Importer/IFC/IFCUtil.h"
#include "Common/PolyTools.h"
#include "PostProcessing/ProcessHelper.h"
#include <assimp/Defines.h>

namespace Assimp {
namespace IFC {

// ------------------------------------------------------------------------------------------------
void TempOpening::Transform(const IfcMatrix4& mat) {
    if(profileMesh) {
        profileMesh->Transform(mat);
    }
    if(profileMesh2D) {
        profileMesh2D->Transform(mat);
    }
    extrusionDir *= IfcMatrix3(mat);
}

// ------------------------------------------------------------------------------------------------
aiMesh* TempMesh::ToMesh()
{
    ai_assert(mVerts.size() == std::accumulate(mVertcnt.begin(),mVertcnt.end(),size_t(0)));

    if (mVerts.empty()) {
        return NULL;
    }

    std::unique_ptr<aiMesh> mesh(new aiMesh());

    // copy vertices
    mesh->mNumVertices = static_cast<unsigned int>(mVerts.size());
    mesh->mVertices = new aiVector3D[mesh->mNumVertices];
    std::copy(mVerts.begin(),mVerts.end(),mesh->mVertices);

    // and build up faces
    mesh->mNumFaces = static_cast<unsigned int>(mVertcnt.size());
    mesh->mFaces = new aiFace[mesh->mNumFaces];

    for(unsigned int i = 0,n=0, acc = 0; i < mesh->mNumFaces; ++n) {
        aiFace& f = mesh->mFaces[i];
        if (!mVertcnt[n]) {
            --mesh->mNumFaces;
            continue;
        }

        f.mNumIndices = mVertcnt[n];
        f.mIndices = new unsigned int[f.mNumIndices];
        for(unsigned int a = 0; a < f.mNumIndices; ++a) {
            f.mIndices[a] = acc++;
        }

        ++i;
    }

    return mesh.release();
}

// ------------------------------------------------------------------------------------------------
void TempMesh::Clear()
{
    mVerts.clear();
    mVertcnt.clear();
}

// ------------------------------------------------------------------------------------------------
void TempMesh::Transform(const IfcMatrix4& mat)
{
    for(IfcVector3& v : mVerts) {
        v *= mat;
    }
}

// ------------------------------------------------------------------------------
IfcVector3 TempMesh::Center() const
{
    return (mVerts.size() == 0) ? IfcVector3(0.0f, 0.0f, 0.0f) : (std::accumulate(mVerts.begin(),mVerts.end(),IfcVector3()) / static_cast<IfcFloat>(mVerts.size()));
}

// ------------------------------------------------------------------------------------------------
void TempMesh::Append(const TempMesh& other)
{
    mVerts.insert(mVerts.end(),other.mVerts.begin(),other.mVerts.end());
    mVertcnt.insert(mVertcnt.end(),other.mVertcnt.begin(),other.mVertcnt.end());
}

// ------------------------------------------------------------------------------------------------
void TempMesh::RemoveDegenerates()
{
    // The strategy is simple: walk the mesh and compute normals using
    // Newell's algorithm. The length of the normals gives the area
    // of the polygons, which is close to zero for lines.

    std::vector<IfcVector3> normals;
    ComputePolygonNormals(normals, false);

    bool drop = false;
    size_t inor = 0;

    std::vector<IfcVector3>::iterator vit = mVerts.begin();
    for (std::vector<unsigned int>::iterator it = mVertcnt.begin(); it != mVertcnt.end(); ++inor) {
        const unsigned int pcount = *it;

        if (normals[inor].SquareLength() < 1e-10f) {
            it = mVertcnt.erase(it);
            vit = mVerts.erase(vit, vit + pcount);

            drop = true;
            continue;
        }

        vit += pcount;
        ++it;
    }

    if(drop) {
        IFCImporter::LogDebug("removing degenerate faces");
    }
}

// ------------------------------------------------------------------------------------------------
IfcVector3 TempMesh::ComputePolygonNormal(const IfcVector3* vtcs, size_t cnt, bool normalize)
{
    std::vector<IfcFloat> temp((cnt+2)*3);
    for( size_t vofs = 0, i = 0; vofs < cnt; ++vofs )
    {
        const IfcVector3& v = vtcs[vofs];
        temp[i++] = v.x;
        temp[i++] = v.y;
        temp[i++] = v.z;
    }

    IfcVector3 nor;
    NewellNormal<3, 3, 3>(nor, static_cast<int>(cnt), &temp[0], &temp[1], &temp[2]);
    return normalize ? nor.Normalize() : nor;
}

// ------------------------------------------------------------------------------------------------
void TempMesh::ComputePolygonNormals(std::vector<IfcVector3>& normals,
    bool normalize,
    size_t ofs) const
{
    size_t max_vcount = 0;
    std::vector<unsigned int>::const_iterator begin = mVertcnt.begin()+ofs, end = mVertcnt.end(),  iit;
    for(iit = begin; iit != end; ++iit) {
        max_vcount = std::max(max_vcount,static_cast<size_t>(*iit));
    }

    std::vector<IfcFloat> temp((max_vcount+2)*4);
    normals.reserve( normals.size() + mVertcnt.size()-ofs );

    // `NewellNormal()` currently has a relatively strange interface and need to
    // re-structure things a bit to meet them.
    size_t vidx = std::accumulate(mVertcnt.begin(),begin,0);
    for(iit = begin; iit != end; vidx += *iit++) {
        if (!*iit) {
            normals.push_back(IfcVector3());
            continue;
        }
        for(size_t vofs = 0, cnt = 0; vofs < *iit; ++vofs) {
            const IfcVector3& v = mVerts[vidx+vofs];
            temp[cnt++] = v.x;
            temp[cnt++] = v.y;
            temp[cnt++] = v.z;
#ifdef ASSIMP_BUILD_DEBUG
            temp[cnt] = std::numeric_limits<IfcFloat>::quiet_NaN();
#endif
            ++cnt;
        }

        normals.push_back(IfcVector3());
        NewellNormal<4,4,4>(normals.back(),*iit,&temp[0],&temp[1],&temp[2]);
    }

    if(normalize) {
        for(IfcVector3& n : normals) {
            n.Normalize();
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Compute the normal of the last polygon in the given mesh
IfcVector3 TempMesh::ComputeLastPolygonNormal(bool normalize) const
{
    return ComputePolygonNormal(&mVerts[mVerts.size() - mVertcnt.back()], mVertcnt.back(), normalize);
}

struct CompareVector
{
    bool operator () (const IfcVector3& a, const IfcVector3& b) const
    {
        IfcVector3 d = a - b;
        IfcFloat eps = 1e-6;
        return d.x < -eps || (std::abs(d.x) < eps && d.y < -eps) || (std::abs(d.x) < eps && std::abs(d.y) < eps && d.z < -eps);
    }
};
struct FindVector
{
    IfcVector3 v;
    FindVector(const IfcVector3& p) : v(p) { }
    bool operator () (const IfcVector3& p) { return FuzzyVectorCompare(1e-6)(p, v); }
};

// ------------------------------------------------------------------------------------------------
void TempMesh::FixupFaceOrientation()
{
    const IfcVector3 vavg = Center();

    // create a list of start indices for all faces to allow random access to faces
    std::vector<size_t> faceStartIndices(mVertcnt.size());
    for( size_t i = 0, a = 0; a < mVertcnt.size(); i += mVertcnt[a], ++a )
        faceStartIndices[a] = i;

    // list all faces on a vertex
    std::map<IfcVector3, std::vector<size_t>, CompareVector> facesByVertex;
    for( size_t a = 0; a < mVertcnt.size(); ++a )
    {
        for( size_t b = 0; b < mVertcnt[a]; ++b )
            facesByVertex[mVerts[faceStartIndices[a] + b]].push_back(a);
    }
    // determine neighbourhood for all polys
    std::vector<size_t> neighbour(mVerts.size(), SIZE_MAX);
    std::vector<size_t> tempIntersect(10);
    for( size_t a = 0; a < mVertcnt.size(); ++a )
    {
        for( size_t b = 0; b < mVertcnt[a]; ++b )
        {
            size_t ib = faceStartIndices[a] + b, nib = faceStartIndices[a] + (b + 1) % mVertcnt[a];
            const std::vector<size_t>& facesOnB = facesByVertex[mVerts[ib]];
            const std::vector<size_t>& facesOnNB = facesByVertex[mVerts[nib]];
            // there should be exactly one or two faces which appear in both lists. Our face and the other side
            std::vector<size_t>::iterator sectstart = tempIntersect.begin();
            std::vector<size_t>::iterator sectend = std::set_intersection(
                facesOnB.begin(), facesOnB.end(), facesOnNB.begin(), facesOnNB.end(), sectstart);

            if( std::distance(sectstart, sectend) != 2 )
                continue;
            if( *sectstart == a )
                ++sectstart;
            neighbour[ib] = *sectstart;
        }
    }

    // now we're getting started. We take the face which is the farthest away from the center. This face is most probably
    // facing outwards. So we reverse this face to point outwards in relation to the center. Then we adapt neighbouring
    // faces to have the same winding until all faces have been tested.
    std::vector<bool> faceDone(mVertcnt.size(), false);
    while( std::count(faceDone.begin(), faceDone.end(), false) != 0 )
    {
        // find the farthest of the remaining faces
        size_t farthestIndex = SIZE_MAX;
        IfcFloat farthestDistance = -1.0;
        for( size_t a = 0; a < mVertcnt.size(); ++a )
        {
            if( faceDone[a] )
                continue;
            IfcVector3 faceCenter = std::accumulate(mVerts.begin() + faceStartIndices[a],
                mVerts.begin() + faceStartIndices[a] + mVertcnt[a], IfcVector3(0.0)) / IfcFloat(mVertcnt[a]);
            IfcFloat dst = (faceCenter - vavg).SquareLength();
            if( dst > farthestDistance ) { farthestDistance = dst; farthestIndex = a; }
        }

        // calculate its normal and reverse the poly if its facing towards the mesh center
        IfcVector3 farthestNormal = ComputePolygonNormal(mVerts.data() + faceStartIndices[farthestIndex], mVertcnt[farthestIndex]);
        IfcVector3 farthestCenter = std::accumulate(mVerts.begin() + faceStartIndices[farthestIndex],
            mVerts.begin() + faceStartIndices[farthestIndex] + mVertcnt[farthestIndex], IfcVector3(0.0))
            / IfcFloat(mVertcnt[farthestIndex]);
        // We accept a bit of negative orientation without reversing. In case of doubt, prefer the orientation given in
        // the file.
        if( (farthestNormal * (farthestCenter - vavg).Normalize()) < -0.4 )
        {
            size_t fsi = faceStartIndices[farthestIndex], fvc = mVertcnt[farthestIndex];
            std::reverse(mVerts.begin() + fsi, mVerts.begin() + fsi + fvc);
            std::reverse(neighbour.begin() + fsi, neighbour.begin() + fsi + fvc);
            // because of the neighbour index belonging to the edge starting with the point at the same index, we need to
            // cycle the neighbours through to match the edges again.
            // Before: points A - B - C - D with edge neighbour p - q - r - s
            // After: points D - C - B - A, reversed neighbours are s - r - q - p, but the should be
            //                r   q   p   s
            for( size_t a = 0; a < fvc - 1; ++a )
                std::swap(neighbour[fsi + a], neighbour[fsi + a + 1]);
        }
        faceDone[farthestIndex] = true;
        std::vector<size_t> todo;
        todo.push_back(farthestIndex);

        // go over its neighbour faces recursively and adapt their winding order to match the farthest face
        while( !todo.empty() )
        {
            size_t tdf = todo.back();
            size_t vsi = faceStartIndices[tdf], vc = mVertcnt[tdf];
            todo.pop_back();

            // check its neighbours
            for( size_t a = 0; a < vc; ++a )
            {
                // ignore neighbours if we already checked them
                size_t nbi = neighbour[vsi + a];
                if( nbi == SIZE_MAX || faceDone[nbi] )
                    continue;

                const IfcVector3& vp = mVerts[vsi + a];
                size_t nbvsi = faceStartIndices[nbi], nbvc = mVertcnt[nbi];
                std::vector<IfcVector3>::iterator it = std::find_if(mVerts.begin() + nbvsi, mVerts.begin() + nbvsi + nbvc, FindVector(vp));
                ai_assert(it != mVerts.begin() + nbvsi + nbvc);
                size_t nb_vidx = std::distance(mVerts.begin() + nbvsi, it);
                // two faces winded in the same direction should have a crossed edge, where one face has p0->p1 and the other
                // has p1'->p0'. If the next point on the neighbouring face is also the next on the current face, we need
                // to reverse the neighbour
                nb_vidx = (nb_vidx + 1) % nbvc;
                size_t oursideidx = (a + 1) % vc;
                if( FuzzyVectorCompare(1e-6)(mVerts[vsi + oursideidx], mVerts[nbvsi + nb_vidx]) )
                {
                    std::reverse(mVerts.begin() + nbvsi, mVerts.begin() + nbvsi + nbvc);
                    std::reverse(neighbour.begin() + nbvsi, neighbour.begin() + nbvsi + nbvc);
                    for( size_t a = 0; a < nbvc - 1; ++a )
                        std::swap(neighbour[nbvsi + a], neighbour[nbvsi + a + 1]);
                }

                // either way we're done with the neighbour. Mark it as done and continue checking from there recursively
                faceDone[nbi] = true;
                todo.push_back(nbi);
            }
        }

        // no more faces reachable from this part of the surface, start over with a disjunct part and its farthest face
    }
}

// ------------------------------------------------------------------------------------------------
void TempMesh::RemoveAdjacentDuplicates() {
    bool drop = false;
    std::vector<IfcVector3>::iterator base = mVerts.begin();
    for(unsigned int& cnt : mVertcnt) {
        if (cnt < 2){
            base += cnt;
            continue;
        }

        IfcVector3 vmin,vmax;
        ArrayBounds(&*base, cnt ,vmin,vmax);


        const IfcFloat epsilon = (vmax-vmin).SquareLength() / static_cast<IfcFloat>(1e9);
        //const IfcFloat dotepsilon = 1e-9;

        //// look for vertices that lie directly on the line between their predecessor and their
        //// successor and replace them with either of them.

        //for(size_t i = 0; i < cnt; ++i) {
        //  IfcVector3& v1 = *(base+i), &v0 = *(base+(i?i-1:cnt-1)), &v2 = *(base+(i+1)%cnt);
        //  const IfcVector3& d0 = (v1-v0), &d1 = (v2-v1);
        //  const IfcFloat l0 = d0.SquareLength(), l1 = d1.SquareLength();
        //  if (!l0 || !l1) {
        //      continue;
        //  }

        //  const IfcFloat d = (d0/std::sqrt(l0))*(d1/std::sqrt(l1));

        //  if ( d >= 1.f-dotepsilon ) {
        //      v1 = v0;
        //  }
        //  else if ( d < -1.f+dotepsilon ) {
        //      v2 = v1;
        //      continue;
        //  }
        //}

        // drop any identical, adjacent vertices. this pass will collect the dropouts
        // of the previous pass as a side-effect.
        FuzzyVectorCompare fz(epsilon);
        std::vector<IfcVector3>::iterator end = base+cnt, e = std::unique( base, end, fz );
        if (e != end) {
            cnt -= static_cast<unsigned int>(std::distance(e, end));
            mVerts.erase(e,end);
            drop  = true;
        }

        // check front and back vertices for this polygon
        if (cnt > 1 && fz(*base,*(base+cnt-1))) {
            mVerts.erase(base+ --cnt);
            drop  = true;
        }

        // removing adjacent duplicates shouldn't erase everything :-)
        ai_assert(cnt>0);
        base += cnt;
    }
    if(drop) {
        IFCImporter::LogDebug("removing duplicate vertices");
    }
}

// ------------------------------------------------------------------------------------------------
void TempMesh::Swap(TempMesh& other)
{
    mVertcnt.swap(other.mVertcnt);
    mVerts.swap(other.mVerts);
}

// ------------------------------------------------------------------------------------------------
bool IsTrue(const ::Assimp::STEP::EXPRESS::BOOLEAN& in)
{
    return (std::string)in == "TRUE" || (std::string)in == "T";
}

// ------------------------------------------------------------------------------------------------
IfcFloat ConvertSIPrefix(const std::string& prefix)
{
    if (prefix == "EXA") {
        return 1e18f;
    }
    else if (prefix == "PETA") {
        return 1e15f;
    }
    else if (prefix == "TERA") {
        return 1e12f;
    }
    else if (prefix == "GIGA") {
        return 1e9f;
    }
    else if (prefix == "MEGA") {
        return 1e6f;
    }
    else if (prefix == "KILO") {
        return 1e3f;
    }
    else if (prefix == "HECTO") {
        return 1e2f;
    }
    else if (prefix == "DECA") {
        return 1e-0f;
    }
    else if (prefix == "DECI") {
        return 1e-1f;
    }
    else if (prefix == "CENTI") {
        return 1e-2f;
    }
    else if (prefix == "MILLI") {
        return 1e-3f;
    }
    else if (prefix == "MICRO") {
        return 1e-6f;
    }
    else if (prefix == "NANO") {
        return 1e-9f;
    }
    else if (prefix == "PICO") {
        return 1e-12f;
    }
    else if (prefix == "FEMTO") {
        return 1e-15f;
    }
    else if (prefix == "ATTO") {
        return 1e-18f;
    }
    else {
        IFCImporter::LogError("Unrecognized SI prefix: " + prefix);
        return 1;
    }
}

// ------------------------------------------------------------------------------------------------
void ConvertColor(aiColor4D& out, const Schema_2x3::IfcColourRgb& in)
{
    out.r = static_cast<float>( in.Red );
    out.g = static_cast<float>( in.Green );
    out.b = static_cast<float>( in.Blue );
    out.a = static_cast<float>( 1.f );
}

// ------------------------------------------------------------------------------------------------
void ConvertColor(aiColor4D& out, const Schema_2x3::IfcColourOrFactor& in,ConversionData& conv,const aiColor4D* base)
{
    if (const ::Assimp::STEP::EXPRESS::REAL* const r = in.ToPtr<::Assimp::STEP::EXPRESS::REAL>()) {
        out.r = out.g = out.b = static_cast<float>(*r);
        if(base) {
            out.r *= static_cast<float>( base->r );
            out.g *= static_cast<float>( base->g );
            out.b *= static_cast<float>( base->b );
            out.a = static_cast<float>( base->a );
        }
        else out.a = 1.0;
    }
    else if (const Schema_2x3::IfcColourRgb* const rgb = in.ResolveSelectPtr<Schema_2x3::IfcColourRgb>(conv.db)) {
        ConvertColor(out,*rgb);
    }
    else {
        IFCImporter::LogWarn("skipping unknown IfcColourOrFactor entity");
    }
}

// ------------------------------------------------------------------------------------------------
void ConvertCartesianPoint(IfcVector3& out, const Schema_2x3::IfcCartesianPoint& in)
{
    out = IfcVector3();
    for(size_t i = 0; i < in.Coordinates.size(); ++i) {
        out[static_cast<unsigned int>(i)] = in.Coordinates[i];
    }
}

// ------------------------------------------------------------------------------------------------
void ConvertVector(IfcVector3& out, const Schema_2x3::IfcVector& in)
{
    ConvertDirection(out,in.Orientation);
    out *= in.Magnitude;
}

// ------------------------------------------------------------------------------------------------
void ConvertDirection(IfcVector3& out, const Schema_2x3::IfcDirection& in)
{
    out = IfcVector3();
    for(size_t i = 0; i < in.DirectionRatios.size(); ++i) {
        out[static_cast<unsigned int>(i)] = in.DirectionRatios[i];
    }
    const IfcFloat len = out.Length();
    if (len<1e-6) {
        IFCImporter::LogWarn("direction vector magnitude too small, normalization would result in a division by zero");
        return;
    }
    out /= len;
}

// ------------------------------------------------------------------------------------------------
void AssignMatrixAxes(IfcMatrix4& out, const IfcVector3& x, const IfcVector3& y, const IfcVector3& z)
{
    out.a1 = x.x;
    out.b1 = x.y;
    out.c1 = x.z;

    out.a2 = y.x;
    out.b2 = y.y;
    out.c2 = y.z;

    out.a3 = z.x;
    out.b3 = z.y;
    out.c3 = z.z;
}

// ------------------------------------------------------------------------------------------------
void ConvertAxisPlacement(IfcMatrix4& out, const Schema_2x3::IfcAxis2Placement3D& in)
{
    IfcVector3 loc;
    ConvertCartesianPoint(loc,in.Location);

    IfcVector3 z(0.f,0.f,1.f),r(1.f,0.f,0.f),x;

    if (in.Axis) {
        ConvertDirection(z,*in.Axis.Get());
    }
    if (in.RefDirection) {
        ConvertDirection(r,*in.RefDirection.Get());
    }

    IfcVector3 v = r.Normalize();
    IfcVector3 tmpx = z * (v*z);

    x = (v-tmpx).Normalize();
    IfcVector3 y = (z^x);

    IfcMatrix4::Translation(loc,out);
    AssignMatrixAxes(out,x,y,z);
}

// ------------------------------------------------------------------------------------------------
void ConvertAxisPlacement(IfcMatrix4& out, const Schema_2x3::IfcAxis2Placement2D& in)
{
    IfcVector3 loc;
    ConvertCartesianPoint(loc,in.Location);

    IfcVector3 x(1.f,0.f,0.f);
    if (in.RefDirection) {
        ConvertDirection(x,*in.RefDirection.Get());
    }

    const IfcVector3 y = IfcVector3(x.y,-x.x,0.f);

    IfcMatrix4::Translation(loc,out);
    AssignMatrixAxes(out,x,y,IfcVector3(0.f,0.f,1.f));
}

// ------------------------------------------------------------------------------------------------
void ConvertAxisPlacement(IfcVector3& axis, IfcVector3& pos, const Schema_2x3::IfcAxis1Placement& in)
{
    ConvertCartesianPoint(pos,in.Location);
    if (in.Axis) {
        ConvertDirection(axis,in.Axis.Get());
    }
    else {
        axis = IfcVector3(0.f,0.f,1.f);
    }
}

// ------------------------------------------------------------------------------------------------
void ConvertAxisPlacement(IfcMatrix4& out, const Schema_2x3::IfcAxis2Placement& in, ConversionData& conv)
{
    if(const Schema_2x3::IfcAxis2Placement3D* pl3 = in.ResolveSelectPtr<Schema_2x3::IfcAxis2Placement3D>(conv.db)) {
        ConvertAxisPlacement(out,*pl3);
    }
    else if(const Schema_2x3::IfcAxis2Placement2D* pl2 = in.ResolveSelectPtr<Schema_2x3::IfcAxis2Placement2D>(conv.db)) {
        ConvertAxisPlacement(out,*pl2);
    }
    else {
        IFCImporter::LogWarn("skipping unknown IfcAxis2Placement entity");
    }
}

// ------------------------------------------------------------------------------------------------
void ConvertTransformOperator(IfcMatrix4& out, const Schema_2x3::IfcCartesianTransformationOperator& op)
{
    IfcVector3 loc;
    ConvertCartesianPoint(loc,op.LocalOrigin);

    IfcVector3 x(1.f,0.f,0.f),y(0.f,1.f,0.f),z(0.f,0.f,1.f);
    if (op.Axis1) {
        ConvertDirection(x,*op.Axis1.Get());
    }
    if (op.Axis2) {
        ConvertDirection(y,*op.Axis2.Get());
    }
    if (const Schema_2x3::IfcCartesianTransformationOperator3D* op2 = op.ToPtr<Schema_2x3::IfcCartesianTransformationOperator3D>()) {
        if(op2->Axis3) {
            ConvertDirection(z,*op2->Axis3.Get());
        }
    }

    IfcMatrix4 locm;
    IfcMatrix4::Translation(loc,locm);
    AssignMatrixAxes(out,x,y,z);


    IfcVector3 vscale;
    if (const Schema_2x3::IfcCartesianTransformationOperator3DnonUniform* nuni = op.ToPtr<Schema_2x3::IfcCartesianTransformationOperator3DnonUniform>()) {
        vscale.x = nuni->Scale?op.Scale.Get():1.f;
        vscale.y = nuni->Scale2?nuni->Scale2.Get():1.f;
        vscale.z = nuni->Scale3?nuni->Scale3.Get():1.f;
    }
    else {
        const IfcFloat sc = op.Scale?op.Scale.Get():1.f;
        vscale = IfcVector3(sc,sc,sc);
    }

    IfcMatrix4 s;
    IfcMatrix4::Scaling(vscale,s);

    out = locm * out * s;
}


} // ! IFC
} // ! Assimp

#endif
