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

/** @file  IFCBoolean.cpp
 *  @brief Implements a subset of Ifc boolean operations
 */

#ifndef ASSIMP_BUILD_NO_IFC_IMPORTER
#include "code/Importer/IFC/IFCUtil.h"
#include "code/Common/PolyTools.h"
#include "code/PostProcessing/ProcessHelper.h"
#include <assimp/Defines.h>

#include <iterator>
#include <tuple>

namespace Assimp {
    namespace IFC {

// ------------------------------------------------------------------------------------------------
// Calculates intersection between line segment and plane. To catch corner cases, specify which side you prefer.
// The function then generates a hit only if the end is beyond a certain margin in that direction, filtering out
// "very close to plane" ghost hits as long as start and end stay directly on or within the given plane side.
bool IntersectSegmentPlane(const IfcVector3& p,const IfcVector3& n, const IfcVector3& e0,
        const IfcVector3& e1, bool assumeStartOnWhiteSide, IfcVector3& out)
{
    const IfcVector3 pdelta = e0 - p, seg = e1 - e0;
    const IfcFloat dotOne = n*seg, dotTwo = -(n*pdelta);

    // if segment ends on plane, do not report a hit. We stay on that side until a following segment starting at this
    // point leaves the plane through the other side
    if( std::abs(dotOne + dotTwo) < 1e-6 )
        return false;

    // if segment starts on the plane, report a hit only if the end lies on the *other* side
    if( std::abs(dotTwo) < 1e-6 )
    {
        if( (assumeStartOnWhiteSide && dotOne + dotTwo < 1e-6) || (!assumeStartOnWhiteSide && dotOne + dotTwo > -1e-6) )
        {
            out = e0;
            return true;
        }
        else
        {
            return false;
        }
    }

    // ignore if segment is parallel to plane and far away from it on either side
    // Warning: if there's a few thousand of such segments which slowly accumulate beyond the epsilon, no hit would be registered
    if( std::abs(dotOne) < 1e-6 )
        return false;

    // t must be in [0..1] if the intersection point is within the given segment
    const IfcFloat t = dotTwo / dotOne;
    if( t > 1.0 || t < 0.0 )
        return false;

    out = e0 + t*seg;
    return true;
}

// ------------------------------------------------------------------------------------------------
void FilterPolygon(std::vector<IfcVector3>& resultpoly)
{
    if( resultpoly.size() < 3 )
    {
        resultpoly.clear();
        return;
    }

    IfcVector3 vmin, vmax;
    ArrayBounds(resultpoly.data(), static_cast<unsigned int>(resultpoly.size()), vmin, vmax);

    // filter our IfcFloat points - those may happen if a point lies
    // directly on the intersection line or directly on the clipping plane
    const IfcFloat epsilon = (vmax - vmin).SquareLength() / 1e6f;
    FuzzyVectorCompare fz(epsilon);
    std::vector<IfcVector3>::iterator e = std::unique(resultpoly.begin(), resultpoly.end(), fz);

    if( e != resultpoly.end() )
        resultpoly.erase(e, resultpoly.end());

    if( !resultpoly.empty() && fz(resultpoly.front(), resultpoly.back()) )
        resultpoly.pop_back();
}

// ------------------------------------------------------------------------------------------------
void WritePolygon(std::vector<IfcVector3>& resultpoly, TempMesh& result)
{
    FilterPolygon(resultpoly);

    if( resultpoly.size() > 2 )
    {
        result.mVerts.insert(result.mVerts.end(), resultpoly.begin(), resultpoly.end());
        result.mVertcnt.push_back(static_cast<unsigned int>(resultpoly.size()));
    }
}


// ------------------------------------------------------------------------------------------------
void ProcessBooleanHalfSpaceDifference(const Schema_2x3::IfcHalfSpaceSolid* hs, TempMesh& result,
    const TempMesh& first_operand,
    ConversionData& /*conv*/)
{
    ai_assert(hs != NULL);

    const Schema_2x3::IfcPlane* const plane = hs->BaseSurface->ToPtr<Schema_2x3::IfcPlane>();
    if(!plane) {
        IFCImporter::LogError("expected IfcPlane as base surface for the IfcHalfSpaceSolid");
        return;
    }

    // extract plane base position vector and normal vector
    IfcVector3 p,n(0.f,0.f,1.f);
    if (plane->Position->Axis) {
        ConvertDirection(n,plane->Position->Axis.Get());
    }
    ConvertCartesianPoint(p,plane->Position->Location);

    if(!IsTrue(hs->AgreementFlag)) {
        n *= -1.f;
    }

    // clip the current contents of `meshout` against the plane we obtained from the second operand
    const std::vector<IfcVector3>& in = first_operand.mVerts;
    std::vector<IfcVector3>& outvert = result.mVerts;

    std::vector<unsigned int>::const_iterator begin = first_operand.mVertcnt.begin(),
        end = first_operand.mVertcnt.end(), iit;

    outvert.reserve(in.size());
    result.mVertcnt.reserve(first_operand.mVertcnt.size());

    unsigned int vidx = 0;
    for(iit = begin; iit != end; vidx += *iit++) {

        unsigned int newcount = 0;
        bool isAtWhiteSide = (in[vidx] - p) * n > -1e-6;
        for( unsigned int i = 0; i < *iit; ++i ) {
            const IfcVector3& e0 = in[vidx + i], e1 = in[vidx + (i + 1) % *iit];

            // does the next segment intersect the plane?
            IfcVector3 isectpos;
            if( IntersectSegmentPlane(p, n, e0, e1, isAtWhiteSide, isectpos) ) {
                if( isAtWhiteSide ) {
                    // e0 is on the right side, so keep it
                    outvert.push_back(e0);
                    outvert.push_back(isectpos);
                    newcount += 2;
                }
                else {
                    // e0 is on the wrong side, so drop it and keep e1 instead
                    outvert.push_back(isectpos);
                    ++newcount;
                }
                isAtWhiteSide = !isAtWhiteSide;
            }
            else
            {
                if( isAtWhiteSide ) {
                    outvert.push_back(e0);
                    ++newcount;
                }
            }
        }

        if (!newcount) {
            continue;
        }

        IfcVector3 vmin,vmax;
        ArrayBounds(&*(outvert.end()-newcount),newcount,vmin,vmax);

        // filter our IfcFloat points - those may happen if a point lies
        // directly on the intersection line. However, due to IfcFloat
        // precision a bitwise comparison is not feasible to detect
        // this case.
        const IfcFloat epsilon = (vmax-vmin).SquareLength() / 1e6f;
        FuzzyVectorCompare fz(epsilon);

        std::vector<IfcVector3>::iterator e = std::unique( outvert.end()-newcount, outvert.end(), fz );

        if (e != outvert.end()) {
            newcount -= static_cast<unsigned int>(std::distance(e,outvert.end()));
            outvert.erase(e,outvert.end());
        }
        if (fz(*( outvert.end()-newcount),outvert.back())) {
            outvert.pop_back();
            --newcount;
        }
        if(newcount > 2) {
            result.mVertcnt.push_back(newcount);
        }
        else while(newcount-->0) {
            result.mVerts.pop_back();
        }

    }
    IFCImporter::LogDebug("generating CSG geometry by plane clipping (IfcBooleanClippingResult)");
}

// ------------------------------------------------------------------------------------------------
// Check if e0-e1 intersects a sub-segment of the given boundary line.
// note: this functions works on 3D vectors, but performs its intersection checks solely in xy.
// New version takes the supposed inside/outside state as a parameter and treats corner cases as if
// the line stays on that side. This should make corner cases more stable.
// Two million assumptions! Boundary should have all z at 0.0, will be treated as closed, should not have
// segments with length <1e-6, self-intersecting might break the corner case handling... just don't go there, ok?
bool IntersectsBoundaryProfile(const IfcVector3& e0, const IfcVector3& e1, const std::vector<IfcVector3>& boundary,
    const bool isStartAssumedInside, std::vector<std::pair<size_t, IfcVector3> >& intersect_results,
    const bool halfOpen = false)
{
    ai_assert(intersect_results.empty());

    // determine winding order - necessary to detect segments going "inwards" or "outwards" from a point directly on the border
    // positive sum of angles means clockwise order when looking down the -Z axis
    IfcFloat windingOrder = 0.0;
    for( size_t i = 0, bcount = boundary.size(); i < bcount; ++i ) {
        IfcVector3 b01 = boundary[(i + 1) % bcount] - boundary[i];
        IfcVector3 b12 = boundary[(i + 2) % bcount] - boundary[(i + 1) % bcount];
        IfcVector3 b1_side = IfcVector3(b01.y, -b01.x, 0.0); // rotated 90° clockwise in Z plane
        // Warning: rough estimate only. A concave poly with lots of small segments each featuring a small counter rotation
        // could fool the accumulation. Correct implementation would be sum( acos( b01 * b2) * sign( b12 * b1_side))
        windingOrder += (b1_side.x*b12.x + b1_side.y*b12.y);
    }
    windingOrder = windingOrder > 0.0 ? 1.0 : -1.0;

    const IfcVector3 e = e1 - e0;

    for( size_t i = 0, bcount = boundary.size(); i < bcount; ++i ) {
        // boundary segment i: b0-b1
        const IfcVector3& b0 = boundary[i];
        const IfcVector3& b1 = boundary[(i + 1) % bcount];
        IfcVector3 b = b1 - b0;

        // segment-segment intersection
        // solve b0 + b*s = e0 + e*t for (s,t)
        const IfcFloat det = (-b.x * e.y + e.x * b.y);
        if( std::abs(det) < 1e-6 ) {
            // no solutions (parallel lines)
            continue;
        }
        IfcFloat b_sqlen_inv = 1.0 / b.SquareLength();

        const IfcFloat x = b0.x - e0.x;
        const IfcFloat y = b0.y - e0.y;
        const IfcFloat s = (x*e.y - e.x*y) / det; // scale along boundary edge
        const IfcFloat t = (x*b.y - b.x*y) / det; // scale along given segment
        const IfcVector3 p = e0 + e*t;
#ifdef ASSIMP_BUILD_DEBUG
        const IfcVector3 check = b0 + b*s - p;
        ai_assert((IfcVector2(check.x, check.y)).SquareLength() < 1e-5);
#endif

        // also calculate the distance of e0 and e1 to the segment. We need to detect the "starts directly on segment"
        // and "ends directly at segment" cases
        bool startsAtSegment, endsAtSegment;
        {
            // calculate closest point to each end on the segment, clamp that point to the segment's length, then check
            // distance to that point. This approach is like testing if e0 is inside a capped cylinder.
            IfcFloat et0 = (b.x*(e0.x - b0.x) + b.y*(e0.y - b0.y)) * b_sqlen_inv;
            IfcVector3 closestPosToE0OnBoundary = b0 + std::max(IfcFloat(0.0), std::min(IfcFloat(1.0), et0)) * b;
            startsAtSegment = (closestPosToE0OnBoundary - IfcVector3(e0.x, e0.y, 0.0)).SquareLength() < 1e-12;
            IfcFloat et1 = (b.x*(e1.x - b0.x) + b.y*(e1.y - b0.y)) * b_sqlen_inv;
            IfcVector3 closestPosToE1OnBoundary = b0 + std::max(IfcFloat(0.0), std::min(IfcFloat(1.0), et1)) * b;
            endsAtSegment = (closestPosToE1OnBoundary - IfcVector3(e1.x, e1.y, 0.0)).SquareLength() < 1e-12;
        }

        // Line segment ends at boundary -> ignore any hit, it will be handled by possibly following segments
        if( endsAtSegment && !halfOpen )
            continue;

        // Line segment starts at boundary -> generate a hit only if following that line would change the INSIDE/OUTSIDE
        // state. This should catch the case where a connected set of segments has a point directly on the boundary,
        // one segment not hitting it because it ends there and the next segment not hitting it because it starts there
        // Should NOT generate a hit if the segment only touches the boundary but turns around and stays inside.
        if( startsAtSegment )
        {
            IfcVector3 inside_dir = IfcVector3(b.y, -b.x, 0.0) * windingOrder;
            bool isGoingInside = (inside_dir * e) > 0.0;
            if( isGoingInside == isStartAssumedInside )
                continue;

            // only insert the point into the list if it is sufficiently far away from the previous intersection point.
            // This way, we avoid duplicate detection if the intersection is directly on the vertex between two segments.
            if( !intersect_results.empty() && intersect_results.back().first == i - 1 )
            {
                const IfcVector3 diff = intersect_results.back().second - e0;
                if( IfcVector2(diff.x, diff.y).SquareLength() < 1e-10 )
                    continue;
            }
            intersect_results.push_back(std::make_pair(i, e0));
            continue;
        }

        // for a valid intersection, s and t should be in range [0,1]. Including a bit of epsilon on s, potential double
        // hits on two consecutive boundary segments are filtered
        if( s >= -1e-6 * b_sqlen_inv && s <= 1.0 + 1e-6*b_sqlen_inv && t >= 0.0 && (t <= 1.0 || halfOpen) )
        {
            // only insert the point into the list if it is sufficiently far away from the previous intersection point.
            // This way, we avoid duplicate detection if the intersection is directly on the vertex between two segments.
            if( !intersect_results.empty() && intersect_results.back().first == i - 1 )
            {
                const IfcVector3 diff = intersect_results.back().second - p;
                if( IfcVector2(diff.x, diff.y).SquareLength() < 1e-10 )
                    continue;
            }
            intersect_results.push_back(std::make_pair(i, p));
        }
    }

    return !intersect_results.empty();
}


// ------------------------------------------------------------------------------------------------
// note: this functions works on 3D vectors, but performs its intersection checks solely in xy.
bool PointInPoly(const IfcVector3& p, const std::vector<IfcVector3>& boundary)
{
    // even-odd algorithm: take a random vector that extends from p to infinite
    // and counts how many times it intersects edges of the boundary.
    // because checking for segment intersections is prone to numeric inaccuracies
    // or double detections (i.e. when hitting multiple adjacent segments at their
    // shared vertices) we do it thrice with different rays and vote on it.

    // the even-odd algorithm doesn't work for points which lie directly on
    // the border of the polygon. If any of our attempts produces this result,
    // we return false immediately.

    std::vector<std::pair<size_t, IfcVector3> > intersected_boundary;
    size_t votes = 0;

    IntersectsBoundaryProfile(p, p + IfcVector3(1.0, 0, 0), boundary, true, intersected_boundary, true);
    votes += intersected_boundary.size() % 2;

    intersected_boundary.clear();
    IntersectsBoundaryProfile(p, p + IfcVector3(0, 1.0, 0), boundary, true, intersected_boundary, true);
    votes += intersected_boundary.size() % 2;

    intersected_boundary.clear();
    IntersectsBoundaryProfile(p, p + IfcVector3(0.6, -0.6, 0.0), boundary, true, intersected_boundary, true);
    votes += intersected_boundary.size() % 2;

    return votes > 1;
}


// ------------------------------------------------------------------------------------------------
void ProcessPolygonalBoundedBooleanHalfSpaceDifference(const Schema_2x3::IfcPolygonalBoundedHalfSpace* hs, TempMesh& result,
                                                       const TempMesh& first_operand,
                                                       ConversionData& conv)
{
    ai_assert(hs != NULL);

    const Schema_2x3::IfcPlane* const plane = hs->BaseSurface->ToPtr<Schema_2x3::IfcPlane>();
    if(!plane) {
        IFCImporter::LogError("expected IfcPlane as base surface for the IfcHalfSpaceSolid");
        return;
    }

    // extract plane base position vector and normal vector
    IfcVector3 p,n(0.f,0.f,1.f);
    if (plane->Position->Axis) {
        ConvertDirection(n,plane->Position->Axis.Get());
    }
    ConvertCartesianPoint(p,plane->Position->Location);

    if(!IsTrue(hs->AgreementFlag)) {
        n *= -1.f;
    }

    n.Normalize();

    // obtain the polygonal bounding volume
    std::shared_ptr<TempMesh> profile = std::shared_ptr<TempMesh>(new TempMesh());
    if(!ProcessCurve(hs->PolygonalBoundary, *profile.get(), conv)) {
        IFCImporter::LogError("expected valid polyline for boundary of boolean halfspace");
        return;
    }

    // determine winding order by calculating the normal.
    IfcVector3 profileNormal = TempMesh::ComputePolygonNormal(profile->mVerts.data(), profile->mVerts.size());

    IfcMatrix4 proj_inv;
    ConvertAxisPlacement(proj_inv,hs->Position);

    // and map everything into a plane coordinate space so all intersection
    // tests can be done in 2D space.
    IfcMatrix4 proj = proj_inv;
    proj.Inverse();

    // clip the current contents of `meshout` against the plane we obtained from the second operand
    const std::vector<IfcVector3>& in = first_operand.mVerts;
    std::vector<IfcVector3>& outvert = result.mVerts;
    std::vector<unsigned int>& outvertcnt = result.mVertcnt;

    outvert.reserve(in.size());
    outvertcnt.reserve(first_operand.mVertcnt.size());

    unsigned int vidx = 0;
    std::vector<unsigned int>::const_iterator begin = first_operand.mVertcnt.begin();
    std::vector<unsigned int>::const_iterator end = first_operand.mVertcnt.end();
    std::vector<unsigned int>::const_iterator iit;
    for( iit = begin; iit != end; vidx += *iit++ )
    {
        // Our new approach: we cut the poly along the plane, then we intersect the part on the black side of the plane
        // against the bounding polygon. All the white parts, and the black part outside the boundary polygon, are kept.
        std::vector<IfcVector3> whiteside, blackside;

        {
            const IfcVector3* srcVertices = &in[vidx];
            const size_t srcVtxCount = *iit;
            if( srcVtxCount == 0 )
                continue;

            IfcVector3 polyNormal = TempMesh::ComputePolygonNormal(srcVertices, srcVtxCount, true);

            // if the poly is parallel to the plane, put it completely on the black or white side
            if( std::abs(polyNormal * n) > 0.9999 )
            {
                bool isOnWhiteSide = (srcVertices[0] - p) * n > -1e-6;
                std::vector<IfcVector3>& targetSide = isOnWhiteSide ? whiteside : blackside;
                targetSide.insert(targetSide.end(), srcVertices, srcVertices + srcVtxCount);
            }
            else
            {
                // otherwise start building one polygon for each side. Whenever the current line segment intersects the plane
                // we put a point there as an end of the current segment. Then we switch to the other side, put a point there, too,
                // as a beginning of the current segment, and simply continue accumulating vertices.
                bool isCurrentlyOnWhiteSide = ((srcVertices[0]) - p) * n > -1e-6;
                for( size_t a = 0; a < srcVtxCount; ++a )
                {
                    IfcVector3 e0 = srcVertices[a];
                    IfcVector3 e1 = srcVertices[(a + 1) % srcVtxCount];
                    IfcVector3 ei;

                    // put starting point to the current mesh
                    std::vector<IfcVector3>& trgt = isCurrentlyOnWhiteSide ? whiteside : blackside;
                    trgt.push_back(srcVertices[a]);

                    // if there's an intersection, put an end vertex there, switch to the other side's mesh,
                    // and add a starting vertex there, too
                    bool isPlaneHit = IntersectSegmentPlane(p, n, e0, e1, isCurrentlyOnWhiteSide, ei);
                    if( isPlaneHit )
                    {
                        if( trgt.empty() || (trgt.back() - ei).SquareLength() > 1e-12 )
                            trgt.push_back(ei);
                        isCurrentlyOnWhiteSide = !isCurrentlyOnWhiteSide;
                        std::vector<IfcVector3>& newtrgt = isCurrentlyOnWhiteSide ? whiteside : blackside;
                        newtrgt.push_back(ei);
                    }
                }
            }
        }

        // the part on the white side can be written into the target mesh right away
        WritePolygon(whiteside, result);

        // The black part is the piece we need to get rid of, but only the part of it within the boundary polygon.
        // So we now need to construct all the polygons that result from BlackSidePoly minus BoundaryPoly.
        FilterPolygon(blackside);

        // Complicated, II. We run along the polygon. a) When we're inside the boundary, we run on until we hit an
        // intersection, which means we're leaving it. We then start a new out poly there. b) When we're outside the
        // boundary, we start collecting vertices until we hit an intersection, then we run along the boundary until we hit
        // an intersection, then we switch back to the poly and run on on this one again, and so on until we got a closed
        // loop. Then we continue with the path we left to catch potential additional polys on the other side of the
        // boundary as described in a)
        if( !blackside.empty() )
        {
            // poly edge index, intersection point, edge index in boundary poly
            std::vector<std::tuple<size_t, IfcVector3, size_t> > intersections;
            bool startedInside = PointInPoly(proj * blackside.front(), profile->mVerts);
            bool isCurrentlyInside = startedInside;

            std::vector<std::pair<size_t, IfcVector3> > intersected_boundary;

            for( size_t a = 0; a < blackside.size(); ++a )
            {
                const IfcVector3 e0 = proj * blackside[a];
                const IfcVector3 e1 = proj * blackside[(a + 1) % blackside.size()];

                intersected_boundary.clear();
                IntersectsBoundaryProfile(e0, e1, profile->mVerts, isCurrentlyInside, intersected_boundary);
                // sort the hits by distance from e0 to get the correct in/out/in sequence. Manually :-( I miss you, C++11.
                if( intersected_boundary.size() > 1 )
                {
                    bool keepSorting = true;
                    while( keepSorting )
                    {
                        keepSorting = false;
                        for( size_t b = 0; b < intersected_boundary.size() - 1; ++b )
                        {
                            if( (intersected_boundary[b + 1].second - e0).SquareLength() < (intersected_boundary[b].second - e0).SquareLength() )
                            {
                                keepSorting = true;
                                std::swap(intersected_boundary[b + 1], intersected_boundary[b]);
                            }
                        }
                    }
                }
                // now add them to the list of intersections
                for( size_t b = 0; b < intersected_boundary.size(); ++b )
                    intersections.push_back(std::make_tuple(a, proj_inv * intersected_boundary[b].second, intersected_boundary[b].first));

                // and calculate our new inside/outside state
                if( intersected_boundary.size() & 1 )
                    isCurrentlyInside = !isCurrentlyInside;
            }

            // we got a list of in-out-combinations of intersections. That should be an even number of intersections, or
            // we're fucked.
            if( (intersections.size() & 1) != 0 )
            {
                IFCImporter::LogWarn("Odd number of intersections, can't work with that. Omitting half space boundary check.");
                continue;
            }

            if( intersections.size() > 1 )
            {
                // If we started outside, the first intersection is a out->in intersection. Cycle them so that it
                // starts with an intersection leaving the boundary
                if( !startedInside )
                for( size_t b = 0; b < intersections.size() - 1; ++b )
                    std::swap(intersections[b], intersections[(b + intersections.size() - 1) % intersections.size()]);

                // Filter pairs of out->in->out that lie too close to each other.
                for( size_t a = 0; intersections.size() > 0 && a < intersections.size() - 1; /**/ )
                {
                    if( (std::get<1>(intersections[a]) - std::get<1>(intersections[(a + 1) % intersections.size()])).SquareLength() < 1e-10 )
                        intersections.erase(intersections.begin() + a, intersections.begin() + a + 2);
                    else
                        a++;
                }
                if( intersections.size() > 1 && (std::get<1>(intersections.back()) - std::get<1>(intersections.front())).SquareLength() < 1e-10 )
                {
                    intersections.pop_back(); intersections.erase(intersections.begin());
                }
            }


            // no intersections at all: either completely inside the boundary, so everything gets discarded, or completely outside.
            // in the latter case we're implementional lost. I'm simply going to ignore this, so a large poly will not get any
            // holes if the boundary is smaller and does not touch it anywhere.
            if( intersections.empty() )
            {
                // starting point was outside -> everything is outside the boundary -> nothing is clipped -> add black side
                // to result mesh unchanged
                if( !startedInside )
                {
                    outvertcnt.push_back(static_cast<unsigned int>(blackside.size()));
                    outvert.insert(outvert.end(), blackside.begin(), blackside.end());
                    continue;
                }
                else
                {
                    // starting point was inside the boundary -> everything is inside the boundary -> nothing is spared from the
                    // clipping -> nothing left to add to the result mesh
                    continue;
                }
            }

            // determine the direction in which we're marching along the boundary polygon. If the src poly is faced upwards
            // and the boundary is also winded this way, we need to march *backwards* on the boundary.
            const IfcVector3 polyNormal = IfcMatrix3(proj) * TempMesh::ComputePolygonNormal(blackside.data(), blackside.size());
            bool marchBackwardsOnBoundary = (profileNormal * polyNormal) >= 0.0;

            // Build closed loops from these intersections. Starting from an intersection leaving the boundary we
            // walk along the polygon to the next intersection (which should be an IS entering the boundary poly).
            // From there we walk along the boundary until we hit another intersection leaving the boundary,
            // walk along the poly to the next IS and so on until we're back at the starting point.
            // We remove every intersection we "used up", so any remaining intersection is the start of a new loop.
            while( !intersections.empty() )
            {
                std::vector<IfcVector3> resultpoly;
                size_t currentIntersecIdx = 0;

                while( true )
                {
                    ai_assert(intersections.size() > currentIntersecIdx + 1);
                    std::tuple<size_t, IfcVector3, size_t> currintsec = intersections[currentIntersecIdx + 0];
                    std::tuple<size_t, IfcVector3, size_t> nextintsec = intersections[currentIntersecIdx + 1];
                    intersections.erase(intersections.begin() + currentIntersecIdx, intersections.begin() + currentIntersecIdx + 2);

                    // we start with an in->out intersection
                    resultpoly.push_back(std::get<1>(currintsec));
                    // climb along the polygon to the next intersection, which should be an out->in
                    size_t numPolyPoints = (std::get<0>(currintsec) > std::get<0>(nextintsec) ? blackside.size() : 0)
                        + std::get<0>(nextintsec) - std::get<0>(currintsec);
                    for( size_t a = 1; a <= numPolyPoints; ++a )
                        resultpoly.push_back(blackside[(std::get<0>(currintsec) + a) % blackside.size()]);
                    // put the out->in intersection
                    resultpoly.push_back(std::get<1>(nextintsec));

                    // generate segments along the boundary polygon that lie in the poly's plane until we hit another intersection
                    IfcVector3 startingPoint = proj * std::get<1>(nextintsec);
                    size_t currentBoundaryEdgeIdx = (std::get<2>(nextintsec) + (marchBackwardsOnBoundary ? 1 : 0)) % profile->mVerts.size();
                    size_t nextIntsecIdx = SIZE_MAX;
                    while( nextIntsecIdx == SIZE_MAX )
                    {
                        IfcFloat t = 1e10;

                        size_t nextBoundaryEdgeIdx = marchBackwardsOnBoundary ? (currentBoundaryEdgeIdx + profile->mVerts.size() - 1) : currentBoundaryEdgeIdx + 1;
                        nextBoundaryEdgeIdx %= profile->mVerts.size();
                        // vertices of the current boundary segments
                        IfcVector3 currBoundaryPoint = profile->mVerts[currentBoundaryEdgeIdx];
                        IfcVector3 nextBoundaryPoint = profile->mVerts[nextBoundaryEdgeIdx];
                        // project the two onto the polygon
                        if( std::abs(polyNormal.z) > 1e-5 )
                        {
                            currBoundaryPoint.z = startingPoint.z + (currBoundaryPoint.x - startingPoint.x) * polyNormal.x/polyNormal.z + (currBoundaryPoint.y - startingPoint.y) * polyNormal.y/polyNormal.z;
                            nextBoundaryPoint.z = startingPoint.z + (nextBoundaryPoint.x - startingPoint.x) * polyNormal.x/polyNormal.z + (nextBoundaryPoint.y - startingPoint.y) * polyNormal.y/polyNormal.z;
                        }

                        // build a direction that goes along the boundary border but lies in the poly plane
                        IfcVector3 boundaryPlaneNormal = ((nextBoundaryPoint - currBoundaryPoint) ^ profileNormal).Normalize();
                        IfcVector3 dirAtPolyPlane = (boundaryPlaneNormal ^ polyNormal).Normalize() * (marchBackwardsOnBoundary ? -1.0 : 1.0);
                        // if we can project the direction to the plane, we can calculate a maximum marching distance along that dir
                        // until we finish that boundary segment and continue on the next
                        if( std::abs(polyNormal.z) > 1e-5 )
                        {
                            t = std::min(t, (nextBoundaryPoint - startingPoint).Length());
                        }

                        // check if the direction hits the loop start - if yes, we got a poly to output
                        IfcVector3 dirToThatPoint = proj * resultpoly.front() - startingPoint;
                        IfcFloat tpt = dirToThatPoint * dirAtPolyPlane;
                        if( tpt > -1e-6 && tpt <= t && (dirToThatPoint - tpt * dirAtPolyPlane).SquareLength() < 1e-10 )
                        {
                            nextIntsecIdx = intersections.size(); // dirty hack to end marching along the boundary and signal the end of the loop
                            t = tpt;
                        }

                        // also check if the direction hits any in->out intersections earlier. If we hit one, we can switch back
                        // to marching along the poly border from that intersection point
                        for( size_t a = 0; a < intersections.size(); a += 2 )
                        {
                            dirToThatPoint = proj * std::get<1>(intersections[a]) - startingPoint;
                            tpt = dirToThatPoint * dirAtPolyPlane;
                            if( tpt > -1e-6 && tpt <= t && (dirToThatPoint - tpt * dirAtPolyPlane).SquareLength() < 1e-10 )
                            {
                                nextIntsecIdx = a; // switch back to poly and march on from this in->out intersection
                                t = tpt;
                            }
                        }

                        // if we keep marching on the boundary, put the segment end point to the result poly and well... keep marching
                        if( nextIntsecIdx == SIZE_MAX )
                        {
                            resultpoly.push_back(proj_inv * nextBoundaryPoint);
                            currentBoundaryEdgeIdx = nextBoundaryEdgeIdx;
                            startingPoint = nextBoundaryPoint;
                        }

                        // quick endless loop check
                        if( resultpoly.size() > blackside.size() + profile->mVerts.size() )
                        {
                            IFCImporter::LogError("Encountered endless loop while clipping polygon against poly-bounded half space.");
                            break;
                        }
                    }

                    // we're back on the poly - if this is the intersection we started from, we got a closed loop.
                    if( nextIntsecIdx >= intersections.size() )
                    {
                        break;
                    }

                    // otherwise it's another intersection. Continue marching from there.
                    currentIntersecIdx = nextIntsecIdx;
                }

                WritePolygon(resultpoly, result);
            }
        }
    }
    IFCImporter::LogDebug("generating CSG geometry by plane clipping with polygonal bounding (IfcBooleanClippingResult)");
}

// ------------------------------------------------------------------------------------------------
void ProcessBooleanExtrudedAreaSolidDifference(const Schema_2x3::IfcExtrudedAreaSolid* as, TempMesh& result,
                                               const TempMesh& first_operand,
                                               ConversionData& conv)
{
    ai_assert(as != NULL);

    // This case is handled by reduction to an instance of the quadrify() algorithm.
    // Obviously, this won't work for arbitrarily complex cases. In fact, the first
    // operand should be near-planar. Luckily, this is usually the case in Ifc
    // buildings.

    std::shared_ptr<TempMesh> meshtmp = std::shared_ptr<TempMesh>(new TempMesh());
    ProcessExtrudedAreaSolid(*as,*meshtmp,conv,false);

    std::vector<TempOpening> openings(1, TempOpening(as,IfcVector3(0,0,0),meshtmp,std::shared_ptr<TempMesh>()));

    result = first_operand;

    TempMesh temp;

    std::vector<IfcVector3>::const_iterator vit = first_operand.mVerts.begin();
    for(unsigned int pcount : first_operand.mVertcnt) {
        temp.Clear();

        temp.mVerts.insert(temp.mVerts.end(), vit, vit + pcount);
        temp.mVertcnt.push_back(pcount);

        // The algorithms used to generate mesh geometry sometimes
        // spit out lines or other degenerates which must be
        // filtered to avoid running into assertions later on.

        // ComputePolygonNormal returns the Newell normal, so the
        // length of the normal is the area of the polygon.
        const IfcVector3& normal = temp.ComputeLastPolygonNormal(false);
        if (normal.SquareLength() < static_cast<IfcFloat>(1e-5)) {
            IFCImporter::LogWarn("skipping degenerate polygon (ProcessBooleanExtrudedAreaSolidDifference)");
            continue;
        }

        GenerateOpenings(openings, std::vector<IfcVector3>(1,IfcVector3(1,0,0)), temp, false, true);
        result.Append(temp);

        vit += pcount;
    }

    IFCImporter::LogDebug("generating CSG geometry by geometric difference to a solid (IfcExtrudedAreaSolid)");
}

// ------------------------------------------------------------------------------------------------
void ProcessBoolean(const Schema_2x3::IfcBooleanResult& boolean, TempMesh& result, ConversionData& conv)
{
    // supported CSG operations:
    //   DIFFERENCE
    if(const Schema_2x3::IfcBooleanResult* const clip = boolean.ToPtr<Schema_2x3::IfcBooleanResult>()) {
        if(clip->Operator != "DIFFERENCE") {
            IFCImporter::LogWarn("encountered unsupported boolean operator: " + (std::string)clip->Operator);
            return;
        }

        // supported cases (1st operand):
        //  IfcBooleanResult -- call ProcessBoolean recursively
        //  IfcSweptAreaSolid -- obtain polygonal geometry first

        // supported cases (2nd operand):
        //  IfcHalfSpaceSolid -- easy, clip against plane
        //  IfcExtrudedAreaSolid -- reduce to an instance of the quadrify() algorithm


        const Schema_2x3::IfcHalfSpaceSolid* const hs = clip->SecondOperand->ResolveSelectPtr<Schema_2x3::IfcHalfSpaceSolid>(conv.db);
        const Schema_2x3::IfcExtrudedAreaSolid* const as = clip->SecondOperand->ResolveSelectPtr<Schema_2x3::IfcExtrudedAreaSolid>(conv.db);
        if(!hs && !as) {
            IFCImporter::LogError("expected IfcHalfSpaceSolid or IfcExtrudedAreaSolid as second clipping operand");
            return;
        }

        TempMesh first_operand;
        if(const Schema_2x3::IfcBooleanResult* const op0 = clip->FirstOperand->ResolveSelectPtr<Schema_2x3::IfcBooleanResult>(conv.db)) {
            ProcessBoolean(*op0,first_operand,conv);
        }
        else if (const Schema_2x3::IfcSweptAreaSolid* const swept = clip->FirstOperand->ResolveSelectPtr<Schema_2x3::IfcSweptAreaSolid>(conv.db)) {
            ProcessSweptAreaSolid(*swept,first_operand,conv);
        }
        else {
            IFCImporter::LogError("expected IfcSweptAreaSolid or IfcBooleanResult as first clipping operand");
            return;
        }

        if(hs) {

            const Schema_2x3::IfcPolygonalBoundedHalfSpace* const hs_bounded = clip->SecondOperand->ResolveSelectPtr<Schema_2x3::IfcPolygonalBoundedHalfSpace>(conv.db);
            if (hs_bounded) {
                ProcessPolygonalBoundedBooleanHalfSpaceDifference(hs_bounded, result, first_operand, conv);
            }
            else {
                ProcessBooleanHalfSpaceDifference(hs, result, first_operand, conv);
            }
        }
        else {
            ProcessBooleanExtrudedAreaSolidDifference(as, result, first_operand, conv);
        }
    }
    else {
        IFCImporter::LogWarn("skipping unknown IfcBooleanResult entity, type is " + boolean.GetClassName());
    }
}

} // ! IFC
} // ! Assimp

#endif

