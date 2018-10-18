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

/** @file  IFCOpenings.cpp
 *  @brief Implements a subset of Ifc CSG operations for pouring
  *    holes for windows and doors into walls.
 */


#ifndef ASSIMP_BUILD_NO_IFC_IMPORTER
#include "IFCUtil.h"
#include "code/PolyTools.h"
#include "code/ProcessHelper.h"

#include "../contrib/poly2tri/poly2tri/poly2tri.h"
#include "../contrib/clipper/clipper.hpp"

#include <iterator>

namespace Assimp {
    namespace IFC {

        using ClipperLib::ulong64;
        // XXX use full -+ range ...
        const ClipperLib::long64 max_ulong64 = 1518500249; // clipper.cpp / hiRange var

        //#define to_int64(p)  (static_cast<ulong64>( std::max( 0., std::min( static_cast<IfcFloat>((p)), 1.) ) * max_ulong64 ))
#define to_int64(p)  (static_cast<ulong64>(static_cast<IfcFloat>((p) ) * max_ulong64 ))
#define from_int64(p) (static_cast<IfcFloat>((p)) / max_ulong64)
#define one_vec (IfcVector2(static_cast<IfcFloat>(1.0),static_cast<IfcFloat>(1.0)))


        // fallback method to generate wall openings
        bool TryAddOpenings_Poly2Tri(const std::vector<TempOpening>& openings,const std::vector<IfcVector3>& nors,
            TempMesh& curmesh);


typedef std::pair< IfcVector2, IfcVector2 > BoundingBox;
typedef std::map<IfcVector2,size_t,XYSorter> XYSortedField;


// ------------------------------------------------------------------------------------------------
void QuadrifyPart(const IfcVector2& pmin, const IfcVector2& pmax, XYSortedField& field,
    const std::vector< BoundingBox >& bbs,
    std::vector<IfcVector2>& out)
{
    if (!(pmin.x-pmax.x) || !(pmin.y-pmax.y)) {
        return;
    }

    IfcFloat xs = 1e10, xe = 1e10;
    bool found = false;

    // Search along the x-axis until we find an opening
    XYSortedField::iterator start = field.begin();
    for(; start != field.end(); ++start) {
        const BoundingBox& bb = bbs[(*start).second];
        if(bb.first.x >= pmax.x) {
            break;
        }

        if (bb.second.x > pmin.x && bb.second.y > pmin.y && bb.first.y < pmax.y) {
            xs = bb.first.x;
            xe = bb.second.x;
            found = true;
            break;
        }
    }

    if (!found) {
        // the rectangle [pmin,pend] is opaque, fill it
        out.push_back(pmin);
        out.push_back(IfcVector2(pmin.x,pmax.y));
        out.push_back(pmax);
        out.push_back(IfcVector2(pmax.x,pmin.y));
        return;
    }

    xs = std::max(pmin.x,xs);
    xe = std::min(pmax.x,xe);

    // see if there's an offset to fill at the top of our quad
    if (xs - pmin.x) {
        out.push_back(pmin);
        out.push_back(IfcVector2(pmin.x,pmax.y));
        out.push_back(IfcVector2(xs,pmax.y));
        out.push_back(IfcVector2(xs,pmin.y));
    }

    // search along the y-axis for all openings that overlap xs and our quad
    IfcFloat ylast = pmin.y;
    found = false;
    for(; start != field.end(); ++start) {
        const BoundingBox& bb = bbs[(*start).second];
        if (bb.first.x > xs || bb.first.y >= pmax.y) {
            break;
        }

        if (bb.second.y > ylast) {

            found = true;
            const IfcFloat ys = std::max(bb.first.y,pmin.y), ye = std::min(bb.second.y,pmax.y);
            if (ys - ylast > 0.0f) {
                QuadrifyPart( IfcVector2(xs,ylast), IfcVector2(xe,ys) ,field,bbs,out);
            }

            // the following are the window vertices

            /*wnd.push_back(IfcVector2(xs,ys));
            wnd.push_back(IfcVector2(xs,ye));
            wnd.push_back(IfcVector2(xe,ye));
            wnd.push_back(IfcVector2(xe,ys));*/
            ylast = ye;
        }
    }
    if (!found) {
        // the rectangle [pmin,pend] is opaque, fill it
        out.push_back(IfcVector2(xs,pmin.y));
        out.push_back(IfcVector2(xs,pmax.y));
        out.push_back(IfcVector2(xe,pmax.y));
        out.push_back(IfcVector2(xe,pmin.y));
        return;
    }
    if (ylast < pmax.y) {
        QuadrifyPart( IfcVector2(xs,ylast), IfcVector2(xe,pmax.y) ,field,bbs,out);
    }

    // now for the whole rest
    if (pmax.x-xe) {
        QuadrifyPart(IfcVector2(xe,pmin.y), pmax ,field,bbs,out);
    }
}

typedef std::vector<IfcVector2> Contour;
typedef std::vector<bool> SkipList; // should probably use int for performance reasons

struct ProjectedWindowContour
{
    Contour contour;
    BoundingBox bb;
    SkipList skiplist;
    bool is_rectangular;


    ProjectedWindowContour(const Contour& contour, const BoundingBox& bb, bool is_rectangular)
        : contour(contour)
        , bb(bb)
        , is_rectangular(is_rectangular)
    {}


    bool IsInvalid() const {
        return contour.empty();
    }

    void FlagInvalid() {
        contour.clear();
    }

