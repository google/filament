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

/** @file  IFCProfile.cpp
 *  @brief Read profile and curves entities from IFC files
 */

#ifndef ASSIMP_BUILD_NO_IFC_IMPORTER
#include "IFCUtil.h"

namespace Assimp {
namespace IFC {
namespace {


// --------------------------------------------------------------------------------
// Conic is the base class for Circle and Ellipse
// --------------------------------------------------------------------------------
class Conic : public Curve {
public:
    // --------------------------------------------------
    Conic(const Schema_2x3::IfcConic& entity, ConversionData& conv)
    : Curve(entity,conv) {
        IfcMatrix4 trafo;
        ConvertAxisPlacement(trafo,*entity.Position,conv);

        // for convenience, extract the matrix rows
        location = IfcVector3(trafo.a4,trafo.b4,trafo.c4);
        p[0] = IfcVector3(trafo.a1,trafo.b1,trafo.c1);
        p[1] = IfcVector3(trafo.a2,trafo.b2,trafo.c2);
        p[2] = IfcVector3(trafo.a3,trafo.b3,trafo.c3);
    }

    // --------------------------------------------------
    bool IsClosed() const {
        return true;
    }

    // --------------------------------------------------
    size_t EstimateSampleCount(IfcFloat a, IfcFloat b) const {
        ai_assert( InRange( a ) );
        ai_assert( InRange( b ) );

        a *= conv.angle_scale;
        b *= conv.angle_scale;

        a = std::fmod(a,static_cast<IfcFloat>( AI_MATH_TWO_PI ));
        b = std::fmod(b,static_cast<IfcFloat>( AI_MATH_TWO_PI ));
        const IfcFloat setting = static_cast<IfcFloat>( AI_MATH_PI * conv.settings.conicSamplingAngle / 180.0 );
        return static_cast<size_t>( std::ceil(std::abs( b-a)) / setting);
    }

    // --------------------------------------------------
    ParamRange GetParametricRange() const {
        return std::make_pair(static_cast<IfcFloat>( 0. ), static_cast<IfcFloat>( AI_MATH_TWO_PI / conv.angle_scale ));
    }

protected:
    IfcVector3 location, p[3];
};

// --------------------------------------------------------------------------------
// Circle
// --------------------------------------------------------------------------------
class Circle : public Conic {
public:
    // --------------------------------------------------
    Circle(const Schema_2x3::IfcCircle& entity, ConversionData& conv)
        : Conic(entity,conv)
        , entity(entity)
    {
    }

    // --------------------------------------------------
    IfcVector3 Eval(IfcFloat u) const {
        u = -conv.angle_scale * u;
        return location + static_cast<IfcFloat>(entity.Radius)*(static_cast<IfcFloat>(std::cos(u))*p[0] +
            static_cast<IfcFloat>(std::sin(u))*p[1]);
    }

private:
    const Schema_2x3::IfcCircle& entity;
};

// --------------------------------------------------------------------------------
// Ellipse
// --------------------------------------------------------------------------------
class Ellipse : public Conic {
public:
    // --------------------------------------------------
    Ellipse(const Schema_2x3::IfcEllipse& entity, ConversionData& conv)
    : Conic(entity,conv)
    , entity(entity) {
        // empty
    }

    // --------------------------------------------------
    IfcVector3 Eval(IfcFloat u) const {
        u = -conv.angle_scale * u;
        return location + static_cast<IfcFloat>(entity.SemiAxis1)*static_cast<IfcFloat>(std::cos(u))*p[0] +
            static_cast<IfcFloat>(entity.SemiAxis2)*static_cast<IfcFloat>(std::sin(u))*p[1];
    }

private:
    const Schema_2x3::IfcEllipse& entity;
};

// --------------------------------------------------------------------------------
// Line
// --------------------------------------------------------------------------------
class Line : public Curve {
public:
    // --------------------------------------------------
    Line(const Schema_2x3::IfcLine& entity, ConversionData& conv)
    : Curve(entity,conv) {
        ConvertCartesianPoint(p,entity.Pnt);
        ConvertVector(v,entity.Dir);
    }

    // --------------------------------------------------
    bool IsClosed() const {
        return false;
    }

