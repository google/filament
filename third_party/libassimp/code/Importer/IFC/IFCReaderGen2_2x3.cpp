/*
Open Asset Import Library (ASSIMP)
----------------------------------------------------------------------

Copyright (c) 2006-2010, ASSIMP Development Team
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

* Neither the name of the ASSIMP team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the ASSIMP Development Team.

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

//#include "AssimpPCH.h"
#ifndef ASSIMP_BUILD_NO_IFC_IMPORTER

#include "IFCReaderGen_2x3.h"

namespace Assimp {
using namespace IFC;
using namespace ::Assimp::IFC::Schema_2x3;

namespace STEP {

template <> size_t GenericFill<IfcSurfaceStyle>(const DB& db, const LIST& params, IfcSurfaceStyle* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcPresentationStyle*>(in));
	if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to IfcSurfaceStyle"); }    do { // convert the 'Side' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->Side, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to IfcSurfaceStyle to be a `IfcSurfaceSide`")); }
    } while(0);
    do { // convert the 'Styles' argument
        std::shared_ptr<const DataType> arg = params[ base++ ];
        try { GenericConvert( in->Styles, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to IfcSurfaceStyle to be a `SET [1:5] OF IfcSurfaceStyleElementSelect`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcAnnotationSurface>(const DB& db, const LIST& params, IfcAnnotationSurface* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGeometricRepresentationItem*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFlowController>(const DB& db, const LIST& params, IfcFlowController* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcDistributionFlowElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcBuildingStorey>(const DB& db, const LIST& params, IfcBuildingStorey* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcSpatialStructureElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcWorkControl>(const DB& db, const LIST& params, IfcWorkControl* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcControl*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcWorkSchedule>(const DB& db, const LIST& params, IfcWorkSchedule* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcWorkControl*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcDuctSegmentType>(const DB& db, const LIST& params, IfcDuctSegmentType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowSegmentType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFace>(const DB& db, const LIST& params, IfcFace* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcTopologicalRepresentationItem*>(in));
	if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to IfcFace"); }    do { // convert the 'Bounds' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::IFC::Schema_2x3::IfcFace,1>::aux_is_derived[0]=true; break; }
        try { GenericConvert( in->Bounds, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to IfcFace to be a `SET [1:?] OF IfcFaceBound`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcStructuralSurfaceMember>(const DB& db, const LIST& params, IfcStructuralSurfaceMember* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcStructuralMember*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcStructuralSurfaceMemberVarying>(const DB& db, const LIST& params, IfcStructuralSurfaceMemberVarying* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcStructuralSurfaceMember*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFaceSurface>(const DB& db, const LIST& params, IfcFaceSurface* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFace*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcCostSchedule>(const DB& db, const LIST& params, IfcCostSchedule* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcControl*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcPlanarExtent>(const DB& db, const LIST& params, IfcPlanarExtent* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGeometricRepresentationItem*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcPlanarBox>(const DB& db, const LIST& params, IfcPlanarBox* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcPlanarExtent*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcColourSpecification>(const DB& db, const LIST& params, IfcColourSpecification* in)
{
	size_t base = 0;
	if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to IfcColourSpecification"); }    do { // convert the 'Name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::IFC::Schema_2x3::IfcColourSpecification,1>::aux_is_derived[0]=true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->Name, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to IfcColourSpecification to be a `IfcLabel`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcVector>(const DB& db, const LIST& params, IfcVector* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGeometricRepresentationItem*>(in));
	if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to IfcVector"); }    do { // convert the 'Orientation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->Orientation, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to IfcVector to be a `IfcDirection`")); }
    } while(0);
    do { // convert the 'Magnitude' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->Magnitude, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to IfcVector to be a `IfcLengthMeasure`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcBeam>(const DB& db, const LIST& params, IfcBeam* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBuildingElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcColourRgb>(const DB& db, const LIST& params, IfcColourRgb* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcColourSpecification*>(in));
	if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to IfcColourRgb"); }    do { // convert the 'Red' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->Red, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to IfcColourRgb to be a `IfcNormalisedRatioMeasure`")); }
    } while(0);
    do { // convert the 'Green' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->Green, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to IfcColourRgb to be a `IfcNormalisedRatioMeasure`")); }
    } while(0);
    do { // convert the 'Blue' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->Blue, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to IfcColourRgb to be a `IfcNormalisedRatioMeasure`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcStructuralPlanarAction>(const DB& db, const LIST& params, IfcStructuralPlanarAction* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcStructuralAction*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcStructuralPlanarActionVarying>(const DB& db, const LIST& params, IfcStructuralPlanarActionVarying* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcStructuralPlanarAction*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcSite>(const DB& db, const LIST& params, IfcSite* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcSpatialStructureElement*>(in));
	if (params.GetSize() < 14) { throw STEP::TypeError("expected 14 arguments to IfcSite"); }    do { // convert the 'RefLatitude' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->RefLatitude, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 9 to IfcSite to be a `IfcCompoundPlaneAngleMeasure`")); }
    } while(0);
    do { // convert the 'RefLongitude' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->RefLongitude, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 10 to IfcSite to be a `IfcCompoundPlaneAngleMeasure`")); }
    } while(0);
    do { // convert the 'RefElevation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->RefElevation, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 11 to IfcSite to be a `IfcLengthMeasure`")); }
    } while(0);
    do { // convert the 'LandTitleNumber' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->LandTitleNumber, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 12 to IfcSite to be a `IfcLabel`")); }
    } while(0);
    do { // convert the 'SiteAddress' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->SiteAddress, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 13 to IfcSite to be a `IfcPostalAddress`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcDiscreteAccessoryType>(const DB& db, const LIST& params, IfcDiscreteAccessoryType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcElementComponentType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcVibrationIsolatorType>(const DB& db, const LIST& params, IfcVibrationIsolatorType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcDiscreteAccessoryType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcEvaporativeCoolerType>(const DB& db, const LIST& params, IfcEvaporativeCoolerType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcEnergyConversionDeviceType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcDistributionChamberElementType>(const DB& db, const LIST& params, IfcDistributionChamberElementType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcDistributionFlowElementType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFeatureElementAddition>(const DB& db, const LIST& params, IfcFeatureElementAddition* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFeatureElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcStructuredDimensionCallout>(const DB& db, const LIST& params, IfcStructuredDimensionCallout* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcDraughtingCallout*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcCoolingTowerType>(const DB& db, const LIST& params, IfcCoolingTowerType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcEnergyConversionDeviceType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcCenterLineProfileDef>(const DB& db, const LIST& params, IfcCenterLineProfileDef* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcArbitraryOpenProfileDef*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcWindowStyle>(const DB& db, const LIST& params, IfcWindowStyle* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcTypeProduct*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcLightSourceGoniometric>(const DB& db, const LIST& params, IfcLightSourceGoniometric* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcLightSource*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcTransformerType>(const DB& db, const LIST& params, IfcTransformerType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcEnergyConversionDeviceType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcMemberType>(const DB& db, const LIST& params, IfcMemberType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBuildingElementType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcSurfaceOfLinearExtrusion>(const DB& db, const LIST& params, IfcSurfaceOfLinearExtrusion* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcSweptSurface*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcMotorConnectionType>(const DB& db, const LIST& params, IfcMotorConnectionType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcEnergyConversionDeviceType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFlowTreatmentDeviceType>(const DB& db, const LIST& params, IfcFlowTreatmentDeviceType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcDistributionFlowElementType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcDuctSilencerType>(const DB& db, const LIST& params, IfcDuctSilencerType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowTreatmentDeviceType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFurnishingElementType>(const DB& db, const LIST& params, IfcFurnishingElementType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcElementType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcSystemFurnitureElementType>(const DB& db, const LIST& params, IfcSystemFurnitureElementType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFurnishingElementType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcWasteTerminalType>(const DB& db, const LIST& params, IfcWasteTerminalType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowTerminalType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcBSplineCurve>(const DB& db, const LIST& params, IfcBSplineCurve* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBoundedCurve*>(in));
	if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to IfcBSplineCurve"); }    do { // convert the 'Degree' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::IFC::Schema_2x3::IfcBSplineCurve,5>::aux_is_derived[0]=true; break; }
        try { GenericConvert( in->Degree, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to IfcBSplineCurve to be a `INTEGER`")); }
    } while(0);
    do { // convert the 'ControlPointsList' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::IFC::Schema_2x3::IfcBSplineCurve,5>::aux_is_derived[1]=true; break; }
        try { GenericConvert( in->ControlPointsList, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to IfcBSplineCurve to be a `LIST [2:?] OF IfcCartesianPoint`")); }
    } while(0);
    do { // convert the 'CurveForm' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::IFC::Schema_2x3::IfcBSplineCurve,5>::aux_is_derived[2]=true; break; }
        try { GenericConvert( in->CurveForm, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to IfcBSplineCurve to be a `IfcBSplineCurveForm`")); }
    } while(0);
    do { // convert the 'ClosedCurve' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::IFC::Schema_2x3::IfcBSplineCurve,5>::aux_is_derived[3]=true; break; }
        try { GenericConvert( in->ClosedCurve, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to IfcBSplineCurve to be a `LOGICAL`")); }
    } while(0);
    do { // convert the 'SelfIntersect' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::IFC::Schema_2x3::IfcBSplineCurve,5>::aux_is_derived[4]=true; break; }
        try { GenericConvert( in->SelfIntersect, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to IfcBSplineCurve to be a `LOGICAL`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcBezierCurve>(const DB& db, const LIST& params, IfcBezierCurve* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBSplineCurve*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcActuatorType>(const DB& db, const LIST& params, IfcActuatorType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcDistributionControlElementType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcDistributionControlElement>(const DB& db, const LIST& params, IfcDistributionControlElement* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcDistributionElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcAnnotation>(const DB& db, const LIST& params, IfcAnnotation* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcProduct*>(in));
	if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to IfcAnnotation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcShellBasedSurfaceModel>(const DB& db, const LIST& params, IfcShellBasedSurfaceModel* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGeometricRepresentationItem*>(in));
	if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to IfcShellBasedSurfaceModel"); }    do { // convert the 'SbsmBoundary' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->SbsmBoundary, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to IfcShellBasedSurfaceModel to be a `SET [1:?] OF IfcShell`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcActionRequest>(const DB& db, const LIST& params, IfcActionRequest* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcControl*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcExtrudedAreaSolid>(const DB& db, const LIST& params, IfcExtrudedAreaSolid* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcSweptAreaSolid*>(in));
	if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to IfcExtrudedAreaSolid"); }    do { // convert the 'ExtrudedDirection' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->ExtrudedDirection, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to IfcExtrudedAreaSolid to be a `IfcDirection`")); }
    } while(0);
    do { // convert the 'Depth' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->Depth, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to IfcExtrudedAreaSolid to be a `IfcPositiveLengthMeasure`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcSystem>(const DB& db, const LIST& params, IfcSystem* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGroup*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFillAreaStyleHatching>(const DB& db, const LIST& params, IfcFillAreaStyleHatching* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGeometricRepresentationItem*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcRelVoidsElement>(const DB& db, const LIST& params, IfcRelVoidsElement* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcRelConnects*>(in));
	if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to IfcRelVoidsElement"); }    do { // convert the 'RelatingBuildingElement' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->RelatingBuildingElement, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to IfcRelVoidsElement to be a `IfcElement`")); }
    } while(0);
    do { // convert the 'RelatedOpeningElement' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->RelatedOpeningElement, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to IfcRelVoidsElement to be a `IfcFeatureElementSubtraction`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcSurfaceCurveSweptAreaSolid>(const DB& db, const LIST& params, IfcSurfaceCurveSweptAreaSolid* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcSweptAreaSolid*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcCartesianTransformationOperator3DnonUniform>(const DB& db, const LIST& params, IfcCartesianTransformationOperator3DnonUniform* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcCartesianTransformationOperator3D*>(in));
	if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to IfcCartesianTransformationOperator3DnonUniform"); }    do { // convert the 'Scale2' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->Scale2, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to IfcCartesianTransformationOperator3DnonUniform to be a `REAL`")); }
    } while(0);
    do { // convert the 'Scale3' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->Scale3, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to IfcCartesianTransformationOperator3DnonUniform to be a `REAL`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcCurtainWallType>(const DB& db, const LIST& params, IfcCurtainWallType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBuildingElementType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcEquipmentStandard>(const DB& db, const LIST& params, IfcEquipmentStandard* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcControl*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFlowStorageDeviceType>(const DB& db, const LIST& params, IfcFlowStorageDeviceType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcDistributionFlowElementType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcDiameterDimension>(const DB& db, const LIST& params, IfcDiameterDimension* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcDimensionCurveDirectedCallout*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcSwitchingDeviceType>(const DB& db, const LIST& params, IfcSwitchingDeviceType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowControllerType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcWindow>(const DB& db, const LIST& params, IfcWindow* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBuildingElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFlowTreatmentDevice>(const DB& db, const LIST& params, IfcFlowTreatmentDevice* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcDistributionFlowElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcChillerType>(const DB& db, const LIST& params, IfcChillerType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcEnergyConversionDeviceType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcRectangleHollowProfileDef>(const DB& db, const LIST& params, IfcRectangleHollowProfileDef* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcRectangleProfileDef*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcBoxedHalfSpace>(const DB& db, const LIST& params, IfcBoxedHalfSpace* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcHalfSpaceSolid*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcAxis2Placement2D>(const DB& db, const LIST& params, IfcAxis2Placement2D* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcPlacement*>(in));
	if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to IfcAxis2Placement2D"); }    do { // convert the 'RefDirection' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->RefDirection, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to IfcAxis2Placement2D to be a `IfcDirection`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcSpaceProgram>(const DB& db, const LIST& params, IfcSpaceProgram* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcControl*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcPoint>(const DB& db, const LIST& params, IfcPoint* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGeometricRepresentationItem*>(in));
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcCartesianPoint>(const DB& db, const LIST& params, IfcCartesianPoint* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcPoint*>(in));
	if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to IfcCartesianPoint"); }    do { // convert the 'Coordinates' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->Coordinates, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to IfcCartesianPoint to be a `LIST [1:3] OF IfcLengthMeasure`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcBoundedSurface>(const DB& db, const LIST& params, IfcBoundedSurface* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcSurface*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcLoop>(const DB& db, const LIST& params, IfcLoop* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcTopologicalRepresentationItem*>(in));
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcPolyLoop>(const DB& db, const LIST& params, IfcPolyLoop* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcLoop*>(in));
	if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to IfcPolyLoop"); }    do { // convert the 'Polygon' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->Polygon, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to IfcPolyLoop to be a `LIST [3:?] OF IfcCartesianPoint`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcTerminatorSymbol>(const DB& db, const LIST& params, IfcTerminatorSymbol* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcAnnotationSymbolOccurrence*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcDimensionCurveTerminator>(const DB& db, const LIST& params, IfcDimensionCurveTerminator* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcTerminatorSymbol*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcTrapeziumProfileDef>(const DB& db, const LIST& params, IfcTrapeziumProfileDef* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcParameterizedProfileDef*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcRepresentationContext>(const DB& db, const LIST& params, IfcRepresentationContext* in)
{
	size_t base = 0;
	if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to IfcRepresentationContext"); }    do { // convert the 'ContextIdentifier' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::IFC::Schema_2x3::IfcRepresentationContext,2>::aux_is_derived[0]=true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->ContextIdentifier, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to IfcRepresentationContext to be a `IfcLabel`")); }
    } while(0);
    do { // convert the 'ContextType' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::IFC::Schema_2x3::IfcRepresentationContext,2>::aux_is_derived[1]=true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->ContextType, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to IfcRepresentationContext to be a `IfcLabel`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcGeometricRepresentationContext>(const DB& db, const LIST& params, IfcGeometricRepresentationContext* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcRepresentationContext*>(in));
	if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to IfcGeometricRepresentationContext"); }    do { // convert the 'CoordinateSpaceDimension' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::IFC::Schema_2x3::IfcGeometricRepresentationContext,4>::aux_is_derived[0]=true; break; }
        try { GenericConvert( in->CoordinateSpaceDimension, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to IfcGeometricRepresentationContext to be a `IfcDimensionCount`")); }
    } while(0);
    do { // convert the 'Precision' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::IFC::Schema_2x3::IfcGeometricRepresentationContext,4>::aux_is_derived[1]=true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->Precision, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to IfcGeometricRepresentationContext to be a `REAL`")); }
    } while(0);
    do { // convert the 'WorldCoordinateSystem' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::IFC::Schema_2x3::IfcGeometricRepresentationContext,4>::aux_is_derived[2]=true; break; }
        try { GenericConvert( in->WorldCoordinateSystem, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to IfcGeometricRepresentationContext to be a `IfcAxis2Placement`")); }
    } while(0);
    do { // convert the 'TrueNorth' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::IFC::Schema_2x3::IfcGeometricRepresentationContext,4>::aux_is_derived[3]=true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->TrueNorth, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to IfcGeometricRepresentationContext to be a `IfcDirection`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcCurveBoundedPlane>(const DB& db, const LIST& params, IfcCurveBoundedPlane* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBoundedSurface*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcSIUnit>(const DB& db, const LIST& params, IfcSIUnit* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcNamedUnit*>(in));
	if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to IfcSIUnit"); }    do { // convert the 'Prefix' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->Prefix, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to IfcSIUnit to be a `IfcSIPrefix`")); }
    } while(0);
    do { // convert the 'Name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->Name, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to IfcSIUnit to be a `IfcSIUnitName`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcStructuralReaction>(const DB& db, const LIST& params, IfcStructuralReaction* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcStructuralActivity*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcStructuralPointReaction>(const DB& db, const LIST& params, IfcStructuralPointReaction* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcStructuralReaction*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcAxis1Placement>(const DB& db, const LIST& params, IfcAxis1Placement* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcPlacement*>(in));
	if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to IfcAxis1Placement"); }    do { // convert the 'Axis' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->Axis, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to IfcAxis1Placement to be a `IfcDirection`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcElectricApplianceType>(const DB& db, const LIST& params, IfcElectricApplianceType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowTerminalType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcSensorType>(const DB& db, const LIST& params, IfcSensorType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcDistributionControlElementType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFurnishingElement>(const DB& db, const LIST& params, IfcFurnishingElement* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcProtectiveDeviceType>(const DB& db, const LIST& params, IfcProtectiveDeviceType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowControllerType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcZShapeProfileDef>(const DB& db, const LIST& params, IfcZShapeProfileDef* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcParameterizedProfileDef*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcScheduleTimeControl>(const DB& db, const LIST& params, IfcScheduleTimeControl* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcControl*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcRepresentationMap>(const DB& db, const LIST& params, IfcRepresentationMap* in)
{
	size_t base = 0;
	if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to IfcRepresentationMap"); }    do { // convert the 'MappingOrigin' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->MappingOrigin, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to IfcRepresentationMap to be a `IfcAxis2Placement`")); }
    } while(0);
    do { // convert the 'MappedRepresentation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->MappedRepresentation, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to IfcRepresentationMap to be a `IfcRepresentation`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcClosedShell>(const DB& db, const LIST& params, IfcClosedShell* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcConnectedFaceSet*>(in));
	if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to IfcClosedShell"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcBuildingElementPart>(const DB& db, const LIST& params, IfcBuildingElementPart* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBuildingElementComponent*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcBlock>(const DB& db, const LIST& params, IfcBlock* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcCsgPrimitive3D*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcLightFixtureType>(const DB& db, const LIST& params, IfcLightFixtureType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowTerminalType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcOpeningElement>(const DB& db, const LIST& params, IfcOpeningElement* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFeatureElementSubtraction*>(in));
	if (params.GetSize() < 8) { throw STEP::TypeError("expected 8 arguments to IfcOpeningElement"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcLightSourceSpot>(const DB& db, const LIST& params, IfcLightSourceSpot* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcLightSourcePositional*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcTendonAnchor>(const DB& db, const LIST& params, IfcTendonAnchor* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcReinforcingElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcElectricFlowStorageDeviceType>(const DB& db, const LIST& params, IfcElectricFlowStorageDeviceType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowStorageDeviceType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcSphere>(const DB& db, const LIST& params, IfcSphere* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcCsgPrimitive3D*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcDamperType>(const DB& db, const LIST& params, IfcDamperType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowControllerType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcProjectOrderRecord>(const DB& db, const LIST& params, IfcProjectOrderRecord* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcControl*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcDistributionChamberElement>(const DB& db, const LIST& params, IfcDistributionChamberElement* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcDistributionFlowElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcMechanicalFastener>(const DB& db, const LIST& params, IfcMechanicalFastener* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFastener*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcRectangularTrimmedSurface>(const DB& db, const LIST& params, IfcRectangularTrimmedSurface* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBoundedSurface*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcZone>(const DB& db, const LIST& params, IfcZone* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGroup*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFanType>(const DB& db, const LIST& params, IfcFanType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowMovingDeviceType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcGeometricSet>(const DB& db, const LIST& params, IfcGeometricSet* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGeometricRepresentationItem*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFillAreaStyleTiles>(const DB& db, const LIST& params, IfcFillAreaStyleTiles* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGeometricRepresentationItem*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcCableSegmentType>(const DB& db, const LIST& params, IfcCableSegmentType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowSegmentType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcRelOverridesProperties>(const DB& db, const LIST& params, IfcRelOverridesProperties* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcRelDefinesByProperties*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcMeasureWithUnit>(const DB& db, const LIST& params, IfcMeasureWithUnit* in)
{
	size_t base = 0;
	if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to IfcMeasureWithUnit"); }    do { // convert the 'ValueComponent' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->ValueComponent, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to IfcMeasureWithUnit to be a `IfcValue`")); }
    } while(0);
    do { // convert the 'UnitComponent' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->UnitComponent, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to IfcMeasureWithUnit to be a `IfcUnit`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcSlabType>(const DB& db, const LIST& params, IfcSlabType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBuildingElementType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcServiceLife>(const DB& db, const LIST& params, IfcServiceLife* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcControl*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFurnitureType>(const DB& db, const LIST& params, IfcFurnitureType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFurnishingElementType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcCostItem>(const DB& db, const LIST& params, IfcCostItem* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcControl*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcReinforcingMesh>(const DB& db, const LIST& params, IfcReinforcingMesh* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcReinforcingElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFacetedBrepWithVoids>(const DB& db, const LIST& params, IfcFacetedBrepWithVoids* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcManifoldSolidBrep*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcGasTerminalType>(const DB& db, const LIST& params, IfcGasTerminalType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowTerminalType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcPile>(const DB& db, const LIST& params, IfcPile* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBuildingElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFillAreaStyleTileSymbolWithStyle>(const DB& db, const LIST& params, IfcFillAreaStyleTileSymbolWithStyle* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGeometricRepresentationItem*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcConstructionMaterialResource>(const DB& db, const LIST& params, IfcConstructionMaterialResource* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcConstructionResource*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcAnnotationCurveOccurrence>(const DB& db, const LIST& params, IfcAnnotationCurveOccurrence* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcAnnotationOccurrence*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcDimensionCurve>(const DB& db, const LIST& params, IfcDimensionCurve* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcAnnotationCurveOccurrence*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcGeometricCurveSet>(const DB& db, const LIST& params, IfcGeometricCurveSet* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGeometricSet*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcRelAggregates>(const DB& db, const LIST& params, IfcRelAggregates* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcRelDecomposes*>(in));
	if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to IfcRelAggregates"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFaceBasedSurfaceModel>(const DB& db, const LIST& params, IfcFaceBasedSurfaceModel* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGeometricRepresentationItem*>(in));
	if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to IfcFaceBasedSurfaceModel"); }    do { // convert the 'FbsmFaces' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->FbsmFaces, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to IfcFaceBasedSurfaceModel to be a `SET [1:?] OF IfcConnectedFaceSet`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcEnergyConversionDevice>(const DB& db, const LIST& params, IfcEnergyConversionDevice* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcDistributionFlowElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcRampFlight>(const DB& db, const LIST& params, IfcRampFlight* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBuildingElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcVertexLoop>(const DB& db, const LIST& params, IfcVertexLoop* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcLoop*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcPlate>(const DB& db, const LIST& params, IfcPlate* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBuildingElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcUShapeProfileDef>(const DB& db, const LIST& params, IfcUShapeProfileDef* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcParameterizedProfileDef*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFaceBound>(const DB& db, const LIST& params, IfcFaceBound* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcTopologicalRepresentationItem*>(in));
	if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to IfcFaceBound"); }    do { // convert the 'Bound' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::IFC::Schema_2x3::IfcFaceBound,2>::aux_is_derived[0]=true; break; }
        try { GenericConvert( in->Bound, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to IfcFaceBound to be a `IfcLoop`")); }
    } while(0);
    do { // convert the 'Orientation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::IFC::Schema_2x3::IfcFaceBound,2>::aux_is_derived[1]=true; break; }
        try { GenericConvert( in->Orientation, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to IfcFaceBound to be a `BOOLEAN`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFaceOuterBound>(const DB& db, const LIST& params, IfcFaceOuterBound* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFaceBound*>(in));
	if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to IfcFaceOuterBound"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcOneDirectionRepeatFactor>(const DB& db, const LIST& params, IfcOneDirectionRepeatFactor* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGeometricRepresentationItem*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcBoilerType>(const DB& db, const LIST& params, IfcBoilerType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcEnergyConversionDeviceType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcConstructionEquipmentResource>(const DB& db, const LIST& params, IfcConstructionEquipmentResource* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcConstructionResource*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcComplexProperty>(const DB& db, const LIST& params, IfcComplexProperty* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcProperty*>(in));
	if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to IfcComplexProperty"); }    do { // convert the 'UsageName' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->UsageName, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to IfcComplexProperty to be a `IfcIdentifier`")); }
    } while(0);
    do { // convert the 'HasProperties' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->HasProperties, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to IfcComplexProperty to be a `SET [1:?] OF IfcProperty`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFooting>(const DB& db, const LIST& params, IfcFooting* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBuildingElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcConstructionProductResource>(const DB& db, const LIST& params, IfcConstructionProductResource* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcConstructionResource*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcDerivedProfileDef>(const DB& db, const LIST& params, IfcDerivedProfileDef* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcProfileDef*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcPropertyTableValue>(const DB& db, const LIST& params, IfcPropertyTableValue* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcSimpleProperty*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFlowMeterType>(const DB& db, const LIST& params, IfcFlowMeterType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowControllerType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcDoorStyle>(const DB& db, const LIST& params, IfcDoorStyle* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcTypeProduct*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcUnitAssignment>(const DB& db, const LIST& params, IfcUnitAssignment* in)
{
	size_t base = 0;
	if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to IfcUnitAssignment"); }    do { // convert the 'Units' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->Units, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to IfcUnitAssignment to be a `SET [1:?] OF IfcUnit`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFlowTerminal>(const DB& db, const LIST& params, IfcFlowTerminal* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcDistributionFlowElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcCraneRailFShapeProfileDef>(const DB& db, const LIST& params, IfcCraneRailFShapeProfileDef* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcParameterizedProfileDef*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFlowSegment>(const DB& db, const LIST& params, IfcFlowSegment* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcDistributionFlowElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcElementQuantity>(const DB& db, const LIST& params, IfcElementQuantity* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcPropertySetDefinition*>(in));
	if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to IfcElementQuantity"); }    do { // convert the 'MethodOfMeasurement' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->MethodOfMeasurement, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to IfcElementQuantity to be a `IfcLabel`")); }
    } while(0);
    do { // convert the 'Quantities' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->Quantities, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to IfcElementQuantity to be a `SET [1:?] OF IfcPhysicalQuantity`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcCurtainWall>(const DB& db, const LIST& params, IfcCurtainWall* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBuildingElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcDiscreteAccessory>(const DB& db, const LIST& params, IfcDiscreteAccessory* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcElementComponent*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcGrid>(const DB& db, const LIST& params, IfcGrid* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcProduct*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcSanitaryTerminalType>(const DB& db, const LIST& params, IfcSanitaryTerminalType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowTerminalType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcSubedge>(const DB& db, const LIST& params, IfcSubedge* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcEdge*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcFilterType>(const DB& db, const LIST& params, IfcFilterType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowTreatmentDeviceType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcTendon>(const DB& db, const LIST& params, IfcTendon* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcReinforcingElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcStructuralLoadGroup>(const DB& db, const LIST& params, IfcStructuralLoadGroup* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGroup*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcPresentationStyleAssignment>(const DB& db, const LIST& params, IfcPresentationStyleAssignment* in)
{
	size_t base = 0;
	if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to IfcPresentationStyleAssignment"); }    do { // convert the 'Styles' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->Styles, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to IfcPresentationStyleAssignment to be a `SET [1:?] OF IfcPresentationStyleSelect`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcStructuralCurveMember>(const DB& db, const LIST& params, IfcStructuralCurveMember* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcStructuralMember*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcLightSourceAmbient>(const DB& db, const LIST& params, IfcLightSourceAmbient* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcLightSource*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcCondition>(const DB& db, const LIST& params, IfcCondition* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGroup*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcPort>(const DB& db, const LIST& params, IfcPort* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcProduct*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcSpace>(const DB& db, const LIST& params, IfcSpace* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcSpatialStructureElement*>(in));
	if (params.GetSize() < 11) { throw STEP::TypeError("expected 11 arguments to IfcSpace"); }    do { // convert the 'InteriorOrExteriorSpace' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->InteriorOrExteriorSpace, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 9 to IfcSpace to be a `IfcInternalOrExternalEnum`")); }
    } while(0);
    do { // convert the 'ElevationWithFlooring' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->ElevationWithFlooring, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 10 to IfcSpace to be a `IfcLengthMeasure`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcHeatExchangerType>(const DB& db, const LIST& params, IfcHeatExchangerType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcEnergyConversionDeviceType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcTankType>(const DB& db, const LIST& params, IfcTankType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowStorageDeviceType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcInventory>(const DB& db, const LIST& params, IfcInventory* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGroup*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcTransportElementType>(const DB& db, const LIST& params, IfcTransportElementType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcElementType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcAirToAirHeatRecoveryType>(const DB& db, const LIST& params, IfcAirToAirHeatRecoveryType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcEnergyConversionDeviceType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcStairFlight>(const DB& db, const LIST& params, IfcStairFlight* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBuildingElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcElectricalElement>(const DB& db, const LIST& params, IfcElectricalElement* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcSurfaceStyleWithTextures>(const DB& db, const LIST& params, IfcSurfaceStyleWithTextures* in)
{
	size_t base = 0;
	if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to IfcSurfaceStyleWithTextures"); }    do { // convert the 'Textures' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->Textures, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to IfcSurfaceStyleWithTextures to be a `LIST [1:?] OF IfcSurfaceTexture`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcBoundingBox>(const DB& db, const LIST& params, IfcBoundingBox* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGeometricRepresentationItem*>(in));
	if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to IfcBoundingBox"); }    do { // convert the 'Corner' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->Corner, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to IfcBoundingBox to be a `IfcCartesianPoint`")); }
    } while(0);
    do { // convert the 'XDim' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->XDim, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to IfcBoundingBox to be a `IfcPositiveLengthMeasure`")); }
    } while(0);
    do { // convert the 'YDim' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->YDim, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to IfcBoundingBox to be a `IfcPositiveLengthMeasure`")); }
    } while(0);
    do { // convert the 'ZDim' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->ZDim, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to IfcBoundingBox to be a `IfcPositiveLengthMeasure`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcWallType>(const DB& db, const LIST& params, IfcWallType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBuildingElementType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcMove>(const DB& db, const LIST& params, IfcMove* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcTask*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcCircle>(const DB& db, const LIST& params, IfcCircle* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcConic*>(in));
	if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to IfcCircle"); }    do { // convert the 'Radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->Radius, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to IfcCircle to be a `IfcPositiveLengthMeasure`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcOffsetCurve2D>(const DB& db, const LIST& params, IfcOffsetCurve2D* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcCurve*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcPointOnCurve>(const DB& db, const LIST& params, IfcPointOnCurve* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcPoint*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcStructuralResultGroup>(const DB& db, const LIST& params, IfcStructuralResultGroup* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGroup*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcSectionedSpine>(const DB& db, const LIST& params, IfcSectionedSpine* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGeometricRepresentationItem*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcSlab>(const DB& db, const LIST& params, IfcSlab* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBuildingElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcVertex>(const DB& db, const LIST& params, IfcVertex* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcTopologicalRepresentationItem*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcVertexPoint>(const DB& db, const LIST& params, IfcVertexPoint* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcVertex*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcStructuralLinearAction>(const DB& db, const LIST& params, IfcStructuralLinearAction* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcStructuralAction*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcStructuralLinearActionVarying>(const DB& db, const LIST& params, IfcStructuralLinearActionVarying* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcStructuralLinearAction*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcBuildingElementProxyType>(const DB& db, const LIST& params, IfcBuildingElementProxyType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBuildingElementType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcProjectionElement>(const DB& db, const LIST& params, IfcProjectionElement* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFeatureElementAddition*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcConversionBasedUnit>(const DB& db, const LIST& params, IfcConversionBasedUnit* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcNamedUnit*>(in));
	if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to IfcConversionBasedUnit"); }    do { // convert the 'Name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->Name, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to IfcConversionBasedUnit to be a `IfcLabel`")); }
    } while(0);
    do { // convert the 'ConversionFactor' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->ConversionFactor, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to IfcConversionBasedUnit to be a `IfcMeasureWithUnit`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcGeometricRepresentationSubContext>(const DB& db, const LIST& params, IfcGeometricRepresentationSubContext* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGeometricRepresentationContext*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcAnnotationSurfaceOccurrence>(const DB& db, const LIST& params, IfcAnnotationSurfaceOccurrence* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcAnnotationOccurrence*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcRoundedEdgeFeature>(const DB& db, const LIST& params, IfcRoundedEdgeFeature* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcEdgeFeature*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcElectricDistributionPoint>(const DB& db, const LIST& params, IfcElectricDistributionPoint* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowController*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcCableCarrierSegmentType>(const DB& db, const LIST& params, IfcCableCarrierSegmentType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowSegmentType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcWallStandardCase>(const DB& db, const LIST& params, IfcWallStandardCase* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcWall*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcCsgSolid>(const DB& db, const LIST& params, IfcCsgSolid* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcSolidModel*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcBeamType>(const DB& db, const LIST& params, IfcBeamType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBuildingElementType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcAnnotationFillArea>(const DB& db, const LIST& params, IfcAnnotationFillArea* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcGeometricRepresentationItem*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcStructuralCurveMemberVarying>(const DB& db, const LIST& params, IfcStructuralCurveMemberVarying* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcStructuralCurveMember*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcPointOnSurface>(const DB& db, const LIST& params, IfcPointOnSurface* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcPoint*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcOrderAction>(const DB& db, const LIST& params, IfcOrderAction* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcTask*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcEdgeLoop>(const DB& db, const LIST& params, IfcEdgeLoop* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcLoop*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcAnnotationFillAreaOccurrence>(const DB& db, const LIST& params, IfcAnnotationFillAreaOccurrence* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcAnnotationOccurrence*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcWorkPlan>(const DB& db, const LIST& params, IfcWorkPlan* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcWorkControl*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcEllipse>(const DB& db, const LIST& params, IfcEllipse* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcConic*>(in));
	if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to IfcEllipse"); }    do { // convert the 'SemiAxis1' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->SemiAxis1, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to IfcEllipse to be a `IfcPositiveLengthMeasure`")); }
    } while(0);
    do { // convert the 'SemiAxis2' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->SemiAxis2, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to IfcEllipse to be a `IfcPositiveLengthMeasure`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcProductDefinitionShape>(const DB& db, const LIST& params, IfcProductDefinitionShape* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcProductRepresentation*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcProjectionCurve>(const DB& db, const LIST& params, IfcProjectionCurve* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcAnnotationCurveOccurrence*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcElectricalCircuit>(const DB& db, const LIST& params, IfcElectricalCircuit* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcSystem*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcRationalBezierCurve>(const DB& db, const LIST& params, IfcRationalBezierCurve* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcBezierCurve*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcStructuralPointAction>(const DB& db, const LIST& params, IfcStructuralPointAction* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcStructuralAction*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcPipeSegmentType>(const DB& db, const LIST& params, IfcPipeSegmentType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowSegmentType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcTwoDirectionRepeatFactor>(const DB& db, const LIST& params, IfcTwoDirectionRepeatFactor* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcOneDirectionRepeatFactor*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcShapeRepresentation>(const DB& db, const LIST& params, IfcShapeRepresentation* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcShapeModel*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcPropertySet>(const DB& db, const LIST& params, IfcPropertySet* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcPropertySetDefinition*>(in));
	if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to IfcPropertySet"); }    do { // convert the 'HasProperties' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->HasProperties, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to IfcPropertySet to be a `SET [1:?] OF IfcProperty`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcSurfaceStyleRendering>(const DB& db, const LIST& params, IfcSurfaceStyleRendering* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcSurfaceStyleShading*>(in));
	if (params.GetSize() < 9) { throw STEP::TypeError("expected 9 arguments to IfcSurfaceStyleRendering"); }    do { // convert the 'Transparency' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->Transparency, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to IfcSurfaceStyleRendering to be a `IfcNormalisedRatioMeasure`")); }
    } while(0);
    do { // convert the 'DiffuseColour' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->DiffuseColour, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to IfcSurfaceStyleRendering to be a `IfcColourOrFactor`")); }
    } while(0);
    do { // convert the 'TransmissionColour' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->TransmissionColour, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to IfcSurfaceStyleRendering to be a `IfcColourOrFactor`")); }
    } while(0);
    do { // convert the 'DiffuseTransmissionColour' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->DiffuseTransmissionColour, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to IfcSurfaceStyleRendering to be a `IfcColourOrFactor`")); }
    } while(0);
    do { // convert the 'ReflectionColour' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->ReflectionColour, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to IfcSurfaceStyleRendering to be a `IfcColourOrFactor`")); }
    } while(0);
    do { // convert the 'SpecularColour' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->SpecularColour, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to IfcSurfaceStyleRendering to be a `IfcColourOrFactor`")); }
    } while(0);
    do { // convert the 'SpecularHighlight' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert( in->SpecularHighlight, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to IfcSurfaceStyleRendering to be a `IfcSpecularHighlightSelect`")); }
    } while(0);
    do { // convert the 'ReflectanceMethod' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert( in->ReflectanceMethod, arg, db ); break; } 
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 8 to IfcSurfaceStyleRendering to be a `IfcReflectanceMethodEnum`")); }
    } while(0);
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcDistributionPort>(const DB& db, const LIST& params, IfcDistributionPort* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcPort*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcPipeFittingType>(const DB& db, const LIST& params, IfcPipeFittingType* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcFlowFittingType*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcTransportElement>(const DB& db, const LIST& params, IfcTransportElement* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcElement*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcAnnotationTextOccurrence>(const DB& db, const LIST& params, IfcAnnotationTextOccurrence* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcAnnotationOccurrence*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcStructuralAnalysisModel>(const DB& db, const LIST& params, IfcStructuralAnalysisModel* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcSystem*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<IfcConditionCriterion>(const DB& db, const LIST& params, IfcConditionCriterion* in)
{
	size_t base = GenericFill(db,params,static_cast<IfcControl*>(in));
// this data structure is not used yet, so there is no code generated to fill its members
	return base;
}

} // ! STEP
} // ! Assimp

#endif
