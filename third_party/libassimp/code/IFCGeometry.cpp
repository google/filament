/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2010, assimp team
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

/** @file  IFCGeometry.cpp
 *  @brief Geometry conversion and synthesis for IFC
 */



#ifndef ASSIMP_BUILD_NO_IFC_IMPORTER
#include "IFCUtil.h"
#include "PolyTools.h"
#include "ProcessHelper.h"

#include "../contrib/poly2tri/poly2tri/poly2tri.h"
#include "../contrib/clipper/clipper.hpp"
#include <memory>

#include <iterator>

namespace Assimp {
    namespace IFC {

// ------------------------------------------------------------------------------------------------
bool ProcessPolyloop(const IfcPolyLoop& loop, TempMesh& meshout, ConversionData& /*conv*/)
{
    size_t cnt = 0;
    for(const IfcCartesianPoint& c : loop.Polygon) {
        IfcVector3 tmp;
        ConvertCartesianPoint(tmp,c);

        meshout.verts.push_back(tmp);
        ++cnt;
    }

    meshout.vertcnt.push_back(static_cast<unsigned int>(cnt));

    // zero- or one- vertex polyloops simply ignored
    if (meshout.vertcnt.back() > 1) {
        return true;
    }

    if (meshout.vertcnt.back()==1) {
        meshout.vertcnt.pop_back();
        meshout.verts.pop_back();
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
void ProcessPolygonBoundaries(TempMesh& result, const TempMesh& inmesh, size_t master_bounds = (size_t)-1)
{
    // handle all trivial cases
    if(inmesh.vertcnt.empty()) {
        return;
    }
    if(inmesh.vertcnt.size() == 1) {
        result.Append(inmesh);
        return;
    }

    ai_assert(std::count(inmesh.vertcnt.begin(), inmesh.vertcnt.end(), 0) == 0);

    typedef std::vector<unsigned int>::const_iterator face_iter;

    face_iter begin = inmesh.vertcnt.begin(), end = inmesh.vertcnt.end(), iit;
    std::vector<unsigned int>::const_iterator outer_polygon_it = end;

    // major task here: given a list of nested polygon boundaries (one of which
    // is the outer contour), reduce the triangulation task arising here to
    // one that can be solved using the "quadrulation" algorithm which we use
    // for pouring windows out of walls. The algorithm does not handle all
    // cases but at least it is numerically stable and gives "nice" triangles.

    // first compute normals for all polygons using Newell's algorithm
    // do not normalize 'normals', we need the original length for computing the polygon area
    std::vector<IfcVector3> normals;
    inmesh.ComputePolygonNormals(normals,false);

    // One of the polygons might be a IfcFaceOuterBound (in which case `master_bounds`
    // is its index). Sadly we can't rely on it, the docs say 'At most one of the bounds
    // shall be of the type IfcFaceOuterBound'
    IfcFloat area_outer_polygon = 1e-10f;
    if (master_bounds != (size_t)-1) {
        ai_assert(master_bounds < inmesh.vertcnt.size());
        outer_polygon_it = begin + master_bounds;
    }
    else {
        for(iit = begin; iit != end; iit++) {
            // find the polygon with the largest area and take it as the outer bound.
            IfcVector3& n = normals[std::distance(begin,iit)];
            const IfcFloat area = n.SquareLength();
            if (area > area_outer_polygon) {
                area_outer_polygon = area;
                outer_polygon_it = iit;
            }
        }
    }

    ai_assert(outer_polygon_it != end);

    const size_t outer_polygon_size = *outer_polygon_it;
    const IfcVector3& master_normal = normals[std::distance(begin, outer_polygon_it)];

    // Generate fake openings to meet the interface for the quadrulate
    // algorithm. It boils down to generating small boxes given the
    // inner polygon and the surface normal of the outer contour.
    // It is important that we use the outer contour's normal because
    // this is the plane onto which the quadrulate algorithm will
    // project the entire mesh.
    std::vector<TempOpening> fake_openings;
    fake_openings.reserve(inmesh.vertcnt.size()-1);

    std::vector<IfcVector3>::const_iterator vit = inmesh.verts.begin(), outer_vit;

    for(iit = begin; iit != end; vit += *iit++) {
        if (iit == outer_polygon_it) {
            outer_vit = vit;
            continue;
        }

        // Filter degenerate polygons to keep them from causing trouble later on
        IfcVector3& n = normals[std::distance(begin,iit)];
        const IfcFloat area = n.SquareLength();
        if (area < 1e-5f) {
            IFCImporter::LogWarn("skipping degenerate polygon (ProcessPolygonBoundaries)");
            continue;
        }

        fake_openings.push_back(TempOpening());
        TempOpening& opening = fake_openings.back();

        opening.extrusionDir = master_normal;
        opening.solid = NULL;

        opening.profileMesh = std::make_shared<TempMesh>();
        opening.profileMesh->verts.reserve(*iit);
        opening.profileMesh->vertcnt.push_back(*iit);

        std::copy(vit, vit + *iit, std::back_inserter(opening.profileMesh->verts));
    }

    // fill a mesh with ONLY the main polygon
    TempMesh temp;
    temp.verts.reserve(outer_polygon_size);
    temp.vertcnt.push_back(static_cast<unsigned int>(outer_polygon_size));
    std::copy(outer_vit, outer_vit+outer_polygon_size,
        std::back_inserter(temp.verts));

    GenerateOpenings(fake_openings, normals, temp, false, false);
    result.Append(temp);
}

// ------------------------------------------------------------------------------------------------
void ProcessConnectedFaceSet(const IfcConnectedFaceSet& fset, TempMesh& result, ConversionData& conv)
{
    for(const IfcFace& face : fset.CfsFaces) {
        // size_t ob = -1, cnt = 0;
        TempMesh meshout;
        for(const IfcFaceBound& bound : face.Bounds) {

            if(const IfcPolyLoop* const polyloop = bound.Bound->ToPtr<IfcPolyLoop>()) {
                if(ProcessPolyloop(*polyloop, meshout,conv)) {

                    // The outer boundary is better determined by checking which
                    // polygon covers the largest area.

                    //if(bound.ToPtr<IfcFaceOuterBound>()) {
                    //  ob = cnt;
                    //}
                    //++cnt;

                }
            }
            else {
                IFCImporter::LogWarn("skipping unknown IfcFaceBound entity, type is " + bound.Bound->GetClassName());
                continue;
            }

            // And this, even though it is sometimes TRUE and sometimes FALSE,
            // does not really improve results.

            /*if(!IsTrue(bound.Orientation)) {
                size_t c = 0;
                for(unsigned int& c : meshout.vertcnt) {
                    std::reverse(result.verts.begin() + cnt,result.verts.begin() + cnt + c);
                    cnt += c;
                }
            }*/
        }
        ProcessPolygonBoundaries(result, meshout);
    }
}

// ------------------------------------------------------------------------------------------------
void ProcessRevolvedAreaSolid(const IfcRevolvedAreaSolid& solid, TempMesh& result, ConversionData& conv)
{
    TempMesh meshout;

    // first read the profile description
    if(!ProcessProfile(*solid.SweptArea,meshout,conv) || meshout.verts.size()<=1) {
        return;
    }

    IfcVector3 axis, pos;
    ConvertAxisPlacement(axis,pos,solid.Axis);

    IfcMatrix4 tb0,tb1;
    IfcMatrix4::Translation(pos,tb0);
    IfcMatrix4::Translation(-pos,tb1);

    const std::vector<IfcVector3>& in = meshout.verts;
    const size_t size=in.size();

    bool has_area = solid.SweptArea->ProfileType == "AREA" && size>2;
    const IfcFloat max_angle = solid.Angle*conv.angle_scale;
    if(std::fabs(max_angle) < 1e-3) {
        if(has_area) {
            result = meshout;
        }
        return;
    }

    const unsigned int cnt_segments = std::max(2u,static_cast<unsigned int>(conv.settings.cylindricalTessellation * std::fabs(max_angle)/AI_MATH_HALF_PI_F));
    const IfcFloat delta = max_angle/cnt_segments;

    has_area = has_area && std::fabs(max_angle) < AI_MATH_TWO_PI_F*0.99;

    result.verts.reserve(size*((cnt_segments+1)*4+(has_area?2:0)));
    result.vertcnt.reserve(size*cnt_segments+2);

    IfcMatrix4 rot;
    rot = tb0 * IfcMatrix4::Rotation(delta,axis,rot) * tb1;

    size_t base = 0;
    std::vector<IfcVector3>& out = result.verts;

    // dummy data to simplify later processing
    for(size_t i = 0; i < size; ++i) {
        out.insert(out.end(),4,in[i]);
    }

    for(unsigned int seg = 0; seg < cnt_segments; ++seg) {
        for(size_t i = 0; i < size; ++i) {
            const size_t next = (i+1)%size;

            result.vertcnt.push_back(4);
            const IfcVector3 base_0 = out[base+i*4+3],base_1 = out[base+next*4+3];

            out.push_back(base_0);
            out.push_back(base_1);
            out.push_back(rot*base_1);
            out.push_back(rot*base_0);
        }
        base += size*4;
    }

    out.erase(out.begin(),out.begin()+size*4);

    if(has_area) {
        // leave the triangulation of the profile area to the ear cutting
        // implementation in aiProcess_Triangulate - for now we just
        // feed in two huge polygons.
        base -= size*8;
        for(size_t i = size; i--; ) {
            out.push_back(out[base+i*4+3]);
        }
        for(size_t i = 0; i < size; ++i ) {
            out.push_back(out[i*4]);
        }
        result.vertcnt.push_back(static_cast<unsigned int>(size));
        result.vertcnt.push_back(static_cast<unsigned int>(size));
    }

    IfcMatrix4 trafo;
    ConvertAxisPlacement(trafo, solid.Position);

    result.Transform(trafo);
    IFCImporter::LogDebug("generate mesh procedurally by radial extrusion (IfcRevolvedAreaSolid)");
}



// ------------------------------------------------------------------------------------------------
void ProcessSweptDiskSolid(const IfcSweptDiskSolid solid, TempMesh& result, ConversionData& conv)
{
    const Curve* const curve = Curve::Convert(*solid.Directrix, conv);
    if(!curve) {
        IFCImporter::LogError("failed to convert Directrix curve (IfcSweptDiskSolid)");
        return;
    }

    const unsigned int cnt_segments = conv.settings.cylindricalTessellation;
    const IfcFloat deltaAngle = AI_MATH_TWO_PI/cnt_segments;

    const size_t samples = curve->EstimateSampleCount(solid.StartParam,solid.EndParam);

    result.verts.reserve(cnt_segments * samples * 4);
    result.vertcnt.reserve((cnt_segments - 1) * samples);

    std::vector<IfcVector3> points;
    points.reserve(cnt_segments * samples);

    TempMesh temp;
    curve->SampleDiscrete(temp,solid.StartParam,solid.EndParam);
    const std::vector<IfcVector3>& curve_points = temp.verts;

    if(curve_points.empty()) {
        IFCImporter::LogWarn("curve evaluation yielded no points (IfcSweptDiskSolid)");
        return;
    }

    IfcVector3 current = curve_points[0];
    IfcVector3 previous = current;
    IfcVector3 next;

    IfcVector3 startvec;
    startvec.x = 1.0f;
    startvec.y = 1.0f;
    startvec.z = 1.0f;

    unsigned int last_dir = 0;

    // generate circles at the sweep positions
    for(size_t i = 0; i < samples; ++i) {

        if(i != samples - 1) {
            next = curve_points[i + 1];
        }

        // get a direction vector reflecting the approximate curvature (i.e. tangent)
        IfcVector3 d = (current-previous) + (next-previous);

        d.Normalize();

        // figure out an arbitrary point q so that (p-q) * d = 0,
        // try to maximize ||(p-q)|| * ||(p_last-q_last)||
        IfcVector3 q;
        bool take_any = false;

        for (unsigned int i = 0; i < 2; ++i, take_any = true) {
            if ((last_dir == 0 || take_any) && std::abs(d.x) > 1e-6) {
                q.y = startvec.y;
                q.z = startvec.z;
                q.x = -(d.y * q.y + d.z * q.z) / d.x;
                last_dir = 0;
                break;
            }
            else if ((last_dir == 1 || take_any) && std::abs(d.y) > 1e-6) {
                q.x = startvec.x;
                q.z = startvec.z;
                q.y = -(d.x * q.x + d.z * q.z) / d.y;
                last_dir = 1;
                break;
            }
            else if ((last_dir == 2 && std::abs(d.z) > 1e-6) || take_any) {
                q.y = startvec.y;
                q.x = startvec.x;
                q.z = -(d.y * q.y + d.x * q.x) / d.z;
                last_dir = 2;
                break;
            }
        }

        q *= solid.Radius / q.Length();
        startvec = q;

        // generate a rotation matrix to rotate q around d
        IfcMatrix4 rot;
        IfcMatrix4::Rotation(deltaAngle,d,rot);

        for (unsigned int seg = 0; seg < cnt_segments; ++seg, q *= rot ) {
            points.push_back(q + current);
        }

        previous = current;
        current = next;
    }

    // make quads
    for(size_t i = 0; i < samples - 1; ++i) {

        const aiVector3D& this_start = points[ i * cnt_segments ];

        // locate corresponding point on next sample ring
        unsigned int best_pair_offset = 0;
        float best_distance_squared = 1e10f;
        for (unsigned int seg = 0; seg < cnt_segments; ++seg) {
            const aiVector3D& p = points[ (i+1) * cnt_segments + seg];
            const float l = (p-this_start).SquareLength();

            if(l < best_distance_squared) {
                best_pair_offset = seg;
                best_distance_squared = l;
            }
        }

        for (unsigned int seg = 0; seg < cnt_segments; ++seg) {

            result.verts.push_back(points[ i * cnt_segments + (seg % cnt_segments)]);
            result.verts.push_back(points[ i * cnt_segments + (seg + 1) % cnt_segments]);
            result.verts.push_back(points[ (i+1) * cnt_segments + ((seg + 1 + best_pair_offset) % cnt_segments)]);
            result.verts.push_back(points[ (i+1) * cnt_segments + ((seg + best_pair_offset) % cnt_segments)]);

            IfcVector3& v1 = *(result.verts.end()-1);
            IfcVector3& v2 = *(result.verts.end()-2);
            IfcVector3& v3 = *(result.verts.end()-3);
            IfcVector3& v4 = *(result.verts.end()-4);

            if (((v4-v3) ^ (v4-v1)) * (v4 - curve_points[i]) < 0.0f) {
                std::swap(v4, v1);
                std::swap(v3, v2);
            }

            result.vertcnt.push_back(4);
        }
    }

    IFCImporter::LogDebug("generate mesh procedurally by sweeping a disk along a curve (IfcSweptDiskSolid)");
}

// ------------------------------------------------------------------------------------------------
IfcMatrix3 DerivePlaneCoordinateSpace(const TempMesh& curmesh, bool& ok, IfcVector3& norOut)
{
    const std::vector<IfcVector3>& out = curmesh.verts;
    IfcMatrix3 m;

    ok = true;

    // The input "mesh" must be a single polygon
    const size_t s = out.size();
    assert(curmesh.vertcnt.size() == 1 && curmesh.vertcnt.back() == s);

    const IfcVector3 any_point = out[s-1];
    IfcVector3 nor;

    // The input polygon is arbitrarily shaped, therefore we might need some tries
    // until we find a suitable normal. Note that Newell's algorithm would give
    // a more robust result, but this variant also gives us a suitable first
    // axis for the 2D coordinate space on the polygon plane, exploiting the
    // fact that the input polygon is nearly always a quad.
    bool done = false;
    size_t i, j;
    for (i = 0; !done && i < s-2; done || ++i) {
        for (j = i+1; j < s-1; ++j) {
            nor = -((out[i]-any_point)^(out[j]-any_point));
            if(std::fabs(nor.Length()) > 1e-8f) {
                done = true;
                break;
            }
        }
    }

    if(!done) {
        ok = false;
        return m;
    }

    nor.Normalize();
    norOut = nor;

    IfcVector3 r = (out[i]-any_point);
    r.Normalize();

    //if(d) {
    //  *d = -any_point * nor;
    //}

    // Reconstruct orthonormal basis
    // XXX use Gram Schmidt for increased robustness
    IfcVector3 u = r ^ nor;
    u.Normalize();

    m.a1 = r.x;
    m.a2 = r.y;
    m.a3 = r.z;

    m.b1 = u.x;
    m.b2 = u.y;
    m.b3 = u.z;

    m.c1 = -nor.x;
    m.c2 = -nor.y;
    m.c3 = -nor.z;

    return m;
}

// Extrudes the given polygon along the direction, converts it into an opening or applies all openings as necessary.
void ProcessExtrudedArea(const IfcExtrudedAreaSolid& solid, const TempMesh& curve,
    const IfcVector3& extrusionDir, TempMesh& result, ConversionData &conv, bool collect_openings)
{
    // Outline: 'curve' is now a list of vertex points forming the underlying profile, extrude along the given axis,
    // forming new triangles.
    const bool has_area = solid.SweptArea->ProfileType == "AREA" && curve.verts.size() > 2;
    if( solid.Depth < 1e-6 ) {
        if( has_area ) {
            result.Append(curve);
        }
        return;
    }

    result.verts.reserve(curve.verts.size()*(has_area ? 4 : 2));
    result.vertcnt.reserve(curve.verts.size() + 2);
    std::vector<IfcVector3> in = curve.verts;

    // First step: transform all vertices into the target coordinate space
    IfcMatrix4 trafo;
    ConvertAxisPlacement(trafo, solid.Position);

    IfcVector3 vmin, vmax;
    MinMaxChooser<IfcVector3>()(vmin, vmax);
    for(IfcVector3& v : in) {
        v *= trafo;

        vmin = std::min(vmin, v);
        vmax = std::max(vmax, v);
    }

    vmax -= vmin;
    const IfcFloat diag = vmax.Length();
    IfcVector3 dir = IfcMatrix3(trafo) * extrusionDir;

    // reverse profile polygon if it's winded in the wrong direction in relation to the extrusion direction
    IfcVector3 profileNormal = TempMesh::ComputePolygonNormal(in.data(), in.size());
    if( profileNormal * dir < 0.0 )
        std::reverse(in.begin(), in.end());

    std::vector<IfcVector3> nors;
    const bool openings = !!conv.apply_openings && conv.apply_openings->size();

    // Compute the normal vectors for all opening polygons as a prerequisite
    // to TryAddOpenings_Poly2Tri()
    // XXX this belongs into the aforementioned function
    if( openings ) {

        if( !conv.settings.useCustomTriangulation ) {
            // it is essential to apply the openings in the correct spatial order. The direction
            // doesn't matter, but we would screw up if we started with e.g. a door in between
            // two windows.
            std::sort(conv.apply_openings->begin(), conv.apply_openings->end(), TempOpening::DistanceSorter(in[0]));
        }

        nors.reserve(conv.apply_openings->size());
        for(TempOpening& t : *conv.apply_openings) {
            TempMesh& bounds = *t.profileMesh.get();

            if( bounds.verts.size() <= 2 ) {
                nors.push_back(IfcVector3());
                continue;
            }
            nors.push_back(((bounds.verts[2] - bounds.verts[0]) ^ (bounds.verts[1] - bounds.verts[0])).Normalize());
        }
    }


    TempMesh temp;
    TempMesh& curmesh = openings ? temp : result;
    std::vector<IfcVector3>& out = curmesh.verts;

    size_t sides_with_openings = 0;
    for( size_t i = 0; i < in.size(); ++i ) {
        const size_t next = (i + 1) % in.size();

        curmesh.vertcnt.push_back(4);

        out.push_back(in[i]);
        out.push_back(in[next]);
        out.push_back(in[next] + dir);
        out.push_back(in[i] + dir);

        if( openings ) {
            if( (in[i] - in[next]).Length() > diag * 0.1 && GenerateOpenings(*conv.apply_openings, nors, temp, true, true, dir) ) {
                ++sides_with_openings;
            }

            result.Append(temp);
            temp.Clear();
        }
    }

    if( openings ) {
        for(TempOpening& opening : *conv.apply_openings) {
            if( !opening.wallPoints.empty() ) {
                IFCImporter::LogError("failed to generate all window caps");
            }
            opening.wallPoints.clear();
        }
    }

    size_t sides_with_v_openings = 0;
    if( has_area ) {

        for( size_t n = 0; n < 2; ++n ) {
            if( n > 0 ) {
                for( size_t i = 0; i < in.size(); ++i )
                    out.push_back(in[i] + dir);
            }
            else {
                for( size_t i = in.size(); i--; )
                    out.push_back(in[i]);
            }

            curmesh.vertcnt.push_back(static_cast<unsigned int>(in.size()));
            if( openings && in.size() > 2 ) {
                if( GenerateOpenings(*conv.apply_openings, nors, temp, true, true, dir) ) {
                    ++sides_with_v_openings;
                }

                result.Append(temp);
                temp.Clear();
            }
        }
    }

    if( openings && ((sides_with_openings == 1 && sides_with_openings) || (sides_with_v_openings == 2 && sides_with_v_openings)) ) {
        IFCImporter::LogWarn("failed to resolve all openings, presumably their topology is not supported by Assimp");
    }

    IFCImporter::LogDebug("generate mesh procedurally by extrusion (IfcExtrudedAreaSolid)");

    // If this is an opening element, store both the extruded mesh and the 2D profile mesh
    // it was created from. Return an empty mesh to the caller.
    if( collect_openings && !result.IsEmpty() ) {
        ai_assert(conv.collect_openings);
        std::shared_ptr<TempMesh> profile = std::shared_ptr<TempMesh>(new TempMesh());
        profile->Swap(result);

        std::shared_ptr<TempMesh> profile2D = std::shared_ptr<TempMesh>(new TempMesh());
        profile2D->verts.insert(profile2D->verts.end(), in.begin(), in.end());
        profile2D->vertcnt.push_back(static_cast<unsigned int>(in.size()));
        conv.collect_openings->push_back(TempOpening(&solid, dir, profile, profile2D));

        ai_assert(result.IsEmpty());
    }
}

// ------------------------------------------------------------------------------------------------
void ProcessExtrudedAreaSolid(const IfcExtrudedAreaSolid& solid, TempMesh& result,
    ConversionData& conv, bool collect_openings)
{
    TempMesh meshout;

    // First read the profile description.
    if(!ProcessProfile(*solid.SweptArea,meshout,conv) || meshout.verts.size()<=1) {
        return;
    }

    IfcVector3 dir;
    ConvertDirection(dir,solid.ExtrudedDirection);
    dir *= solid.Depth;

    // Some profiles bring their own holes, for which we need to provide a container. This all is somewhat backwards,
    // and there's still so many corner cases uncovered - we really need a generic solution to all of this hole carving.
    std::vector<TempOpening> fisherPriceMyFirstOpenings;
    std::vector<TempOpening>* oldApplyOpenings = conv.apply_openings;
    if( const IfcArbitraryProfileDefWithVoids* const cprofile = solid.SweptArea->ToPtr<IfcArbitraryProfileDefWithVoids>() ) {
        if( !cprofile->InnerCurves.empty() ) {
            // read all inner curves and extrude them to form proper openings.
            std::vector<TempOpening>* oldCollectOpenings = conv.collect_openings;
            conv.collect_openings = &fisherPriceMyFirstOpenings;

            for(const IfcCurve* curve : cprofile->InnerCurves) {
                TempMesh curveMesh, tempMesh;
                ProcessCurve(*curve, curveMesh, conv);
                ProcessExtrudedArea(solid, curveMesh, dir, tempMesh, conv, true);
            }
            // and then apply those to the geometry we're about to generate
            conv.apply_openings = conv.collect_openings;
            conv.collect_openings = oldCollectOpenings;
        }
    }

    ProcessExtrudedArea(solid, meshout, dir, result, conv, collect_openings);
    conv.apply_openings = oldApplyOpenings;
}

// ------------------------------------------------------------------------------------------------
void ProcessSweptAreaSolid(const IfcSweptAreaSolid& swept, TempMesh& meshout,
    ConversionData& conv)
{
    if(const IfcExtrudedAreaSolid* const solid = swept.ToPtr<IfcExtrudedAreaSolid>()) {
        ProcessExtrudedAreaSolid(*solid,meshout,conv, !!conv.collect_openings);
    }
    else if(const IfcRevolvedAreaSolid* const rev = swept.ToPtr<IfcRevolvedAreaSolid>()) {
        ProcessRevolvedAreaSolid(*rev,meshout,conv);
    }
    else {
        IFCImporter::LogWarn("skipping unknown IfcSweptAreaSolid entity, type is " + swept.GetClassName());
    }
}

// ------------------------------------------------------------------------------------------------
bool ProcessGeometricItem(const IfcRepresentationItem& geo, unsigned int matid, std::vector<unsigned int>& mesh_indices,
    ConversionData& conv)
{
    bool fix_orientation = false;
    std::shared_ptr< TempMesh > meshtmp = std::make_shared<TempMesh>();
    if(const IfcShellBasedSurfaceModel* shellmod = geo.ToPtr<IfcShellBasedSurfaceModel>()) {
        for(std::shared_ptr<const IfcShell> shell :shellmod->SbsmBoundary) {
            try {
                const EXPRESS::ENTITY& e = shell->To<ENTITY>();
                const IfcConnectedFaceSet& fs = conv.db.MustGetObject(e).To<IfcConnectedFaceSet>();

                ProcessConnectedFaceSet(fs,*meshtmp.get(),conv);
            }
            catch(std::bad_cast&) {
                IFCImporter::LogWarn("unexpected type error, IfcShell ought to inherit from IfcConnectedFaceSet");
            }
        }
        fix_orientation = true;
    }
    else  if(const IfcConnectedFaceSet* fset = geo.ToPtr<IfcConnectedFaceSet>()) {
        ProcessConnectedFaceSet(*fset,*meshtmp.get(),conv);
        fix_orientation = true;
    }
    else  if(const IfcSweptAreaSolid* swept = geo.ToPtr<IfcSweptAreaSolid>()) {
        ProcessSweptAreaSolid(*swept,*meshtmp.get(),conv);
    }
    else  if(const IfcSweptDiskSolid* disk = geo.ToPtr<IfcSweptDiskSolid>()) {
        ProcessSweptDiskSolid(*disk,*meshtmp.get(),conv);
    }
    else if(const IfcManifoldSolidBrep* brep = geo.ToPtr<IfcManifoldSolidBrep>()) {
        ProcessConnectedFaceSet(brep->Outer,*meshtmp.get(),conv);
        fix_orientation = true;
    }
    else if(const IfcFaceBasedSurfaceModel* surf = geo.ToPtr<IfcFaceBasedSurfaceModel>()) {
        for(const IfcConnectedFaceSet& fc : surf->FbsmFaces) {
            ProcessConnectedFaceSet(fc,*meshtmp.get(),conv);
        }
        fix_orientation = true;
    }
    else  if(const IfcBooleanResult* boolean = geo.ToPtr<IfcBooleanResult>()) {
        ProcessBoolean(*boolean,*meshtmp.get(),conv);
    }
    else if(geo.ToPtr<IfcBoundingBox>()) {
        // silently skip over bounding boxes
        return false;
    }
    else {
        IFCImporter::LogWarn("skipping unknown IfcGeometricRepresentationItem entity, type is " + geo.GetClassName());
        return false;
    }

    // Do we just collect openings for a parent element (i.e. a wall)?
    // In such a case, we generate the polygonal mesh as usual,
    // but attach it to a TempOpening instance which will later be applied
    // to the wall it pertains to.

    // Note: swep area solids are added in ProcessExtrudedAreaSolid(),
    // which returns an empty mesh.
    if(conv.collect_openings) {
        if (!meshtmp->IsEmpty()) {
            conv.collect_openings->push_back(TempOpening(geo.ToPtr<IfcSolidModel>(),
                IfcVector3(0,0,0),
                meshtmp,
                std::shared_ptr<TempMesh>()));
        }
        return true;
    }

    if (meshtmp->IsEmpty()) {
        return false;
    }

    meshtmp->RemoveAdjacentDuplicates();
    meshtmp->RemoveDegenerates();

    if(fix_orientation) {
//      meshtmp->FixupFaceOrientation();
    }

    aiMesh* const mesh = meshtmp->ToMesh();
    if(mesh) {
        mesh->mMaterialIndex = matid;
        mesh_indices.push_back(static_cast<unsigned int>(conv.meshes.size()));
        conv.meshes.push_back(mesh);
        return true;
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
void AssignAddedMeshes(std::vector<unsigned int>& mesh_indices,aiNode* nd,
    ConversionData& /*conv*/)
{
    if (!mesh_indices.empty()) {

        // make unique
        std::sort(mesh_indices.begin(),mesh_indices.end());
        std::vector<unsigned int>::iterator it_end = std::unique(mesh_indices.begin(),mesh_indices.end());

        nd->mNumMeshes = static_cast<unsigned int>(std::distance(mesh_indices.begin(),it_end));

        nd->mMeshes = new unsigned int[nd->mNumMeshes];
        for(unsigned int i = 0; i < nd->mNumMeshes; ++i) {
            nd->mMeshes[i] = mesh_indices[i];
        }
    }
}

// ------------------------------------------------------------------------------------------------
bool TryQueryMeshCache(const IfcRepresentationItem& item,
    std::vector<unsigned int>& mesh_indices, unsigned int mat_index,
    ConversionData& conv)
{
    ConversionData::MeshCacheIndex idx(&item, mat_index);
    ConversionData::MeshCache::const_iterator it = conv.cached_meshes.find(idx);
    if (it != conv.cached_meshes.end()) {
        std::copy((*it).second.begin(),(*it).second.end(),std::back_inserter(mesh_indices));
        return true;
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
void PopulateMeshCache(const IfcRepresentationItem& item,
    const std::vector<unsigned int>& mesh_indices, unsigned int mat_index,
    ConversionData& conv)
{
    ConversionData::MeshCacheIndex idx(&item, mat_index);
    conv.cached_meshes[idx] = mesh_indices;
}

// ------------------------------------------------------------------------------------------------
bool ProcessRepresentationItem(const IfcRepresentationItem& item, unsigned int matid,
    std::vector<unsigned int>& mesh_indices,
    ConversionData& conv)
{
    // determine material
    unsigned int localmatid = ProcessMaterials(item.GetID(), matid, conv, true);

    if (!TryQueryMeshCache(item,mesh_indices,localmatid,conv)) {
        if(ProcessGeometricItem(item,localmatid,mesh_indices,conv)) {
            if(mesh_indices.size()) {
                PopulateMeshCache(item,mesh_indices,localmatid,conv);
            }
        }
        else return false;
    }
    return true;
}


} // ! IFC
} // ! Assimp

#endif