    // --------------------------------------------------
    IfcVector3 Eval(IfcFloat u) const {
        return p + u*v;
    }

    // --------------------------------------------------
    size_t EstimateSampleCount(IfcFloat a, IfcFloat b) const {
        ai_assert( InRange( a ) );
        ai_assert( InRange( b ) );
        // two points are always sufficient for a line segment
        return a==b ? 1 : 2;
    }


    // --------------------------------------------------
    void SampleDiscrete(TempMesh& out,IfcFloat a, IfcFloat b) const {
        ai_assert( InRange( a ) );
        ai_assert( InRange( b ) );

        if (a == b) {
            out.mVerts.push_back(Eval(a));
            return;
        }
        out.mVerts.reserve(out.mVerts.size()+2);
        out.mVerts.push_back(Eval(a));
        out.mVerts.push_back(Eval(b));
    }

    // --------------------------------------------------
    ParamRange GetParametricRange() const {
        const IfcFloat inf = std::numeric_limits<IfcFloat>::infinity();

        return std::make_pair(-inf,+inf);
    }

private:
    IfcVector3 p,v;
};

// --------------------------------------------------------------------------------
// CompositeCurve joins multiple smaller, bounded curves
// --------------------------------------------------------------------------------
class CompositeCurve : public BoundedCurve {
    typedef std::pair< std::shared_ptr< BoundedCurve >, bool > CurveEntry;

public:
    // --------------------------------------------------
    CompositeCurve(const Schema_2x3::IfcCompositeCurve& entity, ConversionData& conv)
    : BoundedCurve(entity,conv)
    , total() {
        curves.reserve(entity.Segments.size());
        for(const Schema_2x3::IfcCompositeCurveSegment& curveSegment :entity.Segments) {
            // according to the specification, this must be a bounded curve
            std::shared_ptr< Curve > cv(Curve::Convert(curveSegment.ParentCurve,conv));
            std::shared_ptr< BoundedCurve > bc = std::dynamic_pointer_cast<BoundedCurve>(cv);

            if (!bc) {
                IFCImporter::LogError("expected segment of composite curve to be a bounded curve");
                continue;
            }

            if ( (std::string)curveSegment.Transition != "CONTINUOUS" ) {
                IFCImporter::LogDebug("ignoring transition code on composite curve segment, only continuous transitions are supported");
            }

            curves.push_back( CurveEntry(bc,IsTrue(curveSegment.SameSense)) );
            total += bc->GetParametricRangeDelta();
        }

        if (curves.empty()) {
            throw CurveError("empty composite curve");
        }
    }

    // --------------------------------------------------
    IfcVector3 Eval(IfcFloat u) const {
        if (curves.empty()) {
            return IfcVector3();
        }

        IfcFloat acc = 0;
        for(const CurveEntry& entry : curves) {
            const ParamRange& range = entry.first->GetParametricRange();
            const IfcFloat delta = std::abs(range.second-range.first);
            if (u < acc+delta) {
                return entry.first->Eval( entry.second ? (u-acc) + range.first : range.second-(u-acc));
            }

            acc += delta;
        }
        // clamp to end
        return curves.back().first->Eval(curves.back().first->GetParametricRange().second);
    }

    // --------------------------------------------------
    size_t EstimateSampleCount(IfcFloat a, IfcFloat b) const {
        ai_assert( InRange( a ) );
        ai_assert( InRange( b ) );
        size_t cnt = 0;

        IfcFloat acc = 0;
        for(const CurveEntry& entry : curves) {
            const ParamRange& range = entry.first->GetParametricRange();
            const IfcFloat delta = std::abs(range.second-range.first);
            if (a <= acc+delta && b >= acc) {
                const IfcFloat at =  std::max(static_cast<IfcFloat>( 0. ),a-acc), bt = std::min(delta,b-acc);
                cnt += entry.first->EstimateSampleCount( entry.second ? at + range.first : range.second - bt, entry.second ? bt + range.first : range.second - at );
            }

            acc += delta;
        }

        return cnt;
    }

