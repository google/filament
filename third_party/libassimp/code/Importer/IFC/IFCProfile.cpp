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

// ------------------------------------------------------------------------------------------------
void ProcessPolyLine(const Schema_2x3::IfcPolyline& def, TempMesh& meshout, ConversionData& /*conv*/)
{
    // this won't produce a valid mesh, it just spits out a list of vertices
    IfcVector3 t;
    for(const Schema_2x3::IfcCartesianPoint& cp : def.Points) {
        ConvertCartesianPoint(t,cp);
        meshout.mVerts.push_back(t);
    }
    meshout.mVertcnt.push_back(static_cast<unsigned int>(meshout.mVerts.size()));
}

// ------------------------------------------------------------------------------------------------
bool ProcessCurve(const Schema_2x3::IfcCurve& curve,  TempMesh& meshout, ConversionData& conv)
{
    std::unique_ptr<const Curve> cv(Curve::Convert(curve,conv));
    if (!cv) {
        IFCImporter::LogWarn("skipping unknown IfcCurve entity, type is " + curve.GetClassName());
        return false;
    }

    // we must have a bounded curve at this point
    if (const BoundedCurve* bc = dynamic_cast<const BoundedCurve*>(cv.get())) {
        try {
            bc->SampleDiscrete(meshout);
        }
        catch(const  CurveError& cv) {
            IFCImporter::LogError(cv.mStr + " (error occurred while processing curve)");
            return false;
        }
        meshout.mVertcnt.push_back(static_cast<unsigned int>(meshout.mVerts.size()));
        return true;
    }

    IFCImporter::LogError("cannot use unbounded curve as profile");
    return false;
}

// ------------------------------------------------------------------------------------------------
void ProcessClosedProfile(const Schema_2x3::IfcArbitraryClosedProfileDef& def, TempMesh& meshout, ConversionData& conv)
{
    ProcessCurve(def.OuterCurve,meshout,conv);
}

// ------------------------------------------------------------------------------------------------
void ProcessOpenProfile(const Schema_2x3::IfcArbitraryOpenProfileDef& def, TempMesh& meshout, ConversionData& conv)
{
    ProcessCurve(def.Curve,meshout,conv);
}

// ------------------------------------------------------------------------------------------------
void ProcessParametrizedProfile(const Schema_2x3::IfcParameterizedProfileDef& def, TempMesh& meshout, ConversionData& conv)
{
    if(const Schema_2x3::IfcRectangleProfileDef* const cprofile = def.ToPtr<Schema_2x3::IfcRectangleProfileDef>()) {
        const IfcFloat x = cprofile->XDim*0.5f, y = cprofile->YDim*0.5f;

        meshout.mVerts.reserve(meshout.mVerts.size()+4);
        meshout.mVerts.push_back( IfcVector3( x, y, 0.f ));
        meshout.mVerts.push_back( IfcVector3(-x, y, 0.f ));
        meshout.mVerts.push_back( IfcVector3(-x,-y, 0.f ));
        meshout.mVerts.push_back( IfcVector3( x,-y, 0.f ));
        meshout.mVertcnt.push_back(4);
    }
    else if( const Schema_2x3::IfcCircleProfileDef* const circle = def.ToPtr<Schema_2x3::IfcCircleProfileDef>()) {
        if(def.ToPtr<Schema_2x3::IfcCircleHollowProfileDef>()) {
            // TODO
        }
        const size_t segments = conv.settings.cylindricalTessellation;
        const IfcFloat delta = AI_MATH_TWO_PI_F/segments, radius = circle->Radius;

        meshout.mVerts.reserve(segments);

        IfcFloat angle = 0.f;
        for(size_t i = 0; i < segments; ++i, angle += delta) {
            meshout.mVerts.push_back( IfcVector3( std::cos(angle)*radius, std::sin(angle)*radius, 0.f ));
        }

        meshout.mVertcnt.push_back(static_cast<unsigned int>(segments));
    }
    else if( const Schema_2x3::IfcIShapeProfileDef* const ishape = def.ToPtr<Schema_2x3::IfcIShapeProfileDef>()) {
        // construct simplified IBeam shape
        const IfcFloat offset = (ishape->OverallWidth - ishape->WebThickness) / 2;
        const IfcFloat inner_height = ishape->OverallDepth - ishape->FlangeThickness * 2;

        meshout.mVerts.reserve(12);
        meshout.mVerts.push_back(IfcVector3(0,0,0));
        meshout.mVerts.push_back(IfcVector3(0,ishape->FlangeThickness,0));
        meshout.mVerts.push_back(IfcVector3(offset,ishape->FlangeThickness,0));
        meshout.mVerts.push_back(IfcVector3(offset,ishape->FlangeThickness + inner_height,0));
        meshout.mVerts.push_back(IfcVector3(0,ishape->FlangeThickness + inner_height,0));
        meshout.mVerts.push_back(IfcVector3(0,ishape->OverallDepth,0));
        meshout.mVerts.push_back(IfcVector3(ishape->OverallWidth,ishape->OverallDepth,0));
        meshout.mVerts.push_back(IfcVector3(ishape->OverallWidth,ishape->FlangeThickness + inner_height,0));
        meshout.mVerts.push_back(IfcVector3(offset+ishape->WebThickness,ishape->FlangeThickness + inner_height,0));
        meshout.mVerts.push_back(IfcVector3(offset+ishape->WebThickness,ishape->FlangeThickness,0));
        meshout.mVerts.push_back(IfcVector3(ishape->OverallWidth,ishape->FlangeThickness,0));
        meshout.mVerts.push_back(IfcVector3(ishape->OverallWidth,0,0));

        meshout.mVertcnt.push_back(12);
    }
    else {
        IFCImporter::LogWarn("skipping unknown IfcParameterizedProfileDef entity, type is " + def.GetClassName());
        return;
    }

    IfcMatrix4 trafo;
    ConvertAxisPlacement(trafo, *def.Position);
    meshout.Transform(trafo);
}

// ------------------------------------------------------------------------------------------------
bool ProcessProfile(const Schema_2x3::IfcProfileDef& prof, TempMesh& meshout, ConversionData& conv)
{
    if(const Schema_2x3::IfcArbitraryClosedProfileDef* const cprofile = prof.ToPtr<Schema_2x3::IfcArbitraryClosedProfileDef>()) {
        ProcessClosedProfile(*cprofile,meshout,conv);
    }
    else if(const Schema_2x3::IfcArbitraryOpenProfileDef* const copen = prof.ToPtr<Schema_2x3::IfcArbitraryOpenProfileDef>()) {
        ProcessOpenProfile(*copen,meshout,conv);
    }
    else if(const Schema_2x3::IfcParameterizedProfileDef* const cparam = prof.ToPtr<Schema_2x3::IfcParameterizedProfileDef>()) {
        ProcessParametrizedProfile(*cparam,meshout,conv);
    }
    else {
        IFCImporter::LogWarn("skipping unknown IfcProfileDef entity, type is " + prof.GetClassName());
        return false;
    }
    meshout.RemoveAdjacentDuplicates();
    if (!meshout.mVertcnt.size() || meshout.mVertcnt.front() <= 1) {
        return false;
    }
    return true;
}

} // ! IFC
} // ! Assimp

#endif // ASSIMP_BUILD_NO_IFC_IMPORTER