    void PrepareSkiplist() {
        skiplist.resize(contour.size(),false);
    }
};

typedef std::vector< ProjectedWindowContour > ContourVector;

// ------------------------------------------------------------------------------------------------
bool BoundingBoxesOverlapping( const BoundingBox &ibb, const BoundingBox &bb )
{
    // count the '=' case as non-overlapping but as adjacent to each other
    return ibb.first.x < bb.second.x && ibb.second.x > bb.first.x &&
        ibb.first.y < bb.second.y && ibb.second.y > bb.first.y;
}

// ------------------------------------------------------------------------------------------------
bool IsDuplicateVertex(const IfcVector2& vv, const std::vector<IfcVector2>& temp_contour)
{
    // sanity check for duplicate vertices
    for(const IfcVector2& cp : temp_contour) {
        if ((cp-vv).SquareLength() < 1e-5f) {
            return true;
        }
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
void ExtractVerticesFromClipper(const ClipperLib::Polygon& poly, std::vector<IfcVector2>& temp_contour,
    bool filter_duplicates = false)
{
    temp_contour.clear();
    for(const ClipperLib::IntPoint& point : poly) {
        IfcVector2 vv = IfcVector2( from_int64(point.X), from_int64(point.Y));
        vv = std::max(vv,IfcVector2());
        vv = std::min(vv,one_vec);

        if (!filter_duplicates || !IsDuplicateVertex(vv, temp_contour)) {
            temp_contour.push_back(vv);
        }
    }
}

// ------------------------------------------------------------------------------------------------
BoundingBox GetBoundingBox(const ClipperLib::Polygon& poly)
{
    IfcVector2 newbb_min, newbb_max;
    MinMaxChooser<IfcVector2>()(newbb_min, newbb_max);

    for(const ClipperLib::IntPoint& point : poly) {
        IfcVector2 vv = IfcVector2( from_int64(point.X), from_int64(point.Y));

        // sanity rounding
        vv = std::max(vv,IfcVector2());
        vv = std::min(vv,one_vec);

        newbb_min = std::min(newbb_min,vv);
        newbb_max = std::max(newbb_max,vv);
    }
    return BoundingBox(newbb_min, newbb_max);
}

// ------------------------------------------------------------------------------------------------
void InsertWindowContours(const ContourVector& contours,
    const std::vector<TempOpening>& /*openings*/,
    TempMesh& curmesh)
{
    // fix windows - we need to insert the real, polygonal shapes into the quadratic holes that we have now
    for(size_t i = 0; i < contours.size();++i) {
        const BoundingBox& bb = contours[i].bb;
        const std::vector<IfcVector2>& contour = contours[i].contour;
        if(contour.empty()) {
            continue;
        }

        // check if we need to do it at all - many windows just fit perfectly into their quadratic holes,
        // i.e. their contours *are* already their bounding boxes.
        if (contour.size() == 4) {
            std::set<IfcVector2,XYSorter> verts;
            for(size_t n = 0; n < 4; ++n) {
                verts.insert(contour[n]);
            }
            const std::set<IfcVector2,XYSorter>::const_iterator end = verts.end();
            if (verts.find(bb.first)!=end && verts.find(bb.second)!=end
                && verts.find(IfcVector2(bb.first.x,bb.second.y))!=end
                && verts.find(IfcVector2(bb.second.x,bb.first.y))!=end
                ) {
                    continue;
            }
        }

        const IfcFloat diag = (bb.first-bb.second).Length();
        const IfcFloat epsilon = diag/1000.f;

        // walk through all contour points and find those that lie on the BB corner
        size_t last_hit = -1, very_first_hit = -1;
        IfcVector2 edge;
        for(size_t n = 0, e=0, size = contour.size();; n=(n+1)%size, ++e) {

            // sanity checking
            if (e == size*2) {
                IFCImporter::LogError("encountered unexpected topology while generating window contour");
                break;
            }

            const IfcVector2& v = contour[n];

            bool hit = false;
            if (std::fabs(v.x-bb.first.x)<epsilon) {
                edge.x = bb.first.x;
                hit = true;
            }
            else if (std::fabs(v.x-bb.second.x)<epsilon) {
                edge.x = bb.second.x;
                hit = true;
            }

            if (std::fabs(v.y-bb.first.y)<epsilon) {
                edge.y = bb.first.y;
                hit = true;
            }
            else if (std::fabs(v.y-bb.second.y)<epsilon) {
                edge.y = bb.second.y;
                hit = true;
            }

            if (hit) {
                if (last_hit != (size_t)-1) {

                    const size_t old = curmesh.mVerts.size();
                    size_t cnt = last_hit > n ? size-(last_hit-n) : n-last_hit;
                    for(size_t a = last_hit, e = 0; e <= cnt; a=(a+1)%size, ++e) {
                        // hack: this is to fix cases where opening contours are self-intersecting.
                        // Clipper doesn't produce such polygons, but as soon as we're back in
                        // our brave new floating-point world, very small distances are consumed
                        // by the maximum available precision, leading to self-intersecting
                        // polygons. This fix makes concave windows fail even worse, but
                        // anyway, fail is fail.
                        if ((contour[a] - edge).SquareLength() > diag*diag*0.7) {
                            continue;
                        }
                        curmesh.mVerts.push_back(IfcVector3(contour[a].x, contour[a].y, 0.0f));
                    }

                    if (edge != contour[last_hit]) {

                        IfcVector2 corner = edge;

                        if (std::fabs(contour[last_hit].x-bb.first.x)<epsilon) {
                            corner.x = bb.first.x;
                        }
                        else if (std::fabs(contour[last_hit].x-bb.second.x)<epsilon) {
                            corner.x = bb.second.x;
                        }

                        if (std::fabs(contour[last_hit].y-bb.first.y)<epsilon) {
                            corner.y = bb.first.y;
                        }
                        else if (std::fabs(contour[last_hit].y-bb.second.y)<epsilon) {
                            corner.y = bb.second.y;
                        }

                        curmesh.mVerts.push_back(IfcVector3(corner.x, corner.y, 0.0f));
                    }
                    else if (cnt == 1) {
                        // avoid degenerate polygons (also known as lines or points)
                        curmesh.mVerts.erase(curmesh.mVerts.begin()+old,curmesh.mVerts.end());
                    }

                    if (const size_t d = curmesh.mVerts.size()-old) {
                        curmesh.mVertcnt.push_back(static_cast<unsigned int>(d));
                        std::reverse(curmesh.mVerts.rbegin(),curmesh.mVerts.rbegin()+d);
                    }
                    if (n == very_first_hit) {
                        break;
                    }
                }
                else {
                    very_first_hit = n;
                }

                last_hit = n;
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------
void MergeWindowContours (const std::vector<IfcVector2>& a,
    const std::vector<IfcVector2>& b,
    ClipperLib::ExPolygons& out)
{
    out.clear();

    ClipperLib::Clipper clipper;
    ClipperLib::Polygon clip;

    for(const IfcVector2& pip : a) {
        clip.push_back(ClipperLib::IntPoint(  to_int64(pip.x), to_int64(pip.y) ));
    }

    if (ClipperLib::Orientation(clip)) {
        std::reverse(clip.begin(), clip.end());
    }

    clipper.AddPolygon(clip, ClipperLib::ptSubject);
    clip.clear();

    for(const IfcVector2& pip : b) {
        clip.push_back(ClipperLib::IntPoint(  to_int64(pip.x), to_int64(pip.y) ));
    }

    if (ClipperLib::Orientation(clip)) {
        std::reverse(clip.begin(), clip.end());
    }

    clipper.AddPolygon(clip, ClipperLib::ptSubject);
    clipper.Execute(ClipperLib::ctUnion, out,ClipperLib::pftNonZero,ClipperLib::pftNonZero);
}

// ------------------------------------------------------------------------------------------------
// Subtract a from b
void MakeDisjunctWindowContours (const std::vector<IfcVector2>& a,
    const std::vector<IfcVector2>& b,
    ClipperLib::ExPolygons& out)
{
    out.clear();

    ClipperLib::Clipper clipper;
    ClipperLib::Polygon clip;

    for(const IfcVector2& pip : a) {
        clip.push_back(ClipperLib::IntPoint(  to_int64(pip.x), to_int64(pip.y) ));
    }

    if (ClipperLib::Orientation(clip)) {
        std::reverse(clip.begin(), clip.end());
    }

    clipper.AddPolygon(clip, ClipperLib::ptClip);
    clip.clear();

    for(const IfcVector2& pip : b) {
        clip.push_back(ClipperLib::IntPoint(  to_int64(pip.x), to_int64(pip.y) ));
    }

    if (ClipperLib::Orientation(clip)) {
        std::reverse(clip.begin(), clip.end());
    }

    clipper.AddPolygon(clip, ClipperLib::ptSubject);
    clipper.Execute(ClipperLib::ctDifference, out,ClipperLib::pftNonZero,ClipperLib::pftNonZero);
}

// ------------------------------------------------------------------------------------------------
void CleanupWindowContour(ProjectedWindowContour& window)
{
    std::vector<IfcVector2> scratch;
    std::vector<IfcVector2>& contour = window.contour;

    ClipperLib::Polygon subject;
    ClipperLib::Clipper clipper;
    ClipperLib::ExPolygons clipped;

    for(const IfcVector2& pip : contour) {
        subject.push_back(ClipperLib::IntPoint(  to_int64(pip.x), to_int64(pip.y) ));
    }

    clipper.AddPolygon(subject,ClipperLib::ptSubject);
    clipper.Execute(ClipperLib::ctUnion,clipped,ClipperLib::pftNonZero,ClipperLib::pftNonZero);

    // This should yield only one polygon or something went wrong
    if (clipped.size() != 1) {

        // Empty polygon? drop the contour altogether
        if(clipped.empty()) {
            IFCImporter::LogError("error during polygon clipping, window contour is degenerate");
            window.FlagInvalid();
            return;
        }

        // Else: take the first only
        IFCImporter::LogError("error during polygon clipping, window contour is not convex");
    }

    ExtractVerticesFromClipper(clipped[0].outer, scratch);
    // Assume the bounding box doesn't change during this operation
}

// ------------------------------------------------------------------------------------------------
void CleanupWindowContours(ContourVector& contours)
{
    // Use PolyClipper to clean up window contours
    try {
        for(ProjectedWindowContour& window : contours) {
            CleanupWindowContour(window);
        }
    }
    catch (const char* sx) {
        IFCImporter::LogError("error during polygon clipping, window shape may be wrong: (Clipper: "
            + std::string(sx) + ")");
    }
}

// ------------------------------------------------------------------------------------------------
void CleanupOuterContour(const std::vector<IfcVector2>& contour_flat, TempMesh& curmesh)
{
    std::vector<IfcVector3> vold;
    std::vector<unsigned int> iold;

    vold.reserve(curmesh.mVerts.size());
    iold.reserve(curmesh.mVertcnt.size());

    // Fix the outer contour using polyclipper
    try {

        ClipperLib::Polygon subject;
        ClipperLib::Clipper clipper;
        ClipperLib::ExPolygons clipped;

        ClipperLib::Polygon clip;
        clip.reserve(contour_flat.size());
        for(const IfcVector2& pip : contour_flat) {
            clip.push_back(ClipperLib::IntPoint(  to_int64(pip.x), to_int64(pip.y) ));
        }

        if (!ClipperLib::Orientation(clip)) {
            std::reverse(clip.begin(), clip.end());
        }

        // We need to run polyclipper on every single polygon -- we can't run it one all
        // of them at once or it would merge them all together which would undo all
        // previous steps
        subject.reserve(4);
        size_t index = 0;
        size_t countdown = 0;
        for(const IfcVector3& pip : curmesh.mVerts) {
            if (!countdown) {
                countdown = curmesh.mVertcnt[index++];
                if (!countdown) {
                    continue;
                }
            }
            subject.push_back(ClipperLib::IntPoint(  to_int64(pip.x), to_int64(pip.y) ));
            if (--countdown == 0) {
                if (!ClipperLib::Orientation(subject)) {
                    std::reverse(subject.begin(), subject.end());
                }

                clipper.AddPolygon(subject,ClipperLib::ptSubject);
                clipper.AddPolygon(clip,ClipperLib::ptClip);

                clipper.Execute(ClipperLib::ctIntersection,clipped,ClipperLib::pftNonZero,ClipperLib::pftNonZero);

                for(const ClipperLib::ExPolygon& ex : clipped) {
                    iold.push_back(static_cast<unsigned int>(ex.outer.size()));
                    for(const ClipperLib::IntPoint& point : ex.outer) {
                        vold.push_back(IfcVector3(
                            from_int64(point.X),
                            from_int64(point.Y),
                            0.0f));
                    }
                }

                subject.clear();
                clipped.clear();
                clipper.Clear();
            }
        }
    }
    catch (const char* sx) {
        IFCImporter::LogError("Ifc: error during polygon clipping, wall contour line may be wrong: (Clipper: "
            + std::string(sx) + ")");

        return;
    }

    // swap data arrays
    std::swap(vold,curmesh.mVerts);
    std::swap(iold,curmesh.mVertcnt);
}

typedef std::vector<TempOpening*> OpeningRefs;
typedef std::vector<OpeningRefs > OpeningRefVector;

typedef std::vector<std::pair<
    ContourVector::const_iterator,
    Contour::const_iterator>
> ContourRefVector;

// ------------------------------------------------------------------------------------------------
bool BoundingBoxesAdjacent(const BoundingBox& bb, const BoundingBox& ibb)
{
    // TODO: I'm pretty sure there is a much more compact way to check this
    const IfcFloat epsilon = 1e-5f;
    return  (std::fabs(bb.second.x - ibb.first.x) < epsilon && bb.first.y <= ibb.second.y && bb.second.y >= ibb.first.y) ||
        (std::fabs(bb.first.x - ibb.second.x) < epsilon && ibb.first.y <= bb.second.y && ibb.second.y >= bb.first.y) ||
        (std::fabs(bb.second.y - ibb.first.y) < epsilon && bb.first.x <= ibb.second.x && bb.second.x >= ibb.first.x) ||
        (std::fabs(bb.first.y - ibb.second.y) < epsilon && ibb.first.x <= bb.second.x && ibb.second.x >= bb.first.x);
}

// ------------------------------------------------------------------------------------------------
// Check if m0,m1 intersects n0,n1 assuming same ordering of the points in the line segments
// output the intersection points on n0,n1
bool IntersectingLineSegments(const IfcVector2& n0, const IfcVector2& n1,
    const IfcVector2& m0, const IfcVector2& m1,
    IfcVector2& out0, IfcVector2& out1)
{
    const IfcVector2 n0_to_n1 = n1 - n0;

    const IfcVector2 n0_to_m0 = m0 - n0;
    const IfcVector2 n1_to_m1 = m1 - n1;

    const IfcVector2 n0_to_m1 = m1 - n0;

    const IfcFloat e = 1e-5f;
    const IfcFloat smalle = 1e-9f;

    static const IfcFloat inf = std::numeric_limits<IfcFloat>::infinity();

    if (!(n0_to_m0.SquareLength() < e*e || std::fabs(n0_to_m0 * n0_to_n1) / (n0_to_m0.Length() * n0_to_n1.Length()) > 1-1e-5 )) {
        return false;
    }

    if (!(n1_to_m1.SquareLength() < e*e || std::fabs(n1_to_m1 * n0_to_n1) / (n1_to_m1.Length() * n0_to_n1.Length()) > 1-1e-5 )) {
        return false;
    }

    IfcFloat s0;
    IfcFloat s1;

    // pick the axis with the higher absolute difference so the result
    // is more accurate. Since we cannot guarantee that the axis with
    // the higher absolute difference is big enough as to avoid
    // divisions by zero, the case 0/0 ~ infinity is detected and
    // handled separately.
    if(std::fabs(n0_to_n1.x) > std::fabs(n0_to_n1.y)) {
        s0 = n0_to_m0.x / n0_to_n1.x;
        s1 = n0_to_m1.x / n0_to_n1.x;

        if (std::fabs(s0) == inf && std::fabs(n0_to_m0.x) < smalle) {
            s0 = 0.;
        }
        if (std::fabs(s1) == inf && std::fabs(n0_to_m1.x) < smalle) {
            s1 = 0.;
        }
    }
    else {
        s0 = n0_to_m0.y / n0_to_n1.y;
        s1 = n0_to_m1.y / n0_to_n1.y;

        if (std::fabs(s0) == inf && std::fabs(n0_to_m0.y) < smalle) {
            s0 = 0.;
        }
        if (std::fabs(s1) == inf && std::fabs(n0_to_m1.y) < smalle) {
            s1 = 0.;
        }
    }

    if (s1 < s0) {
        std::swap(s1,s0);
    }

    s0 = std::max(0.0,s0);
    s1 = std::max(0.0,s1);

    s0 = std::min(1.0,s0);
    s1 = std::min(1.0,s1);

    if (std::fabs(s1-s0) < e) {
        return false;
    }

    out0 = n0 + s0 * n0_to_n1;
    out1 = n0 + s1 * n0_to_n1;

    return true;
}

// ------------------------------------------------------------------------------------------------
void FindAdjacentContours(ContourVector::iterator current, const ContourVector& contours)
{
    const IfcFloat sqlen_epsilon = static_cast<IfcFloat>(1e-8);
    const BoundingBox& bb = (*current).bb;

    // What is to be done here is to populate the skip lists for the contour
    // and to add necessary padding points when needed.
    SkipList& skiplist = (*current).skiplist;

    // First step to find possible adjacent contours is to check for adjacent bounding
    // boxes. If the bounding boxes are not adjacent, the contours lines cannot possibly be.
    for (ContourVector::const_iterator it = contours.begin(), end = contours.end(); it != end; ++it) {
        if ((*it).IsInvalid()) {
            continue;
        }

        // this left here to make clear we also run on the current contour
        // to check for overlapping contour segments (which can happen due
        // to projection artifacts).
        //if(it == current) {
        //  continue;
        //}

        const bool is_me = it == current;

        const BoundingBox& ibb = (*it).bb;

        // Assumption: the bounding boxes are pairwise disjoint or identical
        ai_assert(is_me || !BoundingBoxesOverlapping(bb, ibb));

        if (is_me || BoundingBoxesAdjacent(bb, ibb)) {

            // Now do a each-against-everyone check for intersecting contour
            // lines. This obviously scales terribly, but in typical real
            // world Ifc files it will not matter since most windows that
            // are adjacent to each others are rectangular anyway.

            Contour& ncontour = (*current).contour;
            const Contour& mcontour = (*it).contour;

            for (size_t n = 0; n < ncontour.size(); ++n) {
                const IfcVector2 n0 = ncontour[n];
                const IfcVector2 n1 = ncontour[(n+1) % ncontour.size()];

                for (size_t m = 0, mend = (is_me ? n : mcontour.size()); m < mend; ++m) {
                    ai_assert(&mcontour != &ncontour || m < n);

                    const IfcVector2 m0 = mcontour[m];
                    const IfcVector2 m1 = mcontour[(m+1) % mcontour.size()];

                    IfcVector2 isect0, isect1;
                    if (IntersectingLineSegments(n0,n1, m0, m1, isect0, isect1)) {

                        if ((isect0 - n0).SquareLength() > sqlen_epsilon) {
                            ++n;

                            ncontour.insert(ncontour.begin() + n, isect0);
                            skiplist.insert(skiplist.begin() + n, true);
                        }
                        else {
                            skiplist[n] = true;
                        }

                        if ((isect1 - n1).SquareLength() > sqlen_epsilon) {
                            ++n;

                            ncontour.insert(ncontour.begin() + n, isect1);
                            skiplist.insert(skiplist.begin() + n, false);
                        }
                    }
                }
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------
AI_FORCE_INLINE bool LikelyBorder(const IfcVector2& vdelta)
{
    const IfcFloat dot_point_epsilon = static_cast<IfcFloat>(1e-5);
    return std::fabs(vdelta.x * vdelta.y) < dot_point_epsilon;
}

// ------------------------------------------------------------------------------------------------
void FindBorderContours(ContourVector::iterator current)
{
    const IfcFloat border_epsilon_upper = static_cast<IfcFloat>(1-1e-4);
    const IfcFloat border_epsilon_lower = static_cast<IfcFloat>(1e-4);

    bool outer_border = false;
    bool start_on_outer_border = false;

    SkipList& skiplist = (*current).skiplist;
    IfcVector2 last_proj_point;

    const Contour::const_iterator cbegin = (*current).contour.begin(), cend = (*current).contour.end();

    for (Contour::const_iterator cit = cbegin; cit != cend; ++cit) {
        const IfcVector2& proj_point = *cit;

        // Check if this connection is along the outer boundary of the projection
        // plane. In such a case we better drop it because such 'edges' should
        // not have any geometry to close them (think of door openings).
        if (proj_point.x <= border_epsilon_lower || proj_point.x >= border_epsilon_upper ||
            proj_point.y <= border_epsilon_lower || proj_point.y >= border_epsilon_upper) {

                if (outer_border) {
                    ai_assert(cit != cbegin);
                    if (LikelyBorder(proj_point - last_proj_point)) {
                        skiplist[std::distance(cbegin, cit) - 1] = true;
                    }
                }
                else if (cit == cbegin) {
                    start_on_outer_border = true;
                }

                outer_border = true;
        }
        else {
            outer_border = false;
        }

        last_proj_point = proj_point;
    }

    // handle last segment
    if (outer_border && start_on_outer_border) {
        const IfcVector2& proj_point = *cbegin;
        if (LikelyBorder(proj_point - last_proj_point)) {
            skiplist[skiplist.size()-1] = true;
        }
    }
}

// ------------------------------------------------------------------------------------------------
AI_FORCE_INLINE bool LikelyDiagonal(IfcVector2 vdelta)
{
    vdelta.x = std::fabs(vdelta.x);
    vdelta.y = std::fabs(vdelta.y);
    return (std::fabs(vdelta.x-vdelta.y) < 0.8 * std::max(vdelta.x, vdelta.y));
}

// ------------------------------------------------------------------------------------------------
void FindLikelyCrossingLines(ContourVector::iterator current)
{
    SkipList& skiplist = (*current).skiplist;
    IfcVector2 last_proj_point;

    const Contour::const_iterator cbegin = (*current).contour.begin(), cend = (*current).contour.end();
    for (Contour::const_iterator cit = cbegin; cit != cend; ++cit) {
        const IfcVector2& proj_point = *cit;

        if (cit != cbegin) {
            IfcVector2 vdelta = proj_point - last_proj_point;
            if (LikelyDiagonal(vdelta)) {
                skiplist[std::distance(cbegin, cit) - 1] = true;
            }
        }

        last_proj_point = proj_point;
    }

    // handle last segment
    if (LikelyDiagonal(*cbegin - last_proj_point)) {
        skiplist[skiplist.size()-1] = true;
    }
}

// ------------------------------------------------------------------------------------------------
size_t CloseWindows(ContourVector& contours,
    const IfcMatrix4& minv,
    OpeningRefVector& contours_to_openings,
    TempMesh& curmesh)
{
    size_t closed = 0;
    // For all contour points, check if one of the assigned openings does
    // already have points assigned to it. In this case, assume this is
    // the other side of the wall and generate connections between
    // the two holes in order to close the window.

    // All this gets complicated by the fact that contours may pertain to
    // multiple openings(due to merging of adjacent or overlapping openings).
    // The code is based on the assumption that this happens symmetrically
    // on both sides of the wall. If it doesn't (which would be a bug anyway)
    // wrong geometry may be generated.
    for (ContourVector::iterator it = contours.begin(), end = contours.end(); it != end; ++it) {
        if ((*it).IsInvalid()) {
            continue;
        }
        OpeningRefs& refs = contours_to_openings[std::distance(contours.begin(), it)];

        bool has_other_side = false;
        for(const TempOpening* opening : refs) {
            if(!opening->wallPoints.empty()) {
                has_other_side = true;
                break;
            }
        }

        if (has_other_side) {

            ContourRefVector adjacent_contours;

            // prepare a skiplist for this contour. The skiplist is used to
            // eliminate unwanted contour lines for adjacent windows and
            // those bordering the outer frame.
            (*it).PrepareSkiplist();

            FindAdjacentContours(it, contours);
            FindBorderContours(it);

            // if the window is the result of a finite union or intersection of rectangles,
            // there shouldn't be any crossing or diagonal lines in it. Such lines would
            // be artifacts caused by numerical inaccuracies or other bugs in polyclipper
            // and our own code. Since rectangular openings are by far the most frequent
            // case, it is worth filtering for this corner case.
            if((*it).is_rectangular) {
                FindLikelyCrossingLines(it);
            }

            ai_assert((*it).skiplist.size() == (*it).contour.size());

            SkipList::const_iterator skipbegin = (*it).skiplist.begin();

            curmesh.mVerts.reserve(curmesh.mVerts.size() + (*it).contour.size() * 4);
            curmesh.mVertcnt.reserve(curmesh.mVertcnt.size() + (*it).contour.size());

			bool reverseCountourFaces = false;

            // compare base poly normal and contour normal to detect if we need to reverse the face winding
			if(curmesh.mVertcnt.size() > 0) {
				IfcVector3 basePolyNormal = TempMesh::ComputePolygonNormal(curmesh.mVerts.data(), curmesh.mVertcnt.front());
				
				std::vector<IfcVector3> worldSpaceContourVtx(it->contour.size());
				
				for(size_t a = 0; a < it->contour.size(); ++a)
					worldSpaceContourVtx[a] = minv * IfcVector3(it->contour[a].x, it->contour[a].y, 0.0);
				
				IfcVector3 contourNormal = TempMesh::ComputePolygonNormal(worldSpaceContourVtx.data(), worldSpaceContourVtx.size());
				
				reverseCountourFaces = (contourNormal * basePolyNormal) > 0.0;
			}

            // XXX this algorithm is really a bit inefficient - both in terms
            // of constant factor and of asymptotic runtime.
            std::vector<bool>::const_iterator skipit = skipbegin;

            IfcVector3 start0;
            IfcVector3 start1;

            const Contour::const_iterator cbegin = (*it).contour.begin(), cend = (*it).contour.end();

            bool drop_this_edge = false;
            for (Contour::const_iterator cit = cbegin; cit != cend; ++cit, drop_this_edge = *skipit++) {
                const IfcVector2& proj_point = *cit;

                // Locate the closest opposite point. This should be a good heuristic to
                // connect only the points that are really intended to be connected.
                IfcFloat best = static_cast<IfcFloat>(1e10);
                IfcVector3 bestv;

                const IfcVector3 world_point = minv * IfcVector3(proj_point.x,proj_point.y,0.0f);

                for(const TempOpening* opening : refs) {
                    for(const IfcVector3& other : opening->wallPoints) {
                        const IfcFloat sqdist = (world_point - other).SquareLength();

                        if (sqdist < best) {
                            // avoid self-connections
                            if(sqdist < 1e-5) {
                                continue;
                            }

                            bestv = other;
                            best = sqdist;
                        }
                    }
                }

                if (drop_this_edge) {
                    curmesh.mVerts.pop_back();
                    curmesh.mVerts.pop_back();
                }
                else {
                    curmesh.mVerts.push_back(((cit == cbegin) != reverseCountourFaces) ? world_point : bestv);
                    curmesh.mVerts.push_back(((cit == cbegin) != reverseCountourFaces) ? bestv : world_point);

                    curmesh.mVertcnt.push_back(4);
                    ++closed;
                }

                if (cit == cbegin) {
                    start0 = world_point;
                    start1 = bestv;
                    continue;
                }

                curmesh.mVerts.push_back(reverseCountourFaces ? bestv : world_point);
                curmesh.mVerts.push_back(reverseCountourFaces ? world_point : bestv);

                if (cit == cend - 1) {
                    drop_this_edge = *skipit;

                    // Check if the final connection (last to first element) is itself
                    // a border edge that needs to be dropped.
                    if (drop_this_edge) {
                        --closed;
                        curmesh.mVertcnt.pop_back();
                        curmesh.mVerts.pop_back();
                        curmesh.mVerts.pop_back();
                    }
                    else {
                        curmesh.mVerts.push_back(reverseCountourFaces ? start0 : start1);
                        curmesh.mVerts.push_back(reverseCountourFaces ? start1 : start0);
                    }
                }
            }
        }
        else {

            const Contour::const_iterator cbegin = (*it).contour.begin(), cend = (*it).contour.end();
            for(TempOpening* opening : refs) {
                ai_assert(opening->wallPoints.empty());
                opening->wallPoints.reserve(opening->wallPoints.capacity() + (*it).contour.size());
                for (Contour::const_iterator cit = cbegin; cit != cend; ++cit) {

                    const IfcVector2& proj_point = *cit;
                    opening->wallPoints.push_back(minv * IfcVector3(proj_point.x,proj_point.y,0.0f));
                }
            }
        }
    }
    return closed;
}

// ------------------------------------------------------------------------------------------------
void Quadrify(const std::vector< BoundingBox >& bbs, TempMesh& curmesh)
{
    ai_assert(curmesh.IsEmpty());

    std::vector<IfcVector2> quads;
    quads.reserve(bbs.size()*4);

    // sort openings by x and y axis as a preliminiary to the QuadrifyPart() algorithm
    XYSortedField field;
    for (std::vector<BoundingBox>::const_iterator it = bbs.begin(); it != bbs.end(); ++it) {
        if (field.find((*it).first) != field.end()) {
            IFCImporter::LogWarn("constraint failure during generation of wall openings, results may be faulty");
        }
        field[(*it).first] = std::distance(bbs.begin(),it);
    }

    QuadrifyPart(IfcVector2(),one_vec,field,bbs,quads);
    ai_assert(!(quads.size() % 4));

    curmesh.mVertcnt.resize(quads.size()/4,4);
    curmesh.mVerts.reserve(quads.size());
    for(const IfcVector2& v2 : quads) {
        curmesh.mVerts.push_back(IfcVector3(v2.x, v2.y, static_cast<IfcFloat>(0.0)));
    }
}

// ------------------------------------------------------------------------------------------------
void Quadrify(const ContourVector& contours, TempMesh& curmesh)
{
    std::vector<BoundingBox> bbs;
    bbs.reserve(contours.size());

    for(const ContourVector::value_type& val : contours) {
        bbs.push_back(val.bb);
    }

    Quadrify(bbs, curmesh);
}

// ------------------------------------------------------------------------------------------------
IfcMatrix4 ProjectOntoPlane(std::vector<IfcVector2>& out_contour, const TempMesh& in_mesh,
    bool &ok, IfcVector3& nor_out)
{
    const std::vector<IfcVector3>& in_verts = in_mesh.mVerts;
    ok = true;

    IfcMatrix4 m = IfcMatrix4(DerivePlaneCoordinateSpace(in_mesh, ok, nor_out));
    if(!ok) {
        return IfcMatrix4();
    }
#ifdef ASSIMP_BUILD_DEBUG
    const IfcFloat det = m.Determinant();
    ai_assert(std::fabs(det-1) < 1e-5);
#endif

    IfcFloat zcoord = 0;
    out_contour.reserve(in_verts.size());


    IfcVector3 vmin, vmax;
    MinMaxChooser<IfcVector3>()(vmin, vmax);

    // Project all points into the new coordinate system, collect min/max verts on the way
    for(const IfcVector3& x : in_verts) {
        const IfcVector3 vv = m * x;
        // keep Z offset in the plane coordinate system. Ignoring precision issues
        // (which  are present, of course), this should be the same value for
        // all polygon vertices (assuming the polygon is planar).

        // XXX this should be guarded, but we somehow need to pick a suitable
        // epsilon
        // if(coord != -1.0f) {
        //  assert(std::fabs(coord - vv.z) < 1e-3f);
        // }
        zcoord += vv.z;
        vmin = std::min(vv, vmin);
        vmax = std::max(vv, vmax);

        out_contour.push_back(IfcVector2(vv.x,vv.y));
    }

    zcoord /= in_verts.size();

    // Further improve the projection by mapping the entire working set into
    // [0,1] range. This gives us a consistent data range so all epsilons
    // used below can be constants.
    vmax -= vmin;
    for(IfcVector2& vv : out_contour) {
        vv.x  = (vv.x - vmin.x) / vmax.x;
        vv.y  = (vv.y - vmin.y) / vmax.y;

        // sanity rounding
        vv = std::max(vv,IfcVector2());
        vv = std::min(vv,one_vec);
    }

    IfcMatrix4 mult;
    mult.a1 = static_cast<IfcFloat>(1.0) / vmax.x;
    mult.b2 = static_cast<IfcFloat>(1.0) / vmax.y;

    mult.a4 = -vmin.x * mult.a1;
    mult.b4 = -vmin.y * mult.b2;
    mult.c4 = -zcoord;
    m = mult * m;

    // debug code to verify correctness
#ifdef ASSIMP_BUILD_DEBUG
    std::vector<IfcVector2> out_contour2;
    for(const IfcVector3& x : in_verts) {
        const IfcVector3& vv = m * x;

        out_contour2.push_back(IfcVector2(vv.x,vv.y));
        ai_assert(std::fabs(vv.z) < vmax.z + 1e-8);
    }

    for(size_t i = 0; i < out_contour.size(); ++i) {
        ai_assert((out_contour[i]-out_contour2[i]).SquareLength() < 1e-6);
    }
#endif

    return m;
}

// ------------------------------------------------------------------------------------------------
bool GenerateOpenings(std::vector<TempOpening>& openings,
    const std::vector<IfcVector3>& nors,
    TempMesh& curmesh,
    bool check_intersection,
    bool generate_connection_geometry,
    const IfcVector3& wall_extrusion_axis)
{
    OpeningRefVector contours_to_openings;

    // Try to derive a solid base plane within the current surface for use as
    // working coordinate system. Map all vertices onto this plane and
    // rescale them to [0,1] range. This normalization means all further
    // epsilons need not be scaled.
    bool ok = true;

    std::vector<IfcVector2> contour_flat;

    IfcVector3 nor;
    const IfcMatrix4 m = ProjectOntoPlane(contour_flat, curmesh,  ok, nor);
    if(!ok) {
        return false;
    }

    // Obtain inverse transform for getting back to world space later on
    const IfcMatrix4 minv = IfcMatrix4(m).Inverse();

    // Compute bounding boxes for all 2D openings in projection space
    ContourVector contours;

    std::vector<IfcVector2> temp_contour;
    std::vector<IfcVector2> temp_contour2;

    IfcVector3 wall_extrusion_axis_norm = wall_extrusion_axis;
    wall_extrusion_axis_norm.Normalize();

    for(TempOpening& opening :openings) {

        // extrusionDir may be 0,0,0 on case where the opening mesh is not an
        // IfcExtrudedAreaSolid but something else (i.e. a brep)
        IfcVector3 norm_extrusion_dir = opening.extrusionDir;
        if (norm_extrusion_dir.SquareLength() > 1e-10) {
            norm_extrusion_dir.Normalize();
        }
        else {
            norm_extrusion_dir = IfcVector3();
        }

        TempMesh* profile_data =  opening.profileMesh.get();
        bool is_2d_source = false;
        if (opening.profileMesh2D && norm_extrusion_dir.SquareLength() > 0) {

            if(std::fabs(norm_extrusion_dir * wall_extrusion_axis_norm) < 0.1) {
                // horizontal extrusion
                if (std::fabs(norm_extrusion_dir * nor) > 0.9) {
                    profile_data = opening.profileMesh2D.get();
                    is_2d_source = true;
                }
            }
            else {
                // vertical extrusion
                if (std::fabs(norm_extrusion_dir * nor) > 0.9) {
                    profile_data = opening.profileMesh2D.get();
                    is_2d_source = true;
                }
            }
        }
        std::vector<IfcVector3> profile_verts = profile_data->mVerts;
        std::vector<unsigned int> profile_vertcnts = profile_data->mVertcnt;
        if(profile_verts.size() <= 2) {
            continue;
        }

        // The opening meshes are real 3D meshes so skip over all faces
        // clearly facing into the wrong direction. Also, we need to check
        // whether the meshes do actually intersect the base surface plane.
        // This is done by recording minimum and maximum values for the
        // d component of the plane equation for all polys and checking
        // against surface d.

        // Use the sign of the dot product of the face normal to the plane
        // normal to determine to which side of the difference mesh a
        // triangle belongs. Get independent bounding boxes and vertex
        // sets for both sides and take the better one (we can't just
        // take both - this would likely cause major screwup of vertex
        // winding, producing errors as late as in CloseWindows()).
        IfcFloat dmin, dmax;
        MinMaxChooser<IfcFloat>()(dmin,dmax);

        temp_contour.clear();
        temp_contour2.clear();

        IfcVector2 vpmin,vpmax;
        MinMaxChooser<IfcVector2>()(vpmin,vpmax);

        IfcVector2 vpmin2,vpmax2;
        MinMaxChooser<IfcVector2>()(vpmin2,vpmax2);

        for (size_t f = 0, vi_total = 0, fend = profile_vertcnts.size(); f < fend; ++f) {

            bool side_flag = true;
            if (!is_2d_source) {
                const IfcVector3 face_nor = ((profile_verts[vi_total+2] - profile_verts[vi_total]) ^
                    (profile_verts[vi_total+1] - profile_verts[vi_total])).Normalize();

                const IfcFloat abs_dot_face_nor = std::abs(nor * face_nor);
                if (abs_dot_face_nor < 0.9) {
                    vi_total += profile_vertcnts[f];
                    continue;
                }

                side_flag = nor * face_nor > 0;
            }

            for (unsigned int vi = 0, vend = profile_vertcnts[f]; vi < vend; ++vi, ++vi_total) {
                const IfcVector3& x = profile_verts[vi_total];

                const IfcVector3 v = m * x;
                IfcVector2 vv(v.x, v.y);

                //if(check_intersection) {
                    dmin = std::min(dmin, v.z);
                    dmax = std::max(dmax, v.z);
                //}

                // sanity rounding
                vv = std::max(vv,IfcVector2());
                vv = std::min(vv,one_vec);

                if(side_flag) {
                    vpmin = std::min(vpmin,vv);
                    vpmax = std::max(vpmax,vv);
                }
                else {
                    vpmin2 = std::min(vpmin2,vv);
                    vpmax2 = std::max(vpmax2,vv);
                }

                std::vector<IfcVector2>& store = side_flag ? temp_contour : temp_contour2;

                if (!IsDuplicateVertex(vv, store)) {
                    store.push_back(vv);
                }
            }
        }

        if (temp_contour2.size() > 2) {
            ai_assert(!is_2d_source);
            const IfcVector2 area = vpmax-vpmin;
            const IfcVector2 area2 = vpmax2-vpmin2;
            if (temp_contour.size() <= 2 || std::fabs(area2.x * area2.y) > std::fabs(area.x * area.y)) {
                temp_contour.swap(temp_contour2);

                vpmax = vpmax2;
                vpmin = vpmin2;
            }
        }
        if(temp_contour.size() <= 2) {
            continue;
        }

        // TODO: This epsilon may be too large
        const IfcFloat epsilon = std::fabs(dmax-dmin) * 0.0001;
        if (!is_2d_source && check_intersection && (0 < dmin-epsilon || 0 > dmax+epsilon)) {
            continue;
        }

        BoundingBox bb = BoundingBox(vpmin,vpmax);

        // Skip over very small openings - these are likely projection errors
        // (i.e. they don't belong to this side of the wall)
        if(std::fabs(vpmax.x - vpmin.x) * std::fabs(vpmax.y - vpmin.y) < static_cast<IfcFloat>(1e-10)) {
            continue;
        }
        std::vector<TempOpening*> joined_openings(1, &opening);

        bool is_rectangle = temp_contour.size() == 4;

        // See if this BB intersects or is in close adjacency to any other BB we have so far.
        for (ContourVector::iterator it = contours.begin(); it != contours.end(); ) {
            const BoundingBox& ibb = (*it).bb;

            if (BoundingBoxesOverlapping(ibb, bb)) {

                if (!(*it).is_rectangular) {
                    is_rectangle = false;
                }

                const std::vector<IfcVector2>& other = (*it).contour;
                ClipperLib::ExPolygons poly;

                // First check whether subtracting the old contour (to which ibb belongs)
                // from the new contour (to which bb belongs) yields an updated bb which
                // no longer overlaps ibb
                MakeDisjunctWindowContours(other, temp_contour, poly);
                if(poly.size() == 1) {

                    const BoundingBox newbb = GetBoundingBox(poly[0].outer);
                    if (!BoundingBoxesOverlapping(ibb, newbb )) {
                         // Good guy bounding box
                         bb = newbb ;

                         ExtractVerticesFromClipper(poly[0].outer, temp_contour, false);
                         continue;
                    }
                }

                // Take these two overlapping contours and try to merge them. If they
                // overlap (which should not happen, but in fact happens-in-the-real-
                // world [tm] ), resume using a single contour and a single bounding box.
                MergeWindowContours(temp_contour, other, poly);

                if (poly.size() > 1) {
                    return TryAddOpenings_Poly2Tri(openings, nors, curmesh);
                }
                else if (poly.size() == 0) {
                    IFCImporter::LogWarn("ignoring duplicate opening");
                    temp_contour.clear();
                    break;
                }
                else {
                    IFCImporter::LogDebug("merging overlapping openings");
                    ExtractVerticesFromClipper(poly[0].outer, temp_contour, false);

                    // Generate the union of the bounding boxes
                    bb.first = std::min(bb.first, ibb.first);
                    bb.second = std::max(bb.second, ibb.second);

                    // Update contour-to-opening tables accordingly
                    if (generate_connection_geometry) {
                        std::vector<TempOpening*>& t = contours_to_openings[std::distance(contours.begin(),it)];
                        joined_openings.insert(joined_openings.end(), t.begin(), t.end());

                        contours_to_openings.erase(contours_to_openings.begin() + std::distance(contours.begin(),it));
                    }

                    contours.erase(it);

                    // Restart from scratch because the newly formed BB might now
                    // overlap any other BB which its constituent BBs didn't
                    // previously overlap.
                    it = contours.begin();
                    continue;
                }
            }
            ++it;
        }

        if(!temp_contour.empty()) {
            if (generate_connection_geometry) {
                contours_to_openings.push_back(std::vector<TempOpening*>(
                    joined_openings.begin(),
                    joined_openings.end()));
            }

            contours.push_back(ProjectedWindowContour(temp_contour, bb, is_rectangle));
        }
    }

    // Check if we still have any openings left - it may well be that this is
    // not the cause, for example if all the opening candidates don't intersect
    // this surface or point into a direction perpendicular to it.
    if (contours.empty()) {
        return false;
    }

    curmesh.Clear();

    // Generate a base subdivision into quads to accommodate the given list
    // of window bounding boxes.
    Quadrify(contours,curmesh);

    // Run a sanity cleanup pass on the window contours to avoid generating
    // artifacts during the contour generation phase later on.
    CleanupWindowContours(contours);

    // Previously we reduced all windows to rectangular AABBs in projection
    // space, now it is time to fill the gaps between the BBs and the real
    // window openings.
    InsertWindowContours(contours,openings, curmesh);

    // Clip the entire outer contour of our current result against the real
    // outer contour of the surface. This is necessary because the result
    // of the Quadrify() algorithm is always a square area spanning
    // over [0,1]^2 (i.e. entire projection space).
    CleanupOuterContour(contour_flat, curmesh);

    // Undo the projection and get back to world (or local object) space
    for(IfcVector3& v3 : curmesh.mVerts) {
        v3 = minv * v3;
    }

    // Generate window caps to connect the symmetric openings on both sides
    // of the wall.
    if (generate_connection_geometry) {
        CloseWindows(contours, minv, contours_to_openings, curmesh);
    }
    return true;
}

// ------------------------------------------------------------------------------------------------
bool TryAddOpenings_Poly2Tri(const std::vector<TempOpening>& openings,const std::vector<IfcVector3>& nors,
    TempMesh& curmesh)
{
    IFCImporter::LogWarn("forced to use poly2tri fallback method to generate wall openings");
    std::vector<IfcVector3>& out = curmesh.mVerts;

    bool result = false;

    // Try to derive a solid base plane within the current surface for use as
    // working coordinate system.
    bool ok;
    IfcVector3 nor;
    const IfcMatrix3 m = DerivePlaneCoordinateSpace(curmesh, ok, nor);
    if (!ok) {
        return false;
    }

    const IfcMatrix3 minv = IfcMatrix3(m).Inverse();


    IfcFloat coord = -1;

    std::vector<IfcVector2> contour_flat;
    contour_flat.reserve(out.size());

    IfcVector2 vmin, vmax;
    MinMaxChooser<IfcVector2>()(vmin, vmax);

    // Move all points into the new coordinate system, collecting min/max verts on the way
    for(IfcVector3& x : out) {
        const IfcVector3 vv = m * x;

        // keep Z offset in the plane coordinate system. Ignoring precision issues
        // (which  are present, of course), this should be the same value for
        // all polygon vertices (assuming the polygon is planar).


        // XXX this should be guarded, but we somehow need to pick a suitable
        // epsilon
        // if(coord != -1.0f) {
        //  assert(std::fabs(coord - vv.z) < 1e-3f);
        // }

        coord = vv.z;

        vmin = std::min(IfcVector2(vv.x, vv.y), vmin);
        vmax = std::max(IfcVector2(vv.x, vv.y), vmax);

        contour_flat.push_back(IfcVector2(vv.x,vv.y));
    }

    // With the current code in DerivePlaneCoordinateSpace,
    // vmin,vmax should always be the 0...1 rectangle (+- numeric inaccuracies)
    // but here we won't rely on this.

    vmax -= vmin;

    // If this happens then the projection must have been wrong.
    ai_assert(vmax.Length());

    ClipperLib::ExPolygons clipped;
    ClipperLib::Polygons holes_union;


    IfcVector3 wall_extrusion;
    bool first = true;

    try {

        ClipperLib::Clipper clipper_holes;
        size_t c = 0;

        for(const TempOpening& t :openings) {
            const IfcVector3& outernor = nors[c++];
            const IfcFloat dot = nor * outernor;
            if (std::fabs(dot)<1.f-1e-6f) {
                continue;
            }

            const std::vector<IfcVector3>& va = t.profileMesh->mVerts;
            if(va.size() <= 2) {
                continue;
            }

            std::vector<IfcVector2> contour;

            for(const IfcVector3& xx : t.profileMesh->mVerts) {
                IfcVector3 vv = m *  xx, vv_extr = m * (xx + t.extrusionDir);

                const bool is_extruded_side = std::fabs(vv.z - coord) > std::fabs(vv_extr.z - coord);
                if (first) {
                    first = false;
                    if (dot > 0.f) {
                        wall_extrusion = t.extrusionDir;
                        if (is_extruded_side) {
                            wall_extrusion = - wall_extrusion;
                        }
                    }
                }

                // XXX should not be necessary - but it is. Why? For precision reasons?
                vv = is_extruded_side ? vv_extr : vv;
                contour.push_back(IfcVector2(vv.x,vv.y));
            }

            ClipperLib::Polygon hole;
            for(IfcVector2& pip : contour) {
                pip.x  = (pip.x - vmin.x) / vmax.x;
                pip.y  = (pip.y - vmin.y) / vmax.y;

                hole.push_back(ClipperLib::IntPoint(  to_int64(pip.x), to_int64(pip.y) ));
            }

            if (!ClipperLib::Orientation(hole)) {
                std::reverse(hole.begin(), hole.end());
            //  assert(ClipperLib::Orientation(hole));
            }

            /*ClipperLib::Polygons pol_temp(1), pol_temp2(1);
            pol_temp[0] = hole;

            ClipperLib::OffsetPolygons(pol_temp,pol_temp2,5.0);
            hole = pol_temp2[0];*/

            clipper_holes.AddPolygon(hole,ClipperLib::ptSubject);
        }

        clipper_holes.Execute(ClipperLib::ctUnion,holes_union,
            ClipperLib::pftNonZero,
            ClipperLib::pftNonZero);

        if (holes_union.empty()) {
            return false;
        }

        // Now that we have the big union of all holes, subtract it from the outer contour
        // to obtain the final polygon to feed into the triangulator.
        {
            ClipperLib::Polygon poly;
            for(IfcVector2& pip : contour_flat) {
                pip.x  = (pip.x - vmin.x) / vmax.x;
                pip.y  = (pip.y - vmin.y) / vmax.y;

                poly.push_back(ClipperLib::IntPoint( to_int64(pip.x), to_int64(pip.y) ));
            }

            if (ClipperLib::Orientation(poly)) {
                std::reverse(poly.begin(), poly.end());
            }
            clipper_holes.Clear();
            clipper_holes.AddPolygon(poly,ClipperLib::ptSubject);

            clipper_holes.AddPolygons(holes_union,ClipperLib::ptClip);
            clipper_holes.Execute(ClipperLib::ctDifference,clipped,
                ClipperLib::pftNonZero,
                ClipperLib::pftNonZero);
        }

    }
    catch (const char* sx) {
        IFCImporter::LogError("Ifc: error during polygon clipping, skipping openings for this face: (Clipper: "
            + std::string(sx) + ")");

        return false;
    }

    std::vector<IfcVector3> old_verts;
    std::vector<unsigned int> old_vertcnt;

    old_verts.swap(curmesh.mVerts);
    old_vertcnt.swap(curmesh.mVertcnt);

    std::vector< std::vector<p2t::Point*> > contours;
    for(ClipperLib::ExPolygon& clip : clipped) {

        contours.clear();

        // Build the outer polygon contour line for feeding into poly2tri
        std::vector<p2t::Point*> contour_points;
        for(ClipperLib::IntPoint& point : clip.outer) {
            contour_points.push_back( new p2t::Point(from_int64(point.X), from_int64(point.Y)) );
        }

        p2t::CDT* cdt ;
        try {
            // Note: this relies on custom modifications in poly2tri to raise runtime_error's
            // instead if assertions. These failures are not debug only, they can actually
            // happen in production use if the input data is broken. An assertion would be
            // inappropriate.
            cdt = new p2t::CDT(contour_points);
        }
        catch(const std::exception& e) {
            IFCImporter::LogError("Ifc: error during polygon triangulation, skipping some openings: (poly2tri: "
                + std::string(e.what()) + ")");
            continue;
        }


        // Build the poly2tri inner contours for all holes we got from ClipperLib
        for(ClipperLib::Polygon& opening : clip.holes) {

            contours.push_back(std::vector<p2t::Point*>());
            std::vector<p2t::Point*>& contour = contours.back();

            for(ClipperLib::IntPoint& point : opening) {
                contour.push_back( new p2t::Point(from_int64(point.X), from_int64(point.Y)) );
            }

            cdt->AddHole(contour);
        }

        try {
            // Note: See above
            cdt->Triangulate();
        }
        catch(const std::exception& e) {
            IFCImporter::LogError("Ifc: error during polygon triangulation, skipping some openings: (poly2tri: "
                + std::string(e.what()) + ")");
            continue;
        }

        const std::vector<p2t::Triangle*> tris = cdt->GetTriangles();

        // Collect the triangles we just produced
        for(p2t::Triangle* tri : tris) {
            for(int i = 0; i < 3; ++i) {

                const IfcVector2 v = IfcVector2(
                    static_cast<IfcFloat>( tri->GetPoint(i)->x ),
                    static_cast<IfcFloat>( tri->GetPoint(i)->y )
                );

                ai_assert(v.x <= 1.0 && v.x >= 0.0 && v.y <= 1.0 && v.y >= 0.0);
                const IfcVector3 v3 = minv * IfcVector3(vmin.x + v.x * vmax.x, vmin.y + v.y * vmax.y,coord) ;

                curmesh.mVerts.push_back(v3);
            }
            curmesh.mVertcnt.push_back(3);
        }

        result = true;
    }

    if (!result) {
        // revert -- it's a shame, but better than nothing
        curmesh.mVerts.insert(curmesh.mVerts.end(),old_verts.begin(), old_verts.end());
        curmesh.mVertcnt.insert(curmesh.mVertcnt.end(),old_vertcnt.begin(), old_vertcnt.end());

        IFCImporter::LogError("Ifc: revert, could not generate openings for this wall");
    }

    return result;
}


    } // ! IFC
} // ! Assimp

#undef to_int64
#undef from_int64
#undef one_vec

#endif