    // --------------------------------------------------
    void SampleDiscrete(TempMesh& out,IfcFloat a, IfcFloat b) const {
        ai_assert( InRange( a ) );
        ai_assert( InRange( b ) );

        const size_t cnt = EstimateSampleCount(a,b);
        out.mVerts.reserve(out.mVerts.size() + cnt);

        for(const CurveEntry& entry : curves) {
            const size_t cnt = out.mVerts.size();
            entry.first->SampleDiscrete(out);

            if (!entry.second && cnt != out.mVerts.size()) {
                std::reverse(out.mVerts.begin()+cnt,out.mVerts.end());
            }
        }
    }

    // --------------------------------------------------
    ParamRange GetParametricRange() const {
        return std::make_pair(static_cast<IfcFloat>( 0. ),total);
    }

private:
    std::vector< CurveEntry > curves;
    IfcFloat total;
};

// --------------------------------------------------------------------------------
// TrimmedCurve can be used to trim an unbounded curve to a bounded range
// --------------------------------------------------------------------------------
class TrimmedCurve : public BoundedCurve {
public:
    // --------------------------------------------------
    TrimmedCurve(const Schema_2x3::IfcTrimmedCurve& entity, ConversionData& conv)
        : BoundedCurve(entity,conv),
          base(std::shared_ptr<const Curve>(Curve::Convert(entity.BasisCurve,conv)))
    {
        typedef std::shared_ptr<const STEP::EXPRESS::DataType> Entry;

        // for some reason, trimmed curves can either specify a parametric value
        // or a point on the curve, or both. And they can even specify which of the
        // two representations they prefer, even though an information invariant
        // claims that they must be identical if both are present.
        // oh well.
        bool have_param = false, have_point = false;
        IfcVector3 point;
        for(const Entry sel :entity.Trim1) {
            if (const ::Assimp::STEP::EXPRESS::REAL* const r = sel->ToPtr<::Assimp::STEP::EXPRESS::REAL>()) {
                range.first = *r;
                have_param = true;
                break;
            }
            else if (const Schema_2x3::IfcCartesianPoint* const r = sel->ResolveSelectPtr<Schema_2x3::IfcCartesianPoint>(conv.db)) {
                ConvertCartesianPoint(point,*r);
                have_point = true;
            }
        }
        if (!have_param) {
            if (!have_point || !base->ReverseEval(point,range.first)) {
                throw CurveError("IfcTrimmedCurve: failed to read first trim parameter, ignoring curve");
            }
        }
        have_param = false, have_point = false;
        for(const Entry sel :entity.Trim2) {
            if (const ::Assimp::STEP::EXPRESS::REAL* const r = sel->ToPtr<::Assimp::STEP::EXPRESS::REAL>()) {
                range.second = *r;
                have_param = true;
                break;
            }
            else if (const Schema_2x3::IfcCartesianPoint* const r = sel->ResolveSelectPtr<Schema_2x3::IfcCartesianPoint>(conv.db)) {
                ConvertCartesianPoint(point,*r);
                have_point = true;
            }
        }
        if (!have_param) {
            if (!have_point || !base->ReverseEval(point,range.second)) {
                throw CurveError("IfcTrimmedCurve: failed to read second trim parameter, ignoring curve");
            }
        }

        agree_sense = IsTrue(entity.SenseAgreement);
        if( !agree_sense ) {
            std::swap(range.first,range.second);
        }

        // "NOTE In case of a closed curve, it may be necessary to increment t1 or t2
        // by the parametric length for consistency with the sense flag."
        if (base->IsClosed()) {
            if( range.first > range.second ) {
                range.second += base->GetParametricRangeDelta();
            }
        }

        maxval = range.second-range.first;
        ai_assert(maxval >= 0);
    }

    // --------------------------------------------------
    IfcVector3 Eval(IfcFloat p) const {
        ai_assert(InRange(p));
        return base->Eval( TrimParam(p) );
    }

    // --------------------------------------------------
    size_t EstimateSampleCount(IfcFloat a, IfcFloat b) const {
        ai_assert( InRange( a ) );
        ai_assert( InRange( b ) );
        return base->EstimateSampleCount(TrimParam(a),TrimParam(b));
    }

    // --------------------------------------------------
    void SampleDiscrete(TempMesh& out,IfcFloat a,IfcFloat b) const {
        ai_assert(InRange(a) && InRange(b));
        return base->SampleDiscrete(out,TrimParam(a),TrimParam(b));
    }

    // --------------------------------------------------
    ParamRange GetParametricRange() const {
        return std::make_pair(static_cast<IfcFloat>( 0. ),maxval);
    }

private:
    // --------------------------------------------------
    IfcFloat TrimParam(IfcFloat f) const {
        return agree_sense ? f + range.first :  range.second - f;
    }

private:
    ParamRange range;
    IfcFloat maxval;
    bool agree_sense;

    std::shared_ptr<const Curve> base;
};


// --------------------------------------------------------------------------------
// PolyLine is a 'curve' defined by linear interpolation over a set of discrete points
// --------------------------------------------------------------------------------
class PolyLine : public BoundedCurve {
public:
    // --------------------------------------------------
    PolyLine(const Schema_2x3::IfcPolyline& entity, ConversionData& conv)
        : BoundedCurve(entity,conv)
    {
        points.reserve(entity.Points.size());

        IfcVector3 t;
        for(const Schema_2x3::IfcCartesianPoint& cp : entity.Points) {
            ConvertCartesianPoint(t,cp);
            points.push_back(t);
        }
    }

    // --------------------------------------------------
    IfcVector3 Eval(IfcFloat p) const {
        ai_assert(InRange(p));

        const size_t b = static_cast<size_t>(std::floor(p));
        if (b == points.size()-1) {
            return points.back();
        }

        const IfcFloat d = p-static_cast<IfcFloat>(b);
        return points[b+1] * d + points[b] * (static_cast<IfcFloat>( 1. )-d);
    }

    // --------------------------------------------------
    size_t EstimateSampleCount(IfcFloat a, IfcFloat b) const {
        ai_assert(InRange(a) && InRange(b));
        return static_cast<size_t>( std::ceil(b) - std::floor(a) );
    }

    // --------------------------------------------------
    ParamRange GetParametricRange() const {
        return std::make_pair(static_cast<IfcFloat>( 0. ),static_cast<IfcFloat>(points.size()-1));
    }

private:
    std::vector<IfcVector3> points;
};

} // anon

// ------------------------------------------------------------------------------------------------
Curve* Curve::Convert(const IFC::Schema_2x3::IfcCurve& curve,ConversionData& conv) {
    if(curve.ToPtr<Schema_2x3::IfcBoundedCurve>()) {
        if(const Schema_2x3::IfcPolyline* c = curve.ToPtr<Schema_2x3::IfcPolyline>()) {
            return new PolyLine(*c,conv);
        }
        if(const Schema_2x3::IfcTrimmedCurve* c = curve.ToPtr<Schema_2x3::IfcTrimmedCurve>()) {
            return new TrimmedCurve(*c,conv);
        }
        if(const Schema_2x3::IfcCompositeCurve* c = curve.ToPtr<Schema_2x3::IfcCompositeCurve>()) {
            return new CompositeCurve(*c,conv);
        }
    }

    if(curve.ToPtr<Schema_2x3::IfcConic>()) {
        if(const Schema_2x3::IfcCircle* c = curve.ToPtr<Schema_2x3::IfcCircle>()) {
            return new Circle(*c,conv);
        }
        if(const Schema_2x3::IfcEllipse* c = curve.ToPtr<Schema_2x3::IfcEllipse>()) {
            return new Ellipse(*c,conv);
        }
    }

    if(const Schema_2x3::IfcLine* c = curve.ToPtr<Schema_2x3::IfcLine>()) {
        return new Line(*c,conv);
    }

    // XXX OffsetCurve2D, OffsetCurve3D not currently supported
    return NULL;
}

#ifdef ASSIMP_BUILD_DEBUG
// ------------------------------------------------------------------------------------------------
bool Curve::InRange(IfcFloat u) const {
    const ParamRange range = GetParametricRange();
    if (IsClosed()) {
        return true;
    }
    const IfcFloat epsilon = Math::getEpsilon<float>();
    return u - range.first > -epsilon && range.second - u > -epsilon;
}
#endif

// ------------------------------------------------------------------------------------------------
IfcFloat Curve::GetParametricRangeDelta() const {
    const ParamRange& range = GetParametricRange();
    return std::abs(range.second - range.first);
}

// ------------------------------------------------------------------------------------------------
size_t Curve::EstimateSampleCount(IfcFloat a, IfcFloat b) const {
    (void)(a); (void)(b);  
    ai_assert( InRange( a ) );
    ai_assert( InRange( b ) );

    // arbitrary default value, deriving classes should supply better suited values
    return 16;
}

// ------------------------------------------------------------------------------------------------
IfcFloat RecursiveSearch(const Curve* cv, const IfcVector3& val, IfcFloat a, IfcFloat b,
        unsigned int samples, IfcFloat threshold, unsigned int recurse = 0, unsigned int max_recurse = 15) {
    ai_assert(samples>1);

    const IfcFloat delta = (b-a)/samples, inf = std::numeric_limits<IfcFloat>::infinity();
    IfcFloat min_point[2] = {a,b}, min_diff[2] = {inf,inf};
    IfcFloat runner = a;

    for (unsigned int i = 0; i < samples; ++i, runner += delta) {
        const IfcFloat diff = (cv->Eval(runner)-val).SquareLength();
        if (diff < min_diff[0]) {
            min_diff[1] = min_diff[0];
            min_point[1] = min_point[0];

            min_diff[0] = diff;
            min_point[0] = runner;
        }
        else if (diff < min_diff[1]) {
            min_diff[1] = diff;
            min_point[1] = runner;
        }
    }

    ai_assert( min_diff[ 0 ] != inf );
    ai_assert( min_diff[ 1 ] != inf );
    if ( std::fabs(a-min_point[0]) < threshold || recurse >= max_recurse) {
        return min_point[0];
    }

    // fix for closed curves to take their wrap-over into account
    if (cv->IsClosed() && std::fabs(min_point[0]-min_point[1]) > cv->GetParametricRangeDelta()*0.5  ) {
        const Curve::ParamRange& range = cv->GetParametricRange();
        const IfcFloat wrapdiff = (cv->Eval(range.first)-val).SquareLength();

        if (wrapdiff < min_diff[0]) {
            const IfcFloat t = min_point[0];
            min_point[0] = min_point[1] > min_point[0] ? range.first : range.second;
             min_point[1] = t;
        }
    }

    return RecursiveSearch(cv,val,min_point[0],min_point[1],samples,threshold,recurse+1,max_recurse);
}

// ------------------------------------------------------------------------------------------------
bool Curve::ReverseEval(const IfcVector3& val, IfcFloat& paramOut) const
{
    // note: the following algorithm is not guaranteed to find the 'right' parameter value
    // in all possible cases, but it will always return at least some value so this function
    // will never fail in the default implementation.

    // XXX derive threshold from curve topology
    static const IfcFloat threshold = 1e-4f;
    static const unsigned int samples = 16;

    const ParamRange& range = GetParametricRange();
    paramOut = RecursiveSearch(this,val,range.first,range.second,samples,threshold);

    return true;
}

// ------------------------------------------------------------------------------------------------
void Curve::SampleDiscrete(TempMesh& out,IfcFloat a, IfcFloat b) const {
    ai_assert( InRange( a ) );
    ai_assert( InRange( b ) );

    const size_t cnt = std::max(static_cast<size_t>(0),EstimateSampleCount(a,b));
    out.mVerts.reserve( out.mVerts.size() + cnt + 1);

    IfcFloat p = a, delta = (b-a)/cnt;
    for(size_t i = 0; i <= cnt; ++i, p += delta) {
        out.mVerts.push_back(Eval(p));
    }
}

// ------------------------------------------------------------------------------------------------
bool BoundedCurve::IsClosed() const {
    return false;
}

// ------------------------------------------------------------------------------------------------
void BoundedCurve::SampleDiscrete(TempMesh& out) const {
    const ParamRange& range = GetParametricRange();
    ai_assert( range.first != std::numeric_limits<IfcFloat>::infinity() );
    ai_assert( range.second != std::numeric_limits<IfcFloat>::infinity() );

    return SampleDiscrete(out,range.first,range.second);
}

} // IFC
} // Assimp

#endif // ASSIMP_BUILD_NO_IFC_IMPORTER
