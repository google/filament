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

#include "code/Importer/StepFile/StepReaderGen.h"

namespace Assimp {
using namespace StepFile;
namespace STEP {

// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<dimension_callout_component_relationship>(const DB& db, const LIST& params, dimension_callout_component_relationship* in)
{
    size_t base = GenericFill(db, params, static_cast<draughting_callout_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to dimension_callout_component_relationship"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<dimension_callout_relationship>(const DB& db, const LIST& params, dimension_callout_relationship* in)
{
    size_t base = GenericFill(db, params, static_cast<draughting_callout_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to dimension_callout_relationship"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<dimension_curve>(const DB& db, const LIST& params, dimension_curve* in)
{
    size_t base = GenericFill(db, params, static_cast<annotation_curve_occurrence*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to dimension_curve"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<terminator_symbol>(const DB& db, const LIST& params, terminator_symbol* in)
{
    size_t base = GenericFill(db, params, static_cast<annotation_symbol_occurrence*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to terminator_symbol"); }    do { // convert the 'annotated_curve' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::terminator_symbol, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->annotated_curve, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to terminator_symbol to be a `annotation_curve_occurrence`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<dimension_curve_terminator>(const DB& db, const LIST& params, dimension_curve_terminator* in)
{
    size_t base = GenericFill(db, params, static_cast<terminator_symbol*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to dimension_curve_terminator"); }    do { // convert the 'role' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->role, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to dimension_curve_terminator to be a `dimension_extent_usage`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<dimension_curve_terminator_to_projection_curve_associativity>(const DB& db, const LIST& params, dimension_curve_terminator_to_projection_curve_associativity* in)
{
    size_t base = GenericFill(db, params, static_cast<annotation_occurrence_associativity*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to dimension_curve_terminator_to_projection_curve_associativity"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<dimension_pair>(const DB& db, const LIST& params, dimension_pair* in)
{
    size_t base = GenericFill(db, params, static_cast<draughting_callout_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to dimension_pair"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<dimension_text_associativity>(const DB& db, const LIST& params, dimension_text_associativity* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<dimensional_location_with_path>(const DB& db, const LIST& params, dimensional_location_with_path* in)
{
    size_t base = GenericFill(db, params, static_cast<dimensional_location*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to dimensional_location_with_path"); }    do { // convert the 'path' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->path, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to dimensional_location_with_path to be a `shape_aspect`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<dimensional_size_with_path>(const DB& db, const LIST& params, dimensional_size_with_path* in)
{
    size_t base = GenericFill(db, params, static_cast<dimensional_size*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to dimensional_size_with_path"); }    do { // convert the 'path' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->path, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to dimensional_size_with_path to be a `shape_aspect`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<executed_action>(const DB& db, const LIST& params, executed_action* in)
{
    size_t base = GenericFill(db, params, static_cast<action*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to executed_action"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<directed_action>(const DB& db, const LIST& params, directed_action* in)
{
    size_t base = GenericFill(db, params, static_cast<executed_action*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to directed_action"); }    do { // convert the 'directive' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->directive, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to directed_action to be a `action_directive`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<directed_dimensional_location>(const DB& db, const LIST& params, directed_dimensional_location* in)
{
    size_t base = GenericFill(db, params, static_cast<dimensional_location*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to directed_dimensional_location"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<direction>(const DB& db, const LIST& params, direction* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to direction"); }    do { // convert the 'direction_ratios' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->direction_ratios, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to direction to be a `LIST [2:3] OF REAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<document_file>(const DB& db, const LIST& params, document_file* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<document_identifier>(const DB& db, const LIST& params, document_identifier* in)
{
    size_t base = GenericFill(db, params, static_cast<group*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to document_identifier"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<document_identifier_assignment>(const DB& db, const LIST& params, document_identifier_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<group_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to document_identifier_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to document_identifier_assignment to be a `SET [1:?] OF document_identifier_assigned_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<document_product_association>(const DB& db, const LIST& params, document_product_association* in)
{
    size_t base = 0;
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to document_product_association"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::document_product_association, 4>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to document_product_association to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::document_product_association, 4>::aux_is_derived[1] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to document_product_association to be a `text`")); }
    } while (0);
    do { // convert the 'relating_document' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::document_product_association, 4>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->relating_document, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to document_product_association to be a `document`")); }
    } while (0);
    do { // convert the 'related_product' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::document_product_association, 4>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->related_product, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to document_product_association to be a `product_or_formation_or_definition`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<document_product_equivalence>(const DB& db, const LIST& params, document_product_equivalence* in)
{
    size_t base = GenericFill(db, params, static_cast<document_product_association*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to document_product_equivalence"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<dose_equivalent_measure_with_unit>(const DB& db, const LIST& params, dose_equivalent_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to dose_equivalent_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<dose_equivalent_unit>(const DB& db, const LIST& params, dose_equivalent_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to dose_equivalent_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<double_offset_shelled_solid>(const DB& db, const LIST& params, double_offset_shelled_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<shelled_solid*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to double_offset_shelled_solid"); }    do { // convert the 'thickness2' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->thickness2, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to double_offset_shelled_solid to be a `length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<item_defined_transformation>(const DB& db, const LIST& params, item_defined_transformation* in)
{
    size_t base = 0;
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to item_defined_transformation"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::item_defined_transformation, 4>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to item_defined_transformation to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::item_defined_transformation, 4>::aux_is_derived[1] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to item_defined_transformation to be a `text`")); }
    } while (0);
    do { // convert the 'transform_item_1' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::item_defined_transformation, 4>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->transform_item_1, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to item_defined_transformation to be a `representation_item`")); }
    } while (0);
    do { // convert the 'transform_item_2' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::item_defined_transformation, 4>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->transform_item_2, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to item_defined_transformation to be a `representation_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<transformation_with_derived_angle>(const DB& db, const LIST& params, transformation_with_derived_angle* in)
{
    size_t base = GenericFill(db, params, static_cast<item_defined_transformation*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to transformation_with_derived_angle"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<draped_defined_transformation>(const DB& db, const LIST& params, draped_defined_transformation* in)
{
    size_t base = GenericFill(db, params, static_cast<transformation_with_derived_angle*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to draped_defined_transformation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<draughting_annotation_occurrence>(const DB& db, const LIST& params, draughting_annotation_occurrence* in)
{
    size_t base = GenericFill(db, params, static_cast<annotation_occurrence*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to draughting_annotation_occurrence"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<draughting_elements>(const DB& db, const LIST& params, draughting_elements* in)
{
    size_t base = GenericFill(db, params, static_cast<draughting_callout*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to draughting_elements"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<draughting_model>(const DB& db, const LIST& params, draughting_model* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to draughting_model"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<item_identified_representation_usage>(const DB& db, const LIST& params, item_identified_representation_usage* in)
{
    size_t base = 0;
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to item_identified_representation_usage"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::item_identified_representation_usage, 5>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to item_identified_representation_usage to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::item_identified_representation_usage, 5>::aux_is_derived[1] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to item_identified_representation_usage to be a `text`")); }
    } while (0);
    do { // convert the 'definition' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::item_identified_representation_usage, 5>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->definition, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to item_identified_representation_usage to be a `represented_definition`")); }
    } while (0);
    do { // convert the 'used_representation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::item_identified_representation_usage, 5>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->used_representation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to item_identified_representation_usage to be a `representation`")); }
    } while (0);
    do { // convert the 'identified_item' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::item_identified_representation_usage, 5>::aux_is_derived[4] = true; break; }
        try { GenericConvert(in->identified_item, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to item_identified_representation_usage to be a `representation_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<draughting_model_item_association>(const DB& db, const LIST& params, draughting_model_item_association* in)
{
    size_t base = GenericFill(db, params, static_cast<item_identified_representation_usage*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to draughting_model_item_association"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<pre_defined_colour>(const DB& db, const LIST& params, pre_defined_colour* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<draughting_pre_defined_colour>(const DB& db, const LIST& params, draughting_pre_defined_colour* in)
{
    size_t base = GenericFill(db, params, static_cast<pre_defined_colour*>(in));
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<pre_defined_item>(const DB& db, const LIST& params, pre_defined_item* in)
{
    size_t base = 0;
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to pre_defined_item"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::pre_defined_item, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to pre_defined_item to be a `label`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<pre_defined_curve_font>(const DB& db, const LIST& params, pre_defined_curve_font* in)
{
    size_t base = GenericFill(db, params, static_cast<pre_defined_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to pre_defined_curve_font"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<draughting_pre_defined_curve_font>(const DB& db, const LIST& params, draughting_pre_defined_curve_font* in)
{
    size_t base = GenericFill(db, params, static_cast<pre_defined_curve_font*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to draughting_pre_defined_curve_font"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<pre_defined_text_font>(const DB& db, const LIST& params, pre_defined_text_font* in)
{
    size_t base = GenericFill(db, params, static_cast<pre_defined_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to pre_defined_text_font"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<draughting_pre_defined_text_font>(const DB& db, const LIST& params, draughting_pre_defined_text_font* in)
{
    size_t base = GenericFill(db, params, static_cast<pre_defined_text_font*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to draughting_pre_defined_text_font"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<draughting_subfigure_representation>(const DB& db, const LIST& params, draughting_subfigure_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<symbol_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to draughting_subfigure_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<draughting_symbol_representation>(const DB& db, const LIST& params, draughting_symbol_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<symbol_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to draughting_symbol_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<text_literal>(const DB& db, const LIST& params, text_literal* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to text_literal"); }    do { // convert the 'literal' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::text_literal, 5>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->literal, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to text_literal to be a `presentable_text`")); }
    } while (0);
    do { // convert the 'placement' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::text_literal, 5>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->placement, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to text_literal to be a `axis2_placement`")); }
    } while (0);
    do { // convert the 'alignment' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::text_literal, 5>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->alignment, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to text_literal to be a `text_alignment`")); }
    } while (0);
    do { // convert the 'path' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::text_literal, 5>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->path, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to text_literal to be a `text_path`")); }
    } while (0);
    do { // convert the 'font' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::text_literal, 5>::aux_is_derived[4] = true; break; }
        try { GenericConvert(in->font, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to text_literal to be a `font_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<text_literal_with_delineation>(const DB& db, const LIST& params, text_literal_with_delineation* in)
{
    size_t base = GenericFill(db, params, static_cast<text_literal*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to text_literal_with_delineation"); }    do { // convert the 'delineation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::text_literal_with_delineation, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->delineation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to text_literal_with_delineation to be a `text_delineation`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<draughting_text_literal_with_delineation>(const DB& db, const LIST& params, draughting_text_literal_with_delineation* in)
{
    size_t base = GenericFill(db, params, static_cast<text_literal_with_delineation*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to draughting_text_literal_with_delineation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<presentation_set>(const DB& db, const LIST& params, presentation_set* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<drawing_revision>(const DB& db, const LIST& params, drawing_revision* in)
{
    size_t base = GenericFill(db, params, static_cast<presentation_set*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to drawing_revision"); }    do { // convert the 'revision_identifier' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->revision_identifier, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to drawing_revision to be a `identifier`")); }
    } while (0);
    do { // convert the 'drawing_identifier' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->drawing_identifier, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to drawing_revision to be a `drawing_definition`")); }
    } while (0);
    do { // convert the 'intended_scale' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->intended_scale, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to drawing_revision to be a `text`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<presentation_representation>(const DB& db, const LIST& params, presentation_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to presentation_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<presentation_area>(const DB& db, const LIST& params, presentation_area* in)
{
    size_t base = GenericFill(db, params, static_cast<presentation_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to presentation_area"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<drawing_sheet_revision>(const DB& db, const LIST& params, drawing_sheet_revision* in)
{
    size_t base = GenericFill(db, params, static_cast<presentation_area*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to drawing_sheet_revision"); }    do { // convert the 'revision_identifier' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->revision_identifier, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to drawing_sheet_revision to be a `identifier`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<drawing_sheet_revision_sequence>(const DB& db, const LIST& params, drawing_sheet_revision_sequence* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to drawing_sheet_revision_sequence"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<drawing_sheet_revision_usage>(const DB& db, const LIST& params, drawing_sheet_revision_usage* in)
{
    size_t base = GenericFill(db, params, static_cast<area_in_set*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to drawing_sheet_revision_usage"); }    do { // convert the 'sheet_number' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->sheet_number, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to drawing_sheet_revision_usage to be a `identifier`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<edge>(const DB& db, const LIST& params, edge* in)
{
    size_t base = GenericFill(db, params, static_cast<topological_representation_item*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to edge"); }    do { // convert the 'edge_start' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::edge, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->edge_start, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to edge to be a `vertex`")); }
    } while (0);
    do { // convert the 'edge_end' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::edge, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->edge_end, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to edge to be a `vertex`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<edge_based_wireframe_model>(const DB& db, const LIST& params, edge_based_wireframe_model* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to edge_based_wireframe_model"); }    do { // convert the 'ebwm_boundary' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->ebwm_boundary, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to edge_based_wireframe_model to be a `SET [1:?] OF connected_edge_set`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<edge_based_wireframe_shape_representation>(const DB& db, const LIST& params, edge_based_wireframe_shape_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to edge_based_wireframe_shape_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<edge_blended_solid>(const DB& db, const LIST& params, edge_blended_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<modified_solid*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to edge_blended_solid"); }    do { // convert the 'blended_edges' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::edge_blended_solid, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->blended_edges, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to edge_blended_solid to be a `LIST [1:?] OF edge_curve`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<edge_curve>(const DB& db, const LIST& params, edge_curve* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to edge_curve"); }    do { // convert the 'edge_geometry' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->edge_geometry, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to edge_curve to be a `curve`")); }
    } while (0);
    do { // convert the 'same_sense' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->same_sense, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to edge_curve to be a `BOOLEAN`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<edge_loop>(const DB& db, const LIST& params, edge_loop* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<electric_charge_measure_with_unit>(const DB& db, const LIST& params, electric_charge_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to electric_charge_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<electric_charge_unit>(const DB& db, const LIST& params, electric_charge_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to electric_charge_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<electric_current_measure_with_unit>(const DB& db, const LIST& params, electric_current_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to electric_current_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<electric_current_unit>(const DB& db, const LIST& params, electric_current_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<named_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to electric_current_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<electric_potential_measure_with_unit>(const DB& db, const LIST& params, electric_potential_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to electric_potential_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<electric_potential_unit>(const DB& db, const LIST& params, electric_potential_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to electric_potential_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<elementary_brep_shape_representation>(const DB& db, const LIST& params, elementary_brep_shape_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to elementary_brep_shape_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<ellipse>(const DB& db, const LIST& params, ellipse* in)
{
    size_t base = GenericFill(db, params, static_cast<conic*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to ellipse"); }    do { // convert the 'semi_axis_1' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->semi_axis_1, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to ellipse to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'semi_axis_2' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->semi_axis_2, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to ellipse to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<energy_measure_with_unit>(const DB& db, const LIST& params, energy_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to energy_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<energy_unit>(const DB& db, const LIST& params, energy_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to energy_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<property_definition>(const DB& db, const LIST& params, property_definition* in)
{
    size_t base = 0;
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to property_definition"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::property_definition, 3>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to property_definition to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::property_definition, 3>::aux_is_derived[1] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to property_definition to be a `text`")); }
    } while (0);
    do { // convert the 'definition' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::property_definition, 3>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->definition, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to property_definition to be a `characterized_definition`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<fact_type>(const DB& db, const LIST& params, fact_type* in)
{
    size_t base = GenericFill(db, params, static_cast<property_definition*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to fact_type"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<entity_assertion>(const DB& db, const LIST& params, entity_assertion* in)
{
    size_t base = GenericFill(db, params, static_cast<fact_type*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to entity_assertion"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<enum_reference_prefix>(const DB& db, const LIST& params, enum_reference_prefix* in)
{
    size_t base = GenericFill(db, params, static_cast<descriptive_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to enum_reference_prefix"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<evaluated_characteristic>(const DB& db, const LIST& params, evaluated_characteristic* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<evaluated_degenerate_pcurve>(const DB& db, const LIST& params, evaluated_degenerate_pcurve* in)
{
    size_t base = GenericFill(db, params, static_cast<degenerate_pcurve*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to evaluated_degenerate_pcurve"); }    do { // convert the 'equivalent_point' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->equivalent_point, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to evaluated_degenerate_pcurve to be a `cartesian_point`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<evaluation_product_definition>(const DB& db, const LIST& params, evaluation_product_definition* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to evaluation_product_definition"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<event_occurrence>(const DB& db, const LIST& params, event_occurrence* in)
{
    size_t base = 0;
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to event_occurrence"); }    do { // convert the 'id' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::event_occurrence, 3>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->id, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to event_occurrence to be a `identifier`")); }
    } while (0);
    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::event_occurrence, 3>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to event_occurrence to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::event_occurrence, 3>::aux_is_derived[2] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to event_occurrence to be a `text`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_concept_feature_category>(const DB& db, const LIST& params, product_concept_feature_category* in)
{
    size_t base = GenericFill(db, params, static_cast<group*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to product_concept_feature_category"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<exclusive_product_concept_feature_category>(const DB& db, const LIST& params, exclusive_product_concept_feature_category* in)
{
    size_t base = GenericFill(db, params, static_cast<product_concept_feature_category*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to exclusive_product_concept_feature_category"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<uncertainty_qualifier>(const DB& db, const LIST& params, uncertainty_qualifier* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to uncertainty_qualifier"); }    do { // convert the 'measure_name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::uncertainty_qualifier, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->measure_name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to uncertainty_qualifier to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::uncertainty_qualifier, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to uncertainty_qualifier to be a `text`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<standard_uncertainty>(const DB& db, const LIST& params, standard_uncertainty* in)
{
    size_t base = GenericFill(db, params, static_cast<uncertainty_qualifier*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to standard_uncertainty"); }    do { // convert the 'uncertainty_value' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::standard_uncertainty, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->uncertainty_value, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to standard_uncertainty to be a `REAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<expanded_uncertainty>(const DB& db, const LIST& params, expanded_uncertainty* in)
{
    size_t base = GenericFill(db, params, static_cast<standard_uncertainty*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to expanded_uncertainty"); }    do { // convert the 'coverage_factor' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->coverage_factor, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to expanded_uncertainty to be a `REAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<representation_item_relationship>(const DB& db, const LIST& params, representation_item_relationship* in)
{
    size_t base = 0;
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to representation_item_relationship"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::representation_item_relationship, 4>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to representation_item_relationship to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::representation_item_relationship, 4>::aux_is_derived[1] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to representation_item_relationship to be a `text`")); }
    } while (0);
    do { // convert the 'relating_representation_item' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::representation_item_relationship, 4>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->relating_representation_item, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to representation_item_relationship to be a `representation_item`")); }
    } while (0);
    do { // convert the 'related_representation_item' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::representation_item_relationship, 4>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->related_representation_item, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to representation_item_relationship to be a `representation_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<explicit_procedural_representation_item_relationship>(const DB& db, const LIST& params, explicit_procedural_representation_item_relationship* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_item_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to explicit_procedural_representation_item_relationship"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<explicit_procedural_geometric_representation_item_relationship>(const DB& db, const LIST& params, explicit_procedural_geometric_representation_item_relationship* in)
{
    size_t base = GenericFill(db, params, static_cast<explicit_procedural_representation_item_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to explicit_procedural_geometric_representation_item_relationship"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<explicit_procedural_representation_relationship>(const DB& db, const LIST& params, explicit_procedural_representation_relationship* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to explicit_procedural_representation_relationship"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<explicit_procedural_shape_representation_relationship>(const DB& db, const LIST& params, explicit_procedural_shape_representation_relationship* in)
{
    size_t base = GenericFill(db, params, static_cast<explicit_procedural_representation_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to explicit_procedural_shape_representation_relationship"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<expression_conversion_based_unit>(const DB& db, const LIST& params, expression_conversion_based_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<extension>(const DB& db, const LIST& params, extension* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_shape_aspect*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to extension"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<extent>(const DB& db, const LIST& params, extent* in)
{
    size_t base = GenericFill(db, params, static_cast<characterized_object*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to extent"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<external_source>(const DB& db, const LIST& params, external_source* in)
{
    size_t base = 0;
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to external_source"); }    do { // convert the 'source_id' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::external_source, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->source_id, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to external_source to be a `source_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<external_class_library>(const DB& db, const LIST& params, external_class_library* in)
{
    size_t base = GenericFill(db, params, static_cast<external_source*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to external_class_library"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_class>(const DB& db, const LIST& params, externally_defined_class* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_colour>(const DB& db, const LIST& params, externally_defined_colour* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_context_dependent_unit>(const DB& db, const LIST& params, externally_defined_context_dependent_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_conversion_based_unit>(const DB& db, const LIST& params, externally_defined_conversion_based_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_currency>(const DB& db, const LIST& params, externally_defined_currency* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_item>(const DB& db, const LIST& params, externally_defined_item* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to externally_defined_item"); }    do { // convert the 'item_id' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::externally_defined_item, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->item_id, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to externally_defined_item to be a `source_item`")); }
    } while (0);
    do { // convert the 'source' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::externally_defined_item, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->source, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to externally_defined_item to be a `external_source`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_curve_font>(const DB& db, const LIST& params, externally_defined_curve_font* in)
{
    size_t base = GenericFill(db, params, static_cast<externally_defined_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to externally_defined_curve_font"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_dimension_definition>(const DB& db, const LIST& params, externally_defined_dimension_definition* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_general_property>(const DB& db, const LIST& params, externally_defined_general_property* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_hatch_style>(const DB& db, const LIST& params, externally_defined_hatch_style* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_marker>(const DB& db, const LIST& params, externally_defined_marker* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<picture_representation_item>(const DB& db, const LIST& params, picture_representation_item* in)
{
    size_t base = GenericFill(db, params, static_cast<bytes_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to picture_representation_item"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_picture_representation_item>(const DB& db, const LIST& params, externally_defined_picture_representation_item* in)
{
    size_t base = GenericFill(db, params, static_cast<picture_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to externally_defined_picture_representation_item"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_representation_item>(const DB& db, const LIST& params, externally_defined_representation_item* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_string>(const DB& db, const LIST& params, externally_defined_string* in)
{
    size_t base = GenericFill(db, params, static_cast<externally_defined_representation_item*>(in));
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_symbol>(const DB& db, const LIST& params, externally_defined_symbol* in)
{
    size_t base = GenericFill(db, params, static_cast<externally_defined_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to externally_defined_symbol"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_terminator_symbol>(const DB& db, const LIST& params, externally_defined_terminator_symbol* in)
{
    size_t base = GenericFill(db, params, static_cast<externally_defined_symbol*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to externally_defined_terminator_symbol"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_text_font>(const DB& db, const LIST& params, externally_defined_text_font* in)
{
    size_t base = GenericFill(db, params, static_cast<externally_defined_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to externally_defined_text_font"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_tile>(const DB& db, const LIST& params, externally_defined_tile* in)
{
    size_t base = GenericFill(db, params, static_cast<externally_defined_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to externally_defined_tile"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<externally_defined_tile_style>(const DB& db, const LIST& params, externally_defined_tile_style* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<swept_area_solid>(const DB& db, const LIST& params, swept_area_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_model*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to swept_area_solid"); }    do { // convert the 'swept_area' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::swept_area_solid, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->swept_area, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to swept_area_solid to be a `curve_bounded_surface`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<extruded_area_solid>(const DB& db, const LIST& params, extruded_area_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<swept_area_solid*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to extruded_area_solid"); }    do { // convert the 'extruded_direction' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->extruded_direction, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to extruded_area_solid to be a `direction`")); }
    } while (0);
    do { // convert the 'depth' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->depth, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to extruded_area_solid to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<swept_face_solid>(const DB& db, const LIST& params, swept_face_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_model*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to swept_face_solid"); }    do { // convert the 'swept_face' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::swept_face_solid, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->swept_face, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to swept_face_solid to be a `face_surface`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<extruded_face_solid>(const DB& db, const LIST& params, extruded_face_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<swept_face_solid*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to extruded_face_solid"); }    do { // convert the 'extruded_direction' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::extruded_face_solid, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->extruded_direction, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to extruded_face_solid to be a `direction`")); }
    } while (0);
    do { // convert the 'depth' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::extruded_face_solid, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->depth, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to extruded_face_solid to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<extruded_face_solid_with_trim_conditions>(const DB& db, const LIST& params, extruded_face_solid_with_trim_conditions* in)
{
    size_t base = GenericFill(db, params, static_cast<extruded_face_solid*>(in));
    if (params.GetSize() < 10) { throw STEP::TypeError("expected 10 arguments to extruded_face_solid_with_trim_conditions"); }    do { // convert the 'first_trim_condition' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::extruded_face_solid_with_trim_conditions, 6>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->first_trim_condition, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to extruded_face_solid_with_trim_conditions to be a `trim_condition_select`")); }
    } while (0);
    do { // convert the 'second_trim_condition' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::extruded_face_solid_with_trim_conditions, 6>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->second_trim_condition, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to extruded_face_solid_with_trim_conditions to be a `trim_condition_select`")); }
    } while (0);
    do { // convert the 'first_trim_intent' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::extruded_face_solid_with_trim_conditions, 6>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->first_trim_intent, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to extruded_face_solid_with_trim_conditions to be a `trim_intent`")); }
    } while (0);
    do { // convert the 'second_trim_intent' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::extruded_face_solid_with_trim_conditions, 6>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->second_trim_intent, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to extruded_face_solid_with_trim_conditions to be a `trim_intent`")); }
    } while (0);
    do { // convert the 'first_offset' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::extruded_face_solid_with_trim_conditions, 6>::aux_is_derived[4] = true; break; }
        try { GenericConvert(in->first_offset, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 8 to extruded_face_solid_with_trim_conditions to be a `non_negative_length_measure`")); }
    } while (0);
    do { // convert the 'second_offset' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::extruded_face_solid_with_trim_conditions, 6>::aux_is_derived[5] = true; break; }
        try { GenericConvert(in->second_offset, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 9 to extruded_face_solid_with_trim_conditions to be a `non_negative_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<extruded_face_solid_with_draft_angle>(const DB& db, const LIST& params, extruded_face_solid_with_draft_angle* in)
{
    size_t base = GenericFill(db, params, static_cast<extruded_face_solid_with_trim_conditions*>(in));
    if (params.GetSize() < 11) { throw STEP::TypeError("expected 11 arguments to extruded_face_solid_with_draft_angle"); }    do { // convert the 'draft_angle' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->draft_angle, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 10 to extruded_face_solid_with_draft_angle to be a `plane_angle_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<extruded_face_solid_with_multiple_draft_angles>(const DB& db, const LIST& params, extruded_face_solid_with_multiple_draft_angles* in)
{
    size_t base = GenericFill(db, params, static_cast<extruded_face_solid_with_trim_conditions*>(in));
    if (params.GetSize() < 11) { throw STEP::TypeError("expected 11 arguments to extruded_face_solid_with_multiple_draft_angles"); }    do { // convert the 'draft_angles' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->draft_angles, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 10 to extruded_face_solid_with_multiple_draft_angles to be a `LIST [2:?] OF plane_angle_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<face>(const DB& db, const LIST& params, face* in)
{
    size_t base = GenericFill(db, params, static_cast<topological_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to face"); }    do { // convert the 'bounds' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::face, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->bounds, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to face to be a `SET [1:?] OF face_bound`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<face_based_surface_model>(const DB& db, const LIST& params, face_based_surface_model* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to face_based_surface_model"); }    do { // convert the 'fbsm_faces' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->fbsm_faces, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to face_based_surface_model to be a `SET [1:?] OF connected_face_set`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<face_bound>(const DB& db, const LIST& params, face_bound* in)
{
    size_t base = GenericFill(db, params, static_cast<topological_representation_item*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to face_bound"); }    do { // convert the 'bound' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::face_bound, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->bound, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to face_bound to be a `loop`")); }
    } while (0);
    do { // convert the 'orientation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::face_bound, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->orientation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to face_bound to be a `BOOLEAN`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<face_outer_bound>(const DB& db, const LIST& params, face_outer_bound* in)
{
    size_t base = GenericFill(db, params, static_cast<face_bound*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to face_outer_bound"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<faceted_brep>(const DB& db, const LIST& params, faceted_brep* in)
{
    size_t base = GenericFill(db, params, static_cast<manifold_solid_brep*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to faceted_brep"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<faceted_brep_shape_representation>(const DB& db, const LIST& params, faceted_brep_shape_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to faceted_brep_shape_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<fill_area_style>(const DB& db, const LIST& params, fill_area_style* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to fill_area_style"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to fill_area_style to be a `label`")); }
    } while (0);
    do { // convert the 'fill_styles' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->fill_styles, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to fill_area_style to be a `SET [1:?] OF fill_style_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<fill_area_style_hatching>(const DB& db, const LIST& params, fill_area_style_hatching* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to fill_area_style_hatching"); }    do { // convert the 'hatch_line_appearance' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->hatch_line_appearance, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to fill_area_style_hatching to be a `curve_style`")); }
    } while (0);
    do { // convert the 'start_of_next_hatch_line' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->start_of_next_hatch_line, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to fill_area_style_hatching to be a `one_direction_repeat_factor`")); }
    } while (0);
    do { // convert the 'point_of_reference_hatch_line' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->point_of_reference_hatch_line, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to fill_area_style_hatching to be a `cartesian_point`")); }
    } while (0);
    do { // convert the 'pattern_start' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->pattern_start, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to fill_area_style_hatching to be a `cartesian_point`")); }
    } while (0);
    do { // convert the 'hatch_line_angle' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->hatch_line_angle, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to fill_area_style_hatching to be a `plane_angle_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<fill_area_style_tile_coloured_region>(const DB& db, const LIST& params, fill_area_style_tile_coloured_region* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to fill_area_style_tile_coloured_region"); }    do { // convert the 'closed_curve' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->closed_curve, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to fill_area_style_tile_coloured_region to be a `curve_or_annotation_curve_occurrence`")); }
    } while (0);
    do { // convert the 'region_colour' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->region_colour, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to fill_area_style_tile_coloured_region to be a `colour`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<fill_area_style_tile_curve_with_style>(const DB& db, const LIST& params, fill_area_style_tile_curve_with_style* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to fill_area_style_tile_curve_with_style"); }    do { // convert the 'styled_curve' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->styled_curve, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to fill_area_style_tile_curve_with_style to be a `annotation_curve_occurrence`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<fill_area_style_tile_symbol_with_style>(const DB& db, const LIST& params, fill_area_style_tile_symbol_with_style* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to fill_area_style_tile_symbol_with_style"); }    do { // convert the 'symbol' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->symbol, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to fill_area_style_tile_symbol_with_style to be a `annotation_symbol_occurrence`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<fill_area_style_tiles>(const DB& db, const LIST& params, fill_area_style_tiles* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to fill_area_style_tiles"); }    do { // convert the 'tiling_pattern' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->tiling_pattern, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to fill_area_style_tiles to be a `two_direction_repeat_factor`")); }
    } while (0);
    do { // convert the 'tiles' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->tiles, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to fill_area_style_tiles to be a `SET [1:?] OF fill_area_style_tile_shape_select`")); }
    } while (0);
    do { // convert the 'tiling_scale' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->tiling_scale, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to fill_area_style_tiles to be a `positive_ratio_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<shape_representation_relationship>(const DB& db, const LIST& params, shape_representation_relationship* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to shape_representation_relationship"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<flat_pattern_ply_representation_relationship>(const DB& db, const LIST& params, flat_pattern_ply_representation_relationship* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_representation_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to flat_pattern_ply_representation_relationship"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<flatness_tolerance>(const DB& db, const LIST& params, flatness_tolerance* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_tolerance*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to flatness_tolerance"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<force_measure_with_unit>(const DB& db, const LIST& params, force_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to force_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<force_unit>(const DB& db, const LIST& params, force_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to force_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<forward_chaining_rule>(const DB& db, const LIST& params, forward_chaining_rule* in)
{
    size_t base = GenericFill(db, params, static_cast<rule_definition*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to forward_chaining_rule"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<forward_chaining_rule_premise>(const DB& db, const LIST& params, forward_chaining_rule_premise* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<frequency_measure_with_unit>(const DB& db, const LIST& params, frequency_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to frequency_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<frequency_unit>(const DB& db, const LIST& params, frequency_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to frequency_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<func>(const DB& db, const LIST& params, func* in)
{
    size_t base = GenericFill(db, params, static_cast<compound_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to func"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<functional_breakdown_context>(const DB& db, const LIST& params, functional_breakdown_context* in)
{
    size_t base = GenericFill(db, params, static_cast<breakdown_context*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to functional_breakdown_context"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<functional_element_usage>(const DB& db, const LIST& params, functional_element_usage* in)
{
    size_t base = GenericFill(db, params, static_cast<breakdown_element_usage*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to functional_element_usage"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<general_material_property>(const DB& db, const LIST& params, general_material_property* in)
{
    size_t base = GenericFill(db, params, static_cast<general_property*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to general_material_property"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<simple_generic_expression>(const DB& db, const LIST& params, simple_generic_expression* in)
{
    size_t base = GenericFill(db, params, static_cast<generic_expression*>(in));
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<generic_literal>(const DB& db, const LIST& params, generic_literal* in)
{
    size_t base = GenericFill(db, params, static_cast<simple_generic_expression*>(in));
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<generic_variable>(const DB& db, const LIST& params, generic_variable* in)
{
    size_t base = GenericFill(db, params, static_cast<simple_generic_expression*>(in));
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<geometric_alignment>(const DB& db, const LIST& params, geometric_alignment* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_shape_aspect*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to geometric_alignment"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<geometric_set>(const DB& db, const LIST& params, geometric_set* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to geometric_set"); }    do { // convert the 'elements' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::geometric_set, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->elements, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to geometric_set to be a `SET [1:?] OF geometric_set_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<geometric_curve_set>(const DB& db, const LIST& params, geometric_curve_set* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_set*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to geometric_curve_set"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<geometric_intersection>(const DB& db, const LIST& params, geometric_intersection* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_shape_aspect*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to geometric_intersection"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<geometric_item_specific_usage>(const DB& db, const LIST& params, geometric_item_specific_usage* in)
{
    size_t base = GenericFill(db, params, static_cast<item_identified_representation_usage*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to geometric_item_specific_usage"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<geometric_model_element_relationship>(const DB& db, const LIST& params, geometric_model_element_relationship* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<representation_context>(const DB& db, const LIST& params, representation_context* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to representation_context"); }    do { // convert the 'context_identifier' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::representation_context, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->context_identifier, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to representation_context to be a `identifier`")); }
    } while (0);
    do { // convert the 'context_type' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::representation_context, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->context_type, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to representation_context to be a `text`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<geometric_representation_context>(const DB& db, const LIST& params, geometric_representation_context* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_context*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to geometric_representation_context"); }    do { // convert the 'coordinate_space_dimension' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->coordinate_space_dimension, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to geometric_representation_context to be a `dimension_count`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<geometric_tolerance_with_defined_unit>(const DB& db, const LIST& params, geometric_tolerance_with_defined_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_tolerance*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to geometric_tolerance_with_defined_unit"); }    do { // convert the 'unit_size' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->unit_size, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to geometric_tolerance_with_defined_unit to be a `measure_with_unit`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<geometrical_tolerance_callout>(const DB& db, const LIST& params, geometrical_tolerance_callout* in)
{
    size_t base = GenericFill(db, params, static_cast<draughting_callout*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to geometrical_tolerance_callout"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<geometrically_bounded_2d_wireframe_representation>(const DB& db, const LIST& params, geometrically_bounded_2d_wireframe_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to geometrically_bounded_2d_wireframe_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<geometrically_bounded_surface_shape_representation>(const DB& db, const LIST& params, geometrically_bounded_surface_shape_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to geometrically_bounded_surface_shape_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<geometrically_bounded_wireframe_shape_representation>(const DB& db, const LIST& params, geometrically_bounded_wireframe_shape_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to geometrically_bounded_wireframe_shape_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<global_assignment>(const DB& db, const LIST& params, global_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_item_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to global_assignment"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<global_uncertainty_assigned_context>(const DB& db, const LIST& params, global_uncertainty_assigned_context* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_context*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to global_uncertainty_assigned_context"); }    do { // convert the 'uncertainty' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->uncertainty, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to global_uncertainty_assigned_context to be a `SET [1:?] OF uncertainty_measure_with_unit`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<global_unit_assigned_context>(const DB& db, const LIST& params, global_unit_assigned_context* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_context*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to global_unit_assigned_context"); }    do { // convert the 'units' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->units, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to global_unit_assigned_context to be a `SET [1:?] OF unit`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<ground_fact>(const DB& db, const LIST& params, ground_fact* in)
{
    size_t base = GenericFill(db, params, static_cast<atomic_formula*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to ground_fact"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<hardness_representation>(const DB& db, const LIST& params, hardness_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to hardness_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<hidden_element_over_riding_styled_item>(const DB& db, const LIST& params, hidden_element_over_riding_styled_item* in)
{
    size_t base = GenericFill(db, params, static_cast<context_dependent_over_riding_styled_item*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to hidden_element_over_riding_styled_item"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<hyperbola>(const DB& db, const LIST& params, hyperbola* in)
{
    size_t base = GenericFill(db, params, static_cast<conic*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to hyperbola"); }    do { // convert the 'semi_axis' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->semi_axis, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to hyperbola to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'semi_imag_axis' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->semi_imag_axis, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to hyperbola to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<illuminance_measure_with_unit>(const DB& db, const LIST& params, illuminance_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to illuminance_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<illuminance_unit>(const DB& db, const LIST& params, illuminance_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to illuminance_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<included_text_block>(const DB& db, const LIST& params, included_text_block* in)
{
    size_t base = GenericFill(db, params, static_cast<mapped_item*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to included_text_block"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<inclusion_product_concept_feature>(const DB& db, const LIST& params, inclusion_product_concept_feature* in)
{
    size_t base = GenericFill(db, params, static_cast<conditional_concept_feature*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to inclusion_product_concept_feature"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<user_selected_elements>(const DB& db, const LIST& params, user_selected_elements* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to user_selected_elements"); }    do { // convert the 'picked_items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::user_selected_elements, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->picked_items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to user_selected_elements to be a `SET [1:?] OF representation_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<indirectly_selected_elements>(const DB& db, const LIST& params, indirectly_selected_elements* in)
{
    size_t base = GenericFill(db, params, static_cast<user_selected_elements*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to indirectly_selected_elements"); }    do { // convert the 'indirectly_picked_items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->indirectly_picked_items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to indirectly_selected_elements to be a `SET [1:?] OF representation_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<indirectly_selected_shape_elements>(const DB& db, const LIST& params, indirectly_selected_shape_elements* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<inductance_measure_with_unit>(const DB& db, const LIST& params, inductance_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to inductance_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<inductance_unit>(const DB& db, const LIST& params, inductance_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to inductance_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<information_right>(const DB& db, const LIST& params, information_right* in)
{
    size_t base = GenericFill(db, params, static_cast<action_method*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to information_right"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<information_usage_right>(const DB& db, const LIST& params, information_usage_right* in)
{
    size_t base = GenericFill(db, params, static_cast<action_method*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to information_usage_right"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<instance_usage_context_assignment>(const DB& db, const LIST& params, instance_usage_context_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition_context*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to instance_usage_context_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to instance_usage_context_assignment to be a `SET [1:?] OF instance_usage_context_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<instanced_feature>(const DB& db, const LIST& params, instanced_feature* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<literal_number>(const DB& db, const LIST& params, literal_number* in)
{
    size_t base = 0;
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to literal_number"); }    do { // convert the 'the_value' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::literal_number, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->the_value, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to literal_number to be a `NUMBER`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<int_literal>(const DB& db, const LIST& params, int_literal* in)
{
    size_t base = GenericFill(db, params, static_cast<literal_number*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to int_literal"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<integer_representation_item>(const DB& db, const LIST& params, integer_representation_item* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_curve>(const DB& db, const LIST& params, surface_curve* in)
{
    size_t base = GenericFill(db, params, static_cast<curve*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to surface_curve"); }    do { // convert the 'curve_3d' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::surface_curve, 3>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->curve_3d, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to surface_curve to be a `curve`")); }
    } while (0);
    do { // convert the 'associated_geometry' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::surface_curve, 3>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->associated_geometry, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to surface_curve to be a `LIST [1:2] OF pcurve_or_surface`")); }
    } while (0);
    do { // convert the 'master_representation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::surface_curve, 3>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->master_representation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to surface_curve to be a `preferred_surface_curve_representation`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<intersection_curve>(const DB& db, const LIST& params, intersection_curve* in)
{
    size_t base = GenericFill(db, params, static_cast<surface_curve*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to intersection_curve"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<interval_expression>(const DB& db, const LIST& params, interval_expression* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<iso4217_currency>(const DB& db, const LIST& params, iso4217_currency* in)
{
    size_t base = GenericFill(db, params, static_cast<currency*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to iso4217_currency"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<known_source>(const DB& db, const LIST& params, known_source* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<laid_defined_transformation>(const DB& db, const LIST& params, laid_defined_transformation* in)
{
    size_t base = GenericFill(db, params, static_cast<transformation_with_derived_angle*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to laid_defined_transformation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<language>(const DB& db, const LIST& params, language* in)
{
    size_t base = GenericFill(db, params, static_cast<group*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to language"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<leader_curve>(const DB& db, const LIST& params, leader_curve* in)
{
    size_t base = GenericFill(db, params, static_cast<annotation_curve_occurrence*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to leader_curve"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<leader_directed_callout>(const DB& db, const LIST& params, leader_directed_callout* in)
{
    size_t base = GenericFill(db, params, static_cast<draughting_callout*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to leader_directed_callout"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<leader_directed_dimension>(const DB& db, const LIST& params, leader_directed_dimension* in)
{
    size_t base = GenericFill(db, params, static_cast<leader_directed_callout*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to leader_directed_dimension"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<leader_terminator>(const DB& db, const LIST& params, leader_terminator* in)
{
    size_t base = GenericFill(db, params, static_cast<terminator_symbol*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to leader_terminator"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<length_measure_with_unit>(const DB& db, const LIST& params, length_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to length_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<length_unit>(const DB& db, const LIST& params, length_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<named_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to length_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<light_source>(const DB& db, const LIST& params, light_source* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to light_source"); }    do { // convert the 'light_colour' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::light_source, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->light_colour, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to light_source to be a `colour`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<light_source_ambient>(const DB& db, const LIST& params, light_source_ambient* in)
{
    size_t base = GenericFill(db, params, static_cast<light_source*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to light_source_ambient"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<light_source_directional>(const DB& db, const LIST& params, light_source_directional* in)
{
    size_t base = GenericFill(db, params, static_cast<light_source*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to light_source_directional"); }    do { // convert the 'orientation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->orientation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to light_source_directional to be a `direction`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<light_source_positional>(const DB& db, const LIST& params, light_source_positional* in)
{
    size_t base = GenericFill(db, params, static_cast<light_source*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to light_source_positional"); }    do { // convert the 'position' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->position, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to light_source_positional to be a `cartesian_point`")); }
    } while (0);
    do { // convert the 'constant_attenuation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->constant_attenuation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to light_source_positional to be a `REAL`")); }
    } while (0);
    do { // convert the 'distance_attenuation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->distance_attenuation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to light_source_positional to be a `REAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<light_source_spot>(const DB& db, const LIST& params, light_source_spot* in)
{
    size_t base = GenericFill(db, params, static_cast<light_source*>(in));
    if (params.GetSize() < 8) { throw STEP::TypeError("expected 8 arguments to light_source_spot"); }    do { // convert the 'position' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->position, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to light_source_spot to be a `cartesian_point`")); }
    } while (0);
    do { // convert the 'orientation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->orientation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to light_source_spot to be a `direction`")); }
    } while (0);
    do { // convert the 'concentration_exponent' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->concentration_exponent, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to light_source_spot to be a `REAL`")); }
    } while (0);
    do { // convert the 'constant_attenuation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->constant_attenuation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to light_source_spot to be a `REAL`")); }
    } while (0);
    do { // convert the 'distance_attenuation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->distance_attenuation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to light_source_spot to be a `REAL`")); }
    } while (0);
    do { // convert the 'spread_angle' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->spread_angle, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to light_source_spot to be a `positive_plane_angle_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<line>(const DB& db, const LIST& params, line* in)
{
    size_t base = GenericFill(db, params, static_cast<curve*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to line"); }    do { // convert the 'pnt' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->pnt, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to line to be a `cartesian_point`")); }
    } while (0);
    do { // convert the 'dir' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->dir, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to line to be a `vector`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<line_profile_tolerance>(const DB& db, const LIST& params, line_profile_tolerance* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_tolerance*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to line_profile_tolerance"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<linear_dimension>(const DB& db, const LIST& params, linear_dimension* in)
{
    size_t base = GenericFill(db, params, static_cast<dimension_curve_directed_callout*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to linear_dimension"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<simple_clause>(const DB& db, const LIST& params, simple_clause* in)
{
    size_t base = GenericFill(db, params, static_cast<compound_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to simple_clause"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<literal_conjunction>(const DB& db, const LIST& params, literal_conjunction* in)
{
    size_t base = GenericFill(db, params, static_cast<simple_clause*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to literal_conjunction"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<literal_disjunction>(const DB& db, const LIST& params, literal_disjunction* in)
{
    size_t base = GenericFill(db, params, static_cast<simple_clause*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to literal_disjunction"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<logical_literal>(const DB& db, const LIST& params, logical_literal* in)
{
    size_t base = GenericFill(db, params, static_cast<generic_literal*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to logical_literal"); }    do { // convert the 'lit_value' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->lit_value, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to logical_literal to be a `LOGICAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<logical_representation_item>(const DB& db, const LIST& params, logical_representation_item* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<loop>(const DB& db, const LIST& params, loop* in)
{
    size_t base = GenericFill(db, params, static_cast<topological_representation_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to loop"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<loss_tangent_measure_with_unit>(const DB& db, const LIST& params, loss_tangent_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<ratio_measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to loss_tangent_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<lot_effectivity>(const DB& db, const LIST& params, lot_effectivity* in)
{
    size_t base = GenericFill(db, params, static_cast<effectivity*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to lot_effectivity"); }    do { // convert the 'effectivity_lot_id' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->effectivity_lot_id, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to lot_effectivity to be a `identifier`")); }
    } while (0);
    do { // convert the 'effectivity_lot_size' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->effectivity_lot_size, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to lot_effectivity to be a `measure_with_unit`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<luminous_flux_measure_with_unit>(const DB& db, const LIST& params, luminous_flux_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to luminous_flux_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<luminous_flux_unit>(const DB& db, const LIST& params, luminous_flux_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<named_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to luminous_flux_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<luminous_intensity_measure_with_unit>(const DB& db, const LIST& params, luminous_intensity_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to luminous_intensity_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<luminous_intensity_unit>(const DB& db, const LIST& params, luminous_intensity_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<named_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to luminous_intensity_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<magnetic_flux_density_measure_with_unit>(const DB& db, const LIST& params, magnetic_flux_density_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to magnetic_flux_density_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<magnetic_flux_density_unit>(const DB& db, const LIST& params, magnetic_flux_density_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to magnetic_flux_density_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<magnetic_flux_measure_with_unit>(const DB& db, const LIST& params, magnetic_flux_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to magnetic_flux_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<magnetic_flux_unit>(const DB& db, const LIST& params, magnetic_flux_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to magnetic_flux_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<make_from_usage_option>(const DB& db, const LIST& params, make_from_usage_option* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition_usage*>(in));
    if (params.GetSize() < 8) { throw STEP::TypeError("expected 8 arguments to make_from_usage_option"); }    do { // convert the 'ranking' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->ranking, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to make_from_usage_option to be a `INTEGER`")); }
    } while (0);
    do { // convert the 'ranking_rationale' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->ranking_rationale, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to make_from_usage_option to be a `text`")); }
    } while (0);
    do { // convert the 'quantity' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->quantity, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to make_from_usage_option to be a `measure_with_unit`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<manifold_subsurface_shape_representation>(const DB& db, const LIST& params, manifold_subsurface_shape_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to manifold_subsurface_shape_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<manifold_surface_shape_representation>(const DB& db, const LIST& params, manifold_surface_shape_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to manifold_surface_shape_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<mass_measure_with_unit>(const DB& db, const LIST& params, mass_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to mass_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<mass_unit>(const DB& db, const LIST& params, mass_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<named_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to mass_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<material_property>(const DB& db, const LIST& params, material_property* in)
{
    size_t base = GenericFill(db, params, static_cast<property_definition*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to material_property"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<property_definition_representation>(const DB& db, const LIST& params, property_definition_representation* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to property_definition_representation"); }    do { // convert the 'definition' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::property_definition_representation, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->definition, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to property_definition_representation to be a `represented_definition`")); }
    } while (0);
    do { // convert the 'used_representation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::property_definition_representation, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->used_representation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to property_definition_representation to be a `representation`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<material_property_representation>(const DB& db, const LIST& params, material_property_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<property_definition_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to material_property_representation"); }    do { // convert the 'dependent_environment' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->dependent_environment, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to material_property_representation to be a `data_environment`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<measure_representation_item>(const DB& db, const LIST& params, measure_representation_item* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_context>(const DB& db, const LIST& params, product_context* in)
{
    size_t base = GenericFill(db, params, static_cast<application_context_element*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to product_context"); }    do { // convert the 'discipline_type' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_context, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->discipline_type, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to product_context to be a `label`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<mechanical_context>(const DB& db, const LIST& params, mechanical_context* in)
{
    size_t base = GenericFill(db, params, static_cast<product_context*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to mechanical_context"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<mechanical_design_and_draughting_relationship>(const DB& db, const LIST& params, mechanical_design_and_draughting_relationship* in)
{
    size_t base = GenericFill(db, params, static_cast<definitional_representation_relationship_with_same_context*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to mechanical_design_and_draughting_relationship"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<mechanical_design_geometric_presentation_area>(const DB& db, const LIST& params, mechanical_design_geometric_presentation_area* in)
{
    size_t base = GenericFill(db, params, static_cast<presentation_area*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to mechanical_design_geometric_presentation_area"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<mechanical_design_geometric_presentation_representation>(const DB& db, const LIST& params, mechanical_design_geometric_presentation_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to mechanical_design_geometric_presentation_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<mechanical_design_presentation_representation_with_draughting>(const DB& db, const LIST& params, mechanical_design_presentation_representation_with_draughting* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to mechanical_design_presentation_representation_with_draughting"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<mechanical_design_shaded_presentation_area>(const DB& db, const LIST& params, mechanical_design_shaded_presentation_area* in)
{
    size_t base = GenericFill(db, params, static_cast<presentation_area*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to mechanical_design_shaded_presentation_area"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<mechanical_design_shaded_presentation_representation>(const DB& db, const LIST& params, mechanical_design_shaded_presentation_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to mechanical_design_shaded_presentation_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<min_and_major_ply_orientation_basis>(const DB& db, const LIST& params, min_and_major_ply_orientation_basis* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<modified_geometric_tolerance>(const DB& db, const LIST& params, modified_geometric_tolerance* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_tolerance*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to modified_geometric_tolerance"); }    do { // convert the 'modifier' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->modifier, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to modified_geometric_tolerance to be a `limit_condition`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<modified_solid_with_placed_configuration>(const DB& db, const LIST& params, modified_solid_with_placed_configuration* in)
{
    size_t base = GenericFill(db, params, static_cast<modified_solid*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to modified_solid_with_placed_configuration"); }    do { // convert the 'placing' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::modified_solid_with_placed_configuration, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->placing, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to modified_solid_with_placed_configuration to be a `axis2_placement_3d`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<moments_of_inertia_representation>(const DB& db, const LIST& params, moments_of_inertia_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to moments_of_inertia_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<multi_language_attribute_assignment>(const DB& db, const LIST& params, multi_language_attribute_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<attribute_value_assignment*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to multi_language_attribute_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to multi_language_attribute_assignment to be a `SET [1:?] OF multi_language_attribute_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<multiple_arity_boolean_expression>(const DB& db, const LIST& params, multiple_arity_boolean_expression* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<multiple_arity_generic_expression>(const DB& db, const LIST& params, multiple_arity_generic_expression* in)
{
    size_t base = GenericFill(db, params, static_cast<generic_expression*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to multiple_arity_generic_expression"); }    do { // convert the 'operands' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->operands, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to multiple_arity_generic_expression to be a `LIST [2:?] OF generic_expression`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<multiple_arity_numeric_expression>(const DB& db, const LIST& params, multiple_arity_numeric_expression* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<next_assembly_usage_occurrence>(const DB& db, const LIST& params, next_assembly_usage_occurrence* in)
{
    size_t base = GenericFill(db, params, static_cast<assembly_component_usage*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to next_assembly_usage_occurrence"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<non_manifold_surface_shape_representation>(const DB& db, const LIST& params, non_manifold_surface_shape_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to non_manifold_surface_shape_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<null_representation_item>(const DB& db, const LIST& params, null_representation_item* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to null_representation_item"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<numeric_expression>(const DB& db, const LIST& params, numeric_expression* in)
{
    size_t base = GenericFill(db, params, static_cast<expression*>(in));
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<offset_curve_2d>(const DB& db, const LIST& params, offset_curve_2d* in)
{
    size_t base = GenericFill(db, params, static_cast<curve*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to offset_curve_2d"); }    do { // convert the 'basis_curve' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->basis_curve, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to offset_curve_2d to be a `curve`")); }
    } while (0);
    do { // convert the 'distance' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->distance, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to offset_curve_2d to be a `length_measure`")); }
    } while (0);
    do { // convert the 'self_intersect' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->self_intersect, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to offset_curve_2d to be a `LOGICAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<offset_curve_3d>(const DB& db, const LIST& params, offset_curve_3d* in)
{
    size_t base = GenericFill(db, params, static_cast<curve*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to offset_curve_3d"); }    do { // convert the 'basis_curve' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->basis_curve, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to offset_curve_3d to be a `curve`")); }
    } while (0);
    do { // convert the 'distance' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->distance, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to offset_curve_3d to be a `length_measure`")); }
    } while (0);
    do { // convert the 'self_intersect' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->self_intersect, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to offset_curve_3d to be a `LOGICAL`")); }
    } while (0);
    do { // convert the 'ref_direction' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->ref_direction, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to offset_curve_3d to be a `direction`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<offset_surface>(const DB& db, const LIST& params, offset_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<surface*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to offset_surface"); }    do { // convert the 'basis_surface' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->basis_surface, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to offset_surface to be a `surface`")); }
    } while (0);
    do { // convert the 'distance' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->distance, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to offset_surface to be a `length_measure`")); }
    } while (0);
    do { // convert the 'self_intersect' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->self_intersect, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to offset_surface to be a `LOGICAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<one_direction_repeat_factor>(const DB& db, const LIST& params, one_direction_repeat_factor* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to one_direction_repeat_factor"); }    do { // convert the 'repeat_factor' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::one_direction_repeat_factor, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->repeat_factor, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to one_direction_repeat_factor to be a `vector`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<open_shell>(const DB& db, const LIST& params, open_shell* in)
{
    size_t base = GenericFill(db, params, static_cast<connected_face_set*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to open_shell"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<ordinal_date>(const DB& db, const LIST& params, ordinal_date* in)
{
    size_t base = GenericFill(db, params, static_cast<date*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to ordinal_date"); }    do { // convert the 'day_component' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->day_component, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to ordinal_date to be a `day_in_year_number`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<projection_directed_callout>(const DB& db, const LIST& params, projection_directed_callout* in)
{
    size_t base = GenericFill(db, params, static_cast<draughting_callout*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to projection_directed_callout"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<ordinate_dimension>(const DB& db, const LIST& params, ordinate_dimension* in)
{
    size_t base = GenericFill(db, params, static_cast<projection_directed_callout*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to ordinate_dimension"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<organizational_address>(const DB& db, const LIST& params, organizational_address* in)
{
    size_t base = GenericFill(db, params, static_cast<address*>(in));
    if (params.GetSize() < 14) { throw STEP::TypeError("expected 14 arguments to organizational_address"); }    do { // convert the 'organizations' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->organizations, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 12 to organizational_address to be a `SET [1:?] OF organization`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 13 to organizational_address to be a `text`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<oriented_closed_shell>(const DB& db, const LIST& params, oriented_closed_shell* in)
{
    size_t base = GenericFill(db, params, static_cast<closed_shell*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to oriented_closed_shell"); }    do { // convert the 'closed_shell_element' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->closed_shell_element, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to oriented_closed_shell to be a `closed_shell`")); }
    } while (0);
    do { // convert the 'orientation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->orientation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to oriented_closed_shell to be a `BOOLEAN`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<oriented_edge>(const DB& db, const LIST& params, oriented_edge* in)
{
    size_t base = GenericFill(db, params, static_cast<edge*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to oriented_edge"); }    do { // convert the 'edge_element' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->edge_element, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to oriented_edge to be a `edge`")); }
    } while (0);
    do { // convert the 'orientation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->orientation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to oriented_edge to be a `BOOLEAN`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<oriented_face>(const DB& db, const LIST& params, oriented_face* in)
{
    size_t base = GenericFill(db, params, static_cast<face*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to oriented_face"); }    do { // convert the 'face_element' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->face_element, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to oriented_face to be a `face`")); }
    } while (0);
    do { // convert the 'orientation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->orientation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to oriented_face to be a `BOOLEAN`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<oriented_open_shell>(const DB& db, const LIST& params, oriented_open_shell* in)
{
    size_t base = GenericFill(db, params, static_cast<open_shell*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to oriented_open_shell"); }    do { // convert the 'open_shell_element' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->open_shell_element, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to oriented_open_shell to be a `open_shell`")); }
    } while (0);
    do { // convert the 'orientation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->orientation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to oriented_open_shell to be a `BOOLEAN`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<path>(const DB& db, const LIST& params, path* in)
{
    size_t base = GenericFill(db, params, static_cast<topological_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to path"); }    do { // convert the 'edge_list' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::path, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->edge_list, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to path to be a `LIST [1:?] OF oriented_edge`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<oriented_path>(const DB& db, const LIST& params, oriented_path* in)
{
    size_t base = GenericFill(db, params, static_cast<path*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to oriented_path"); }    do { // convert the 'path_element' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->path_element, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to oriented_path to be a `path`")); }
    } while (0);
    do { // convert the 'orientation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->orientation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to oriented_path to be a `BOOLEAN`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<oriented_surface>(const DB& db, const LIST& params, oriented_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<surface*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to oriented_surface"); }    do { // convert the 'orientation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->orientation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to oriented_surface to be a `BOOLEAN`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<outer_boundary_curve>(const DB& db, const LIST& params, outer_boundary_curve* in)
{
    size_t base = GenericFill(db, params, static_cast<boundary_curve*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to outer_boundary_curve"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<package_product_concept_feature>(const DB& db, const LIST& params, package_product_concept_feature* in)
{
    size_t base = GenericFill(db, params, static_cast<product_concept_feature*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to package_product_concept_feature"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<parabola>(const DB& db, const LIST& params, parabola* in)
{
    size_t base = GenericFill(db, params, static_cast<conic*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to parabola"); }    do { // convert the 'focal_dist' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->focal_dist, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to parabola to be a `length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<parallel_offset>(const DB& db, const LIST& params, parallel_offset* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_shape_aspect*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to parallel_offset"); }    do { // convert the 'offset' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->offset, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to parallel_offset to be a `measure_with_unit`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<parallelism_tolerance>(const DB& db, const LIST& params, parallelism_tolerance* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_tolerance_with_datum_reference*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to parallelism_tolerance"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<parametric_representation_context>(const DB& db, const LIST& params, parametric_representation_context* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_context*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to parametric_representation_context"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<partial_document_with_structured_text_representation_assignment>(const DB& db, const LIST& params, partial_document_with_structured_text_representation_assignment* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<pcurve>(const DB& db, const LIST& params, pcurve* in)
{
    size_t base = GenericFill(db, params, static_cast<curve*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to pcurve"); }    do { // convert the 'basis_surface' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->basis_surface, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to pcurve to be a `surface`")); }
    } while (0);
    do { // convert the 'reference_to_curve' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->reference_to_curve, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to pcurve to be a `definitional_representation`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<percentage_laminate_definition>(const DB& db, const LIST& params, percentage_laminate_definition* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to percentage_laminate_definition"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<zone_structural_makeup>(const DB& db, const LIST& params, zone_structural_makeup* in)
{
    size_t base = GenericFill(db, params, static_cast<laminate_table*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to zone_structural_makeup"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<percentage_laminate_table>(const DB& db, const LIST& params, percentage_laminate_table* in)
{
    size_t base = GenericFill(db, params, static_cast<zone_structural_makeup*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to percentage_laminate_table"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<percentage_ply_definition>(const DB& db, const LIST& params, percentage_ply_definition* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to percentage_ply_definition"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<perpendicular_to>(const DB& db, const LIST& params, perpendicular_to* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_shape_aspect*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to perpendicular_to"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<perpendicularity_tolerance>(const DB& db, const LIST& params, perpendicularity_tolerance* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_tolerance_with_datum_reference*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to perpendicularity_tolerance"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<person_and_organization_address>(const DB& db, const LIST& params, person_and_organization_address* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<personal_address>(const DB& db, const LIST& params, personal_address* in)
{
    size_t base = GenericFill(db, params, static_cast<address*>(in));
    if (params.GetSize() < 14) { throw STEP::TypeError("expected 14 arguments to personal_address"); }    do { // convert the 'people' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->people, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 12 to personal_address to be a `SET [1:?] OF person`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 13 to personal_address to be a `text`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<physical_breakdown_context>(const DB& db, const LIST& params, physical_breakdown_context* in)
{
    size_t base = GenericFill(db, params, static_cast<breakdown_context*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to physical_breakdown_context"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<physical_element_usage>(const DB& db, const LIST& params, physical_element_usage* in)
{
    size_t base = GenericFill(db, params, static_cast<breakdown_element_usage*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to physical_element_usage"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<presentation_view>(const DB& db, const LIST& params, presentation_view* in)
{
    size_t base = GenericFill(db, params, static_cast<presentation_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to presentation_view"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<picture_representation>(const DB& db, const LIST& params, picture_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<presentation_view*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to picture_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<placed_datum_target_feature>(const DB& db, const LIST& params, placed_datum_target_feature* in)
{
    size_t base = GenericFill(db, params, static_cast<datum_target*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to placed_datum_target_feature"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<placed_feature>(const DB& db, const LIST& params, placed_feature* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_aspect*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to placed_feature"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<planar_extent>(const DB& db, const LIST& params, planar_extent* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to planar_extent"); }    do { // convert the 'size_in_x' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::planar_extent, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->size_in_x, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to planar_extent to be a `length_measure`")); }
    } while (0);
    do { // convert the 'size_in_y' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::planar_extent, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->size_in_y, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to planar_extent to be a `length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<planar_box>(const DB& db, const LIST& params, planar_box* in)
{
    size_t base = GenericFill(db, params, static_cast<planar_extent*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to planar_box"); }    do { // convert the 'placement' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->placement, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to planar_box to be a `axis2_placement`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<plane>(const DB& db, const LIST& params, plane* in)
{
    size_t base = GenericFill(db, params, static_cast<elementary_surface*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to plane"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<plane_angle_measure_with_unit>(const DB& db, const LIST& params, plane_angle_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to plane_angle_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<plane_angle_unit>(const DB& db, const LIST& params, plane_angle_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<named_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to plane_angle_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<ply_laminate_definition>(const DB& db, const LIST& params, ply_laminate_definition* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to ply_laminate_definition"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<ply_laminate_sequence_definition>(const DB& db, const LIST& params, ply_laminate_sequence_definition* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to ply_laminate_sequence_definition"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<ply_laminate_table>(const DB& db, const LIST& params, ply_laminate_table* in)
{
    size_t base = GenericFill(db, params, static_cast<part_laminate_table*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to ply_laminate_table"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<point_and_vector>(const DB& db, const LIST& params, point_and_vector* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<point_on_curve>(const DB& db, const LIST& params, point_on_curve* in)
{
    size_t base = GenericFill(db, params, static_cast<point*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to point_on_curve"); }    do { // convert the 'basis_curve' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->basis_curve, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to point_on_curve to be a `curve`")); }
    } while (0);
    do { // convert the 'point_parameter' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->point_parameter, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to point_on_curve to be a `parameter_value`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<point_on_surface>(const DB& db, const LIST& params, point_on_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<point*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to point_on_surface"); }    do { // convert the 'basis_surface' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->basis_surface, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to point_on_surface to be a `surface`")); }
    } while (0);
    do { // convert the 'point_parameter_u' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->point_parameter_u, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to point_on_surface to be a `parameter_value`")); }
    } while (0);
    do { // convert the 'point_parameter_v' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->point_parameter_v, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to point_on_surface to be a `parameter_value`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<point_path>(const DB& db, const LIST& params, point_path* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<point_replica>(const DB& db, const LIST& params, point_replica* in)
{
    size_t base = GenericFill(db, params, static_cast<point*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to point_replica"); }    do { // convert the 'parent_pt' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->parent_pt, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to point_replica to be a `point`")); }
    } while (0);
    do { // convert the 'transformation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->transformation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to point_replica to be a `cartesian_transformation_operator`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<point_style>(const DB& db, const LIST& params, point_style* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to point_style"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to point_style to be a `label`")); }
    } while (0);
    do { // convert the 'marker' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->marker, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to point_style to be a `marker_select`")); }
    } while (0);
    do { // convert the 'marker_size' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->marker_size, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to point_style to be a `size_select`")); }
    } while (0);
    do { // convert the 'marker_colour' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->marker_colour, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to point_style to be a `colour`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<polar_complex_number_literal>(const DB& db, const LIST& params, polar_complex_number_literal* in)
{
    size_t base = GenericFill(db, params, static_cast<generic_literal*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to polar_complex_number_literal"); }    do { // convert the 'radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::polar_complex_number_literal, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to polar_complex_number_literal to be a `REAL`")); }
    } while (0);
    do { // convert the 'angle' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::polar_complex_number_literal, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->angle, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to polar_complex_number_literal to be a `REAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<poly_loop>(const DB& db, const LIST& params, poly_loop* in)
{
    size_t base = 0;
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to poly_loop"); }    do { // convert the 'polygon' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->polygon, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to poly_loop to be a `LIST [3:?] OF cartesian_point`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<polyline>(const DB& db, const LIST& params, polyline* in)
{
    size_t base = GenericFill(db, params, static_cast<bounded_curve*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to polyline"); }    do { // convert the 'points' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->points, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to polyline to be a `LIST [2:?] OF cartesian_point`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<position_tolerance>(const DB& db, const LIST& params, position_tolerance* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_tolerance*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to position_tolerance"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<positioned_sketch>(const DB& db, const LIST& params, positioned_sketch* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to positioned_sketch"); }    do { // convert the 'sketch_basis' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->sketch_basis, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to positioned_sketch to be a `sketch_basis_select`")); }
    } while (0);
    do { // convert the 'auxiliary_elements' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->auxiliary_elements, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to positioned_sketch to be a `SET [0:?] OF auxiliary_geometric_representation_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<power_measure_with_unit>(const DB& db, const LIST& params, power_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to power_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<power_unit>(const DB& db, const LIST& params, power_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to power_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<pre_defined_symbol>(const DB& db, const LIST& params, pre_defined_symbol* in)
{
    size_t base = GenericFill(db, params, static_cast<pre_defined_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to pre_defined_symbol"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<pre_defined_dimension_symbol>(const DB& db, const LIST& params, pre_defined_dimension_symbol* in)
{
    size_t base = GenericFill(db, params, static_cast<pre_defined_symbol*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to pre_defined_dimension_symbol"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<pre_defined_geometrical_tolerance_symbol>(const DB& db, const LIST& params, pre_defined_geometrical_tolerance_symbol* in)
{
    size_t base = GenericFill(db, params, static_cast<pre_defined_symbol*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to pre_defined_geometrical_tolerance_symbol"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<pre_defined_marker>(const DB& db, const LIST& params, pre_defined_marker* in)
{
    size_t base = GenericFill(db, params, static_cast<pre_defined_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to pre_defined_marker"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<pre_defined_point_marker_symbol>(const DB& db, const LIST& params, pre_defined_point_marker_symbol* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<pre_defined_surface_condition_symbol>(const DB& db, const LIST& params, pre_defined_surface_condition_symbol* in)
{
    size_t base = GenericFill(db, params, static_cast<pre_defined_symbol*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to pre_defined_surface_condition_symbol"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<pre_defined_surface_side_style>(const DB& db, const LIST& params, pre_defined_surface_side_style* in)
{
    size_t base = GenericFill(db, params, static_cast<pre_defined_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to pre_defined_surface_side_style"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<pre_defined_terminator_symbol>(const DB& db, const LIST& params, pre_defined_terminator_symbol* in)
{
    size_t base = GenericFill(db, params, static_cast<pre_defined_symbol*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to pre_defined_terminator_symbol"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<pre_defined_tile>(const DB& db, const LIST& params, pre_defined_tile* in)
{
    size_t base = GenericFill(db, params, static_cast<pre_defined_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to pre_defined_tile"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<predefined_picture_representation_item>(const DB& db, const LIST& params, predefined_picture_representation_item* in)
{
    size_t base = GenericFill(db, params, static_cast<picture_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to predefined_picture_representation_item"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<presentation_style_assignment>(const DB& db, const LIST& params, presentation_style_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to presentation_style_assignment"); }    do { // convert the 'styles' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::presentation_style_assignment, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->styles, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to presentation_style_assignment to be a `SET [1:?] OF presentation_style_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<presentation_style_by_context>(const DB& db, const LIST& params, presentation_style_by_context* in)
{
    size_t base = GenericFill(db, params, static_cast<presentation_style_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to presentation_style_by_context"); }    do { // convert the 'style_context' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->style_context, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to presentation_style_by_context to be a `style_context_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<pressure_measure_with_unit>(const DB& db, const LIST& params, pressure_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to pressure_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<pressure_unit>(const DB& db, const LIST& params, pressure_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to pressure_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<procedural_representation>(const DB& db, const LIST& params, procedural_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to procedural_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<procedural_representation_sequence>(const DB& db, const LIST& params, procedural_representation_sequence* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_item*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to procedural_representation_sequence"); }    do { // convert the 'elements' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->elements, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to procedural_representation_sequence to be a `LIST [1:?] OF representation_item`")); }
    } while (0);
    do { // convert the 'suppressed_items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->suppressed_items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to procedural_representation_sequence to be a `SET [0:?] OF representation_item`")); }
    } while (0);
    do { // convert the 'rationale' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->rationale, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to procedural_representation_sequence to be a `text`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<procedural_shape_representation>(const DB& db, const LIST& params, procedural_shape_representation* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<procedural_shape_representation_sequence>(const DB& db, const LIST& params, procedural_shape_representation_sequence* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_category>(const DB& db, const LIST& params, product_category* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to product_category"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_category, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to product_category to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_category, 2>::aux_is_derived[1] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to product_category to be a `text`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_class>(const DB& db, const LIST& params, product_class* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_concept_context>(const DB& db, const LIST& params, product_concept_context* in)
{
    size_t base = GenericFill(db, params, static_cast<application_context_element*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to product_concept_context"); }    do { // convert the 'market_segment_type' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->market_segment_type, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to product_concept_context to be a `label`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_concept_feature_category_usage>(const DB& db, const LIST& params, product_concept_feature_category_usage* in)
{
    size_t base = GenericFill(db, params, static_cast<group_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to product_concept_feature_category_usage"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to product_concept_feature_category_usage to be a `SET [1:?] OF category_usage_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_definition_element_relationship>(const DB& db, const LIST& params, product_definition_element_relationship* in)
{
    size_t base = GenericFill(db, params, static_cast<group*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to product_definition_element_relationship"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_definition_formation>(const DB& db, const LIST& params, product_definition_formation* in)
{
    size_t base = 0;
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to product_definition_formation"); }    do { // convert the 'id' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_definition_formation, 3>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->id, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to product_definition_formation to be a `identifier`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_definition_formation, 3>::aux_is_derived[1] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to product_definition_formation to be a `text`")); }
    } while (0);
    do { // convert the 'of_product' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_definition_formation, 3>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->of_product, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to product_definition_formation to be a `product`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_definition_formation_with_specified_source>(const DB& db, const LIST& params, product_definition_formation_with_specified_source* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition_formation*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to product_definition_formation_with_specified_source"); }    do { // convert the 'make_or_buy' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->make_or_buy, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to product_definition_formation_with_specified_source to be a `source`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_definition_group_assignment>(const DB& db, const LIST& params, product_definition_group_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<group_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to product_definition_group_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to product_definition_group_assignment to be a `SET [1:1] OF product_definition_or_product_definition_relationship`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_definition_shape>(const DB& db, const LIST& params, product_definition_shape* in)
{
    size_t base = GenericFill(db, params, static_cast<property_definition*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to product_definition_shape"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_definition_with_associated_documents>(const DB& db, const LIST& params, product_definition_with_associated_documents* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to product_definition_with_associated_documents"); }    do { // convert the 'documentation_ids' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->documentation_ids, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to product_definition_with_associated_documents to be a `SET [1:?] OF document`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_identification>(const DB& db, const LIST& params, product_identification* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_material_composition_relationship>(const DB& db, const LIST& params, product_material_composition_relationship* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition_relationship*>(in));
    if (params.GetSize() < 9) { throw STEP::TypeError("expected 9 arguments to product_material_composition_relationship"); }    do { // convert the 'class' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->class_, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to product_material_composition_relationship to be a `label`")); }
    } while (0);
    do { // convert the 'constituent_amount' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->constituent_amount, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to product_material_composition_relationship to be a `SET [1:?] OF characterized_product_composition_value`")); }
    } while (0);
    do { // convert the 'composition_basis' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->composition_basis, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to product_material_composition_relationship to be a `label`")); }
    } while (0);
    do { // convert the 'determination_method' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->determination_method, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 8 to product_material_composition_relationship to be a `text`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_related_product_category>(const DB& db, const LIST& params, product_related_product_category* in)
{
    size_t base = GenericFill(db, params, static_cast<product_category*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to product_related_product_category"); }    do { // convert the 'products' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->products, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to product_related_product_category to be a `SET [1:?] OF product`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_specification>(const DB& db, const LIST& params, product_specification* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<tolerance_zone_definition>(const DB& db, const LIST& params, tolerance_zone_definition* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to tolerance_zone_definition"); }    do { // convert the 'zone' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::tolerance_zone_definition, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->zone, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to tolerance_zone_definition to be a `tolerance_zone`")); }
    } while (0);
    do { // convert the 'boundaries' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::tolerance_zone_definition, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->boundaries, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to tolerance_zone_definition to be a `SET [1:?] OF shape_aspect`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<projected_zone_definition>(const DB& db, const LIST& params, projected_zone_definition* in)
{
    size_t base = GenericFill(db, params, static_cast<tolerance_zone_definition*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to projected_zone_definition"); }    do { // convert the 'projection_end' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->projection_end, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to projected_zone_definition to be a `shape_aspect`")); }
    } while (0);
    do { // convert the 'projected_length' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->projected_length, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to projected_zone_definition to be a `measure_with_unit`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<projection_curve>(const DB& db, const LIST& params, projection_curve* in)
{
    size_t base = GenericFill(db, params, static_cast<annotation_curve_occurrence*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to projection_curve"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<promissory_usage_occurrence>(const DB& db, const LIST& params, promissory_usage_occurrence* in)
{
    size_t base = GenericFill(db, params, static_cast<assembly_component_usage*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to promissory_usage_occurrence"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<qualified_representation_item>(const DB& db, const LIST& params, qualified_representation_item* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to qualified_representation_item"); }    do { // convert the 'qualifiers' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->qualifiers, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to qualified_representation_item to be a `SET [1:?] OF value_qualifier`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<qualitative_uncertainty>(const DB& db, const LIST& params, qualitative_uncertainty* in)
{
    size_t base = GenericFill(db, params, static_cast<uncertainty_qualifier*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to qualitative_uncertainty"); }    do { // convert the 'uncertainty_value' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->uncertainty_value, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to qualitative_uncertainty to be a `text`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<quantified_assembly_component_usage>(const DB& db, const LIST& params, quantified_assembly_component_usage* in)
{
    size_t base = GenericFill(db, params, static_cast<assembly_component_usage*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to quantified_assembly_component_usage"); }    do { // convert the 'quantity' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->quantity, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to quantified_assembly_component_usage to be a `measure_with_unit`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<quasi_uniform_curve>(const DB& db, const LIST& params, quasi_uniform_curve* in)
{
    size_t base = GenericFill(db, params, static_cast<b_spline_curve*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to quasi_uniform_curve"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<quasi_uniform_surface>(const DB& db, const LIST& params, quasi_uniform_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<b_spline_surface*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to quasi_uniform_surface"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<radioactivity_measure_with_unit>(const DB& db, const LIST& params, radioactivity_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to radioactivity_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<radioactivity_unit>(const DB& db, const LIST& params, radioactivity_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to radioactivity_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<radius_dimension>(const DB& db, const LIST& params, radius_dimension* in)
{
    size_t base = GenericFill(db, params, static_cast<dimension_curve_directed_callout*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to radius_dimension"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<range_characteristic>(const DB& db, const LIST& params, range_characteristic* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<ratio_unit>(const DB& db, const LIST& params, ratio_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<named_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to ratio_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<rational_b_spline_curve>(const DB& db, const LIST& params, rational_b_spline_curve* in)
{
    size_t base = GenericFill(db, params, static_cast<b_spline_curve*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to rational_b_spline_curve"); }    do { // convert the 'weights_data' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->weights_data, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to rational_b_spline_curve to be a `LIST [2:?] OF REAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<rational_b_spline_surface>(const DB& db, const LIST& params, rational_b_spline_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<b_spline_surface*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to rational_b_spline_surface"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<rational_representation_item>(const DB& db, const LIST& params, rational_representation_item* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<real_literal>(const DB& db, const LIST& params, real_literal* in)
{
    size_t base = GenericFill(db, params, static_cast<literal_number*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to real_literal"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<real_representation_item>(const DB& db, const LIST& params, real_representation_item* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<rectangular_composite_surface>(const DB& db, const LIST& params, rectangular_composite_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<bounded_surface*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to rectangular_composite_surface"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<rectangular_trimmed_surface>(const DB& db, const LIST& params, rectangular_trimmed_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<bounded_surface*>(in));
    if (params.GetSize() < 8) { throw STEP::TypeError("expected 8 arguments to rectangular_trimmed_surface"); }    do { // convert the 'basis_surface' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->basis_surface, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to rectangular_trimmed_surface to be a `surface`")); }
    } while (0);
    do { // convert the 'u1' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->u1, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to rectangular_trimmed_surface to be a `parameter_value`")); }
    } while (0);
    do { // convert the 'u2' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->u2, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to rectangular_trimmed_surface to be a `parameter_value`")); }
    } while (0);
    do { // convert the 'v1' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->v1, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to rectangular_trimmed_surface to be a `parameter_value`")); }
    } while (0);
    do { // convert the 'v2' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->v2, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to rectangular_trimmed_surface to be a `parameter_value`")); }
    } while (0);
    do { // convert the 'usense' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->usense, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to rectangular_trimmed_surface to be a `BOOLEAN`")); }
    } while (0);
    do { // convert the 'vsense' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->vsense, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to rectangular_trimmed_surface to be a `BOOLEAN`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<referenced_modified_datum>(const DB& db, const LIST& params, referenced_modified_datum* in)
{
    size_t base = GenericFill(db, params, static_cast<datum_reference*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to referenced_modified_datum"); }    do { // convert the 'modifier' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->modifier, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to referenced_modified_datum to be a `limit_condition`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<relative_event_occurrence>(const DB& db, const LIST& params, relative_event_occurrence* in)
{
    size_t base = GenericFill(db, params, static_cast<event_occurrence*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to relative_event_occurrence"); }    do { // convert the 'base_event' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->base_event, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to relative_event_occurrence to be a `event_occurrence`")); }
    } while (0);
    do { // convert the 'offset' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->offset, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to relative_event_occurrence to be a `time_measure_with_unit`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<rep_item_group>(const DB& db, const LIST& params, rep_item_group* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<reparametrised_composite_curve_segment>(const DB& db, const LIST& params, reparametrised_composite_curve_segment* in)
{
    size_t base = GenericFill(db, params, static_cast<composite_curve_segment*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to reparametrised_composite_curve_segment"); }    do { // convert the 'param_length' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->param_length, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to reparametrised_composite_curve_segment to be a `parameter_value`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<representation_relationship_with_transformation>(const DB& db, const LIST& params, representation_relationship_with_transformation* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_relationship*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to representation_relationship_with_transformation"); }    do { // convert the 'transformation_operator' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->transformation_operator, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to representation_relationship_with_transformation to be a `transformation`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<requirement_assigned_object>(const DB& db, const LIST& params, requirement_assigned_object* in)
{
    size_t base = GenericFill(db, params, static_cast<group_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to requirement_assigned_object"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to requirement_assigned_object to be a `SET [1:1] OF requirement_assigned_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<requirement_assignment>(const DB& db, const LIST& params, requirement_assignment* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<requirement_source>(const DB& db, const LIST& params, requirement_source* in)
{
    size_t base = GenericFill(db, params, static_cast<group*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to requirement_source"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<requirement_view_definition_relationship>(const DB& db, const LIST& params, requirement_view_definition_relationship* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition_relationship*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to requirement_view_definition_relationship"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<resistance_measure_with_unit>(const DB& db, const LIST& params, resistance_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to resistance_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<resistance_unit>(const DB& db, const LIST& params, resistance_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to resistance_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<revolved_area_solid>(const DB& db, const LIST& params, revolved_area_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<swept_area_solid*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to revolved_area_solid"); }    do { // convert the 'axis' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->axis, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to revolved_area_solid to be a `axis1_placement`")); }
    } while (0);
    do { // convert the 'angle' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->angle, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to revolved_area_solid to be a `plane_angle_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<revolved_face_solid>(const DB& db, const LIST& params, revolved_face_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<swept_face_solid*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to revolved_face_solid"); }    do { // convert the 'axis' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::revolved_face_solid, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->axis, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to revolved_face_solid to be a `axis1_placement`")); }
    } while (0);
    do { // convert the 'angle' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::revolved_face_solid, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->angle, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to revolved_face_solid to be a `plane_angle_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<revolved_face_solid_with_trim_conditions>(const DB& db, const LIST& params, revolved_face_solid_with_trim_conditions* in)
{
    size_t base = GenericFill(db, params, static_cast<revolved_face_solid*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to revolved_face_solid_with_trim_conditions"); }    do { // convert the 'first_trim_condition' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->first_trim_condition, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to revolved_face_solid_with_trim_conditions to be a `trim_condition_select`")); }
    } while (0);
    do { // convert the 'second_trim_condition' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->second_trim_condition, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to revolved_face_solid_with_trim_conditions to be a `trim_condition_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<right_angular_wedge>(const DB& db, const LIST& params, right_angular_wedge* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to right_angular_wedge"); }    do { // convert the 'position' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->position, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to right_angular_wedge to be a `axis2_placement_3d`")); }
    } while (0);
    do { // convert the 'x' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->x, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to right_angular_wedge to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'y' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->y, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to right_angular_wedge to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'z' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->z, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to right_angular_wedge to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'ltx' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->ltx, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to right_angular_wedge to be a `length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<right_circular_cone>(const DB& db, const LIST& params, right_circular_cone* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to right_circular_cone"); }    do { // convert the 'position' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->position, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to right_circular_cone to be a `axis1_placement`")); }
    } while (0);
    do { // convert the 'height' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->height, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to right_circular_cone to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to right_circular_cone to be a `length_measure`")); }
    } while (0);
    do { // convert the 'semi_angle' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->semi_angle, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to right_circular_cone to be a `plane_angle_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<right_circular_cylinder>(const DB& db, const LIST& params, right_circular_cylinder* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to right_circular_cylinder"); }    do { // convert the 'position' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->position, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to right_circular_cylinder to be a `axis1_placement`")); }
    } while (0);
    do { // convert the 'height' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->height, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to right_circular_cylinder to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to right_circular_cylinder to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<right_to_usage_association>(const DB& db, const LIST& params, right_to_usage_association* in)
{
    size_t base = GenericFill(db, params, static_cast<action_method_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to right_to_usage_association"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<roundness_tolerance>(const DB& db, const LIST& params, roundness_tolerance* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_tolerance*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to roundness_tolerance"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<row_representation_item>(const DB& db, const LIST& params, row_representation_item* in)
{
    size_t base = GenericFill(db, params, static_cast<compound_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to row_representation_item"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<row_value>(const DB& db, const LIST& params, row_value* in)
{
    size_t base = GenericFill(db, params, static_cast<compound_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to row_value"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<row_variable>(const DB& db, const LIST& params, row_variable* in)
{
    size_t base = GenericFill(db, params, static_cast<abstract_variable*>(in));
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<rule_action>(const DB& db, const LIST& params, rule_action* in)
{
    size_t base = GenericFill(db, params, static_cast<action*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to rule_action"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<rule_condition>(const DB& db, const LIST& params, rule_condition* in)
{
    size_t base = GenericFill(db, params, static_cast<atomic_formula*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to rule_condition"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<rule_set>(const DB& db, const LIST& params, rule_set* in)
{
    size_t base = GenericFill(db, params, static_cast<rule_software_definition*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to rule_set"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<rule_set_group>(const DB& db, const LIST& params, rule_set_group* in)
{
    size_t base = GenericFill(db, params, static_cast<rule_software_definition*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to rule_set_group"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<rule_superseded_assignment>(const DB& db, const LIST& params, rule_superseded_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<action_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to rule_superseded_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to rule_superseded_assignment to be a `SET [1:?] OF rule_superseded_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<rule_supersedence>(const DB& db, const LIST& params, rule_supersedence* in)
{
    size_t base = GenericFill(db, params, static_cast<rule_action*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to rule_supersedence"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_curve_swept_area_solid>(const DB& db, const LIST& params, surface_curve_swept_area_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<swept_area_solid*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to surface_curve_swept_area_solid"); }    do { // convert the 'directrix' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::surface_curve_swept_area_solid, 4>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->directrix, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to surface_curve_swept_area_solid to be a `curve`")); }
    } while (0);
    do { // convert the 'start_param' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::surface_curve_swept_area_solid, 4>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->start_param, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to surface_curve_swept_area_solid to be a `REAL`")); }
    } while (0);
    do { // convert the 'end_param' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::surface_curve_swept_area_solid, 4>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->end_param, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to surface_curve_swept_area_solid to be a `REAL`")); }
    } while (0);
    do { // convert the 'reference_surface' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::surface_curve_swept_area_solid, 4>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->reference_surface, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to surface_curve_swept_area_solid to be a `surface`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<ruled_surface_swept_area_solid>(const DB& db, const LIST& params, ruled_surface_swept_area_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<surface_curve_swept_area_solid*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to ruled_surface_swept_area_solid"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<runout_zone_definition>(const DB& db, const LIST& params, runout_zone_definition* in)
{
    size_t base = GenericFill(db, params, static_cast<tolerance_zone_definition*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to runout_zone_definition"); }    do { // convert the 'orientation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->orientation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to runout_zone_definition to be a `runout_zone_orientation`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<runout_zone_orientation>(const DB& db, const LIST& params, runout_zone_orientation* in)
{
    size_t base = 0;
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to runout_zone_orientation"); }    do { // convert the 'angle' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::runout_zone_orientation, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->angle, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to runout_zone_orientation to be a `measure_with_unit`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<runout_zone_orientation_reference_direction>(const DB& db, const LIST& params, runout_zone_orientation_reference_direction* in)
{
    size_t base = GenericFill(db, params, static_cast<runout_zone_orientation*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to runout_zone_orientation_reference_direction"); }    do { // convert the 'orientation_defining_relationship' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->orientation_defining_relationship, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to runout_zone_orientation_reference_direction to be a `shape_aspect_relationship`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<satisfied_requirement>(const DB& db, const LIST& params, satisfied_requirement* in)
{
    size_t base = GenericFill(db, params, static_cast<group_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to satisfied_requirement"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to satisfied_requirement to be a `SET [1:1] OF product_definition`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<satisfies_requirement>(const DB& db, const LIST& params, satisfies_requirement* in)
{
    size_t base = GenericFill(db, params, static_cast<group*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to satisfies_requirement"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<satisfying_item>(const DB& db, const LIST& params, satisfying_item* in)
{
    size_t base = GenericFill(db, params, static_cast<group_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to satisfying_item"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to satisfying_item to be a `SET [1:1] OF requirement_satisfaction_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<scalar_variable>(const DB& db, const LIST& params, scalar_variable* in)
{
    size_t base = GenericFill(db, params, static_cast<abstract_variable*>(in));
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<scattering_parameter>(const DB& db, const LIST& params, scattering_parameter* in)
{
    size_t base = GenericFill(db, params, static_cast<polar_complex_number_literal*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to scattering_parameter"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<sculptured_solid>(const DB& db, const LIST& params, sculptured_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<modified_solid*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to sculptured_solid"); }    do { // convert the 'sculpturing_element' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->sculpturing_element, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to sculptured_solid to be a `generalized_surface_select`")); }
    } while (0);
    do { // convert the 'positive_side' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->positive_side, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to sculptured_solid to be a `BOOLEAN`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<seam_curve>(const DB& db, const LIST& params, seam_curve* in)
{
    size_t base = GenericFill(db, params, static_cast<surface_curve*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to seam_curve"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<serial_numbered_effectivity>(const DB& db, const LIST& params, serial_numbered_effectivity* in)
{
    size_t base = GenericFill(db, params, static_cast<effectivity*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to serial_numbered_effectivity"); }    do { // convert the 'effectivity_start_id' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->effectivity_start_id, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to serial_numbered_effectivity to be a `identifier`")); }
    } while (0);
    do { // convert the 'effectivity_end_id' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->effectivity_end_id, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to serial_numbered_effectivity to be a `identifier`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<shape_aspect_associativity>(const DB& db, const LIST& params, shape_aspect_associativity* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_aspect_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to shape_aspect_associativity"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<shape_aspect_deriving_relationship>(const DB& db, const LIST& params, shape_aspect_deriving_relationship* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_aspect_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to shape_aspect_deriving_relationship"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<shape_definition_representation>(const DB& db, const LIST& params, shape_definition_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<property_definition_representation*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to shape_definition_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<shape_dimension_representation>(const DB& db, const LIST& params, shape_dimension_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to shape_dimension_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<shape_feature_definition>(const DB& db, const LIST& params, shape_feature_definition* in)
{
    size_t base = GenericFill(db, params, static_cast<characterized_object*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to shape_feature_definition"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<shape_representation_with_parameters>(const DB& db, const LIST& params, shape_representation_with_parameters* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to shape_representation_with_parameters"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<shell_based_surface_model>(const DB& db, const LIST& params, shell_based_surface_model* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to shell_based_surface_model"); }    do { // convert the 'sbsm_boundary' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->sbsm_boundary, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to shell_based_surface_model to be a `SET [1:?] OF shell`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<shell_based_wireframe_model>(const DB& db, const LIST& params, shell_based_wireframe_model* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to shell_based_wireframe_model"); }    do { // convert the 'sbwm_boundary' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->sbwm_boundary, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to shell_based_wireframe_model to be a `SET [1:?] OF shell`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<shell_based_wireframe_shape_representation>(const DB& db, const LIST& params, shell_based_wireframe_shape_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to shell_based_wireframe_shape_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<si_absorbed_dose_unit>(const DB& db, const LIST& params, si_absorbed_dose_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<si_capacitance_unit>(const DB& db, const LIST& params, si_capacitance_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<si_conductance_unit>(const DB& db, const LIST& params, si_conductance_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<si_dose_equivalent_unit>(const DB& db, const LIST& params, si_dose_equivalent_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<si_electric_charge_unit>(const DB& db, const LIST& params, si_electric_charge_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<si_electric_potential_unit>(const DB& db, const LIST& params, si_electric_potential_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<si_energy_unit>(const DB& db, const LIST& params, si_energy_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<si_force_unit>(const DB& db, const LIST& params, si_force_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<si_frequency_unit>(const DB& db, const LIST& params, si_frequency_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<si_illuminance_unit>(const DB& db, const LIST& params, si_illuminance_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<si_inductance_unit>(const DB& db, const LIST& params, si_inductance_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<si_magnetic_flux_density_unit>(const DB& db, const LIST& params, si_magnetic_flux_density_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<si_magnetic_flux_unit>(const DB& db, const LIST& params, si_magnetic_flux_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<si_power_unit>(const DB& db, const LIST& params, si_power_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<si_pressure_unit>(const DB& db, const LIST& params, si_pressure_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<si_radioactivity_unit>(const DB& db, const LIST& params, si_radioactivity_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<si_resistance_unit>(const DB& db, const LIST& params, si_resistance_unit* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<si_unit>(const DB& db, const LIST& params, si_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<named_unit*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to si_unit"); }    do { // convert the 'prefix' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->prefix, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to si_unit to be a `si_prefix`")); }
    } while (0);
    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to si_unit to be a `si_unit_name`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<simple_boolean_expression>(const DB& db, const LIST& params, simple_boolean_expression* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<simple_numeric_expression>(const DB& db, const LIST& params, simple_numeric_expression* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<slash_expression>(const DB& db, const LIST& params, slash_expression* in)
{
    size_t base = GenericFill(db, params, static_cast<binary_numeric_expression*>(in));
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<smeared_material_definition>(const DB& db, const LIST& params, smeared_material_definition* in)
{
    size_t base = GenericFill(db, params, static_cast<zone_structural_makeup*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to smeared_material_definition"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_angle_measure_with_unit>(const DB& db, const LIST& params, solid_angle_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to solid_angle_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_angle_unit>(const DB& db, const LIST& params, solid_angle_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<named_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to solid_angle_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_curve_font>(const DB& db, const LIST& params, solid_curve_font* in)
{
    size_t base = GenericFill(db, params, static_cast<pre_defined_curve_font*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to solid_curve_font"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_replica>(const DB& db, const LIST& params, solid_replica* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_model*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to solid_replica"); }    do { // convert the 'parent_solid' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->parent_solid, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to solid_replica to be a `solid_model`")); }
    } while (0);
    do { // convert the 'transformation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->transformation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to solid_replica to be a `cartesian_transformation_operator_3d`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_chamfered_edges>(const DB& db, const LIST& params, solid_with_chamfered_edges* in)
{
    size_t base = GenericFill(db, params, static_cast<edge_blended_solid*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to solid_with_chamfered_edges"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_angle_based_chamfer>(const DB& db, const LIST& params, solid_with_angle_based_chamfer* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_chamfered_edges*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to solid_with_angle_based_chamfer"); }    do { // convert the 'offset_distance' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->offset_distance, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to solid_with_angle_based_chamfer to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'left_offset' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->left_offset, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to solid_with_angle_based_chamfer to be a `BOOLEAN`")); }
    } while (0);
    do { // convert the 'offset_angle' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->offset_angle, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to solid_with_angle_based_chamfer to be a `positive_plane_angle_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_shape_element_pattern>(const DB& db, const LIST& params, solid_with_shape_element_pattern* in)
{
    size_t base = GenericFill(db, params, static_cast<modified_solid_with_placed_configuration*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to solid_with_shape_element_pattern"); }    do { // convert the 'replicated_element' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::solid_with_shape_element_pattern, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->replicated_element, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to solid_with_shape_element_pattern to be a `modified_solid_with_placed_configuration`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_circular_pattern>(const DB& db, const LIST& params, solid_with_circular_pattern* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_shape_element_pattern*>(in));
    if (params.GetSize() < 9) { throw STEP::TypeError("expected 9 arguments to solid_with_circular_pattern"); }    do { // convert the 'replicate_count' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::solid_with_circular_pattern, 4>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->replicate_count, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to solid_with_circular_pattern to be a `positive_integer`")); }
    } while (0);
    do { // convert the 'angular_spacing' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::solid_with_circular_pattern, 4>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->angular_spacing, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to solid_with_circular_pattern to be a `plane_angle_measure`")); }
    } while (0);
    do { // convert the 'radial_alignment' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::solid_with_circular_pattern, 4>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->radial_alignment, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to solid_with_circular_pattern to be a `BOOLEAN`")); }
    } while (0);
    do { // convert the 'reference_point' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::solid_with_circular_pattern, 4>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->reference_point, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 8 to solid_with_circular_pattern to be a `point`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_depression>(const DB& db, const LIST& params, solid_with_depression* in)
{
    size_t base = GenericFill(db, params, static_cast<modified_solid_with_placed_configuration*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to solid_with_depression"); }    do { // convert the 'depth' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::solid_with_depression, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->depth, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to solid_with_depression to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_pocket>(const DB& db, const LIST& params, solid_with_pocket* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_depression*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to solid_with_pocket"); }    do { // convert the 'floor_blend_radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::solid_with_pocket, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->floor_blend_radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to solid_with_pocket to be a `non_negative_length_measure`")); }
    } while (0);
    do { // convert the 'draft_angle' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::solid_with_pocket, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->draft_angle, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to solid_with_pocket to be a `plane_angle_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_circular_pocket>(const DB& db, const LIST& params, solid_with_circular_pocket* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_pocket*>(in));
    if (params.GetSize() < 8) { throw STEP::TypeError("expected 8 arguments to solid_with_circular_pocket"); }    do { // convert the 'pocket_radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->pocket_radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to solid_with_circular_pocket to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_protrusion>(const DB& db, const LIST& params, solid_with_protrusion* in)
{
    size_t base = GenericFill(db, params, static_cast<modified_solid_with_placed_configuration*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to solid_with_protrusion"); }    do { // convert the 'protrusion_height' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::solid_with_protrusion, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->protrusion_height, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to solid_with_protrusion to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'protrusion_draft_angle' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::solid_with_protrusion, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->protrusion_draft_angle, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to solid_with_protrusion to be a `plane_angle_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_circular_protrusion>(const DB& db, const LIST& params, solid_with_circular_protrusion* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_protrusion*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to solid_with_circular_protrusion"); }    do { // convert the 'protrusion_radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->protrusion_radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to solid_with_circular_protrusion to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_hole>(const DB& db, const LIST& params, solid_with_hole* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_depression*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to solid_with_hole"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_stepped_round_hole>(const DB& db, const LIST& params, solid_with_stepped_round_hole* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_hole*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to solid_with_stepped_round_hole"); }    do { // convert the 'segments' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::solid_with_stepped_round_hole, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->segments, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to solid_with_stepped_round_hole to be a `positive_integer`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_conical_bottom_round_hole>(const DB& db, const LIST& params, solid_with_conical_bottom_round_hole* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_stepped_round_hole*>(in));
    if (params.GetSize() < 8) { throw STEP::TypeError("expected 8 arguments to solid_with_conical_bottom_round_hole"); }    do { // convert the 'semi_apex_angle' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->semi_apex_angle, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to solid_with_conical_bottom_round_hole to be a `positive_plane_angle_measure`")); }
    } while (0);
    do { // convert the 'tip_radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->tip_radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to solid_with_conical_bottom_round_hole to be a `non_negative_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_constant_radius_edge_blend>(const DB& db, const LIST& params, solid_with_constant_radius_edge_blend* in)
{
    size_t base = GenericFill(db, params, static_cast<edge_blended_solid*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to solid_with_constant_radius_edge_blend"); }    do { // convert the 'radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to solid_with_constant_radius_edge_blend to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_slot>(const DB& db, const LIST& params, solid_with_slot* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_depression*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to solid_with_slot"); }    do { // convert the 'slot_width' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::solid_with_slot, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->slot_width, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to solid_with_slot to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'closed_ends' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::solid_with_slot, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->closed_ends, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to solid_with_slot to be a `LIST [2:2] OF LOGICAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_curved_slot>(const DB& db, const LIST& params, solid_with_curved_slot* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_slot*>(in));
    if (params.GetSize() < 8) { throw STEP::TypeError("expected 8 arguments to solid_with_curved_slot"); }    do { // convert the 'slot_centreline' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->slot_centreline, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to solid_with_curved_slot to be a `bounded_curve`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_double_offset_chamfer>(const DB& db, const LIST& params, solid_with_double_offset_chamfer* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_chamfered_edges*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to solid_with_double_offset_chamfer"); }    do { // convert the 'left_offset_distance' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->left_offset_distance, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to solid_with_double_offset_chamfer to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'right_offset_distance' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->right_offset_distance, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to solid_with_double_offset_chamfer to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_flat_bottom_round_hole>(const DB& db, const LIST& params, solid_with_flat_bottom_round_hole* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_stepped_round_hole*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to solid_with_flat_bottom_round_hole"); }    do { // convert the 'fillet_radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->fillet_radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to solid_with_flat_bottom_round_hole to be a `non_negative_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_general_pocket>(const DB& db, const LIST& params, solid_with_general_pocket* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_pocket*>(in));
    if (params.GetSize() < 9) { throw STEP::TypeError("expected 9 arguments to solid_with_general_pocket"); }    do { // convert the 'profile' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->profile, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to solid_with_general_pocket to be a `positioned_sketch`")); }
    } while (0);
    do { // convert the 'reference_point' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->reference_point, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 8 to solid_with_general_pocket to be a `point`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_general_protrusion>(const DB& db, const LIST& params, solid_with_general_protrusion* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_protrusion*>(in));
    if (params.GetSize() < 8) { throw STEP::TypeError("expected 8 arguments to solid_with_general_protrusion"); }    do { // convert the 'profile' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->profile, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to solid_with_general_protrusion to be a `positioned_sketch`")); }
    } while (0);
    do { // convert the 'reference_point' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->reference_point, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to solid_with_general_protrusion to be a `point`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_groove>(const DB& db, const LIST& params, solid_with_groove* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_depression*>(in));
    if (params.GetSize() < 10) { throw STEP::TypeError("expected 10 arguments to solid_with_groove"); }    do { // convert the 'groove_radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->groove_radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to solid_with_groove to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'groove_width' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->groove_width, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to solid_with_groove to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'draft_angle' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->draft_angle, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to solid_with_groove to be a `plane_angle_measure`")); }
    } while (0);
    do { // convert the 'floor_fillet_radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->floor_fillet_radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 8 to solid_with_groove to be a `non_negative_length_measure`")); }
    } while (0);
    do { // convert the 'external_groove' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->external_groove, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 9 to solid_with_groove to be a `BOOLEAN`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_incomplete_circular_pattern>(const DB& db, const LIST& params, solid_with_incomplete_circular_pattern* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_circular_pattern*>(in));
    if (params.GetSize() < 10) { throw STEP::TypeError("expected 10 arguments to solid_with_incomplete_circular_pattern"); }    do { // convert the 'omitted_instances' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->omitted_instances, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 9 to solid_with_incomplete_circular_pattern to be a `SET [1:?] OF positive_integer`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_rectangular_pattern>(const DB& db, const LIST& params, solid_with_rectangular_pattern* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_shape_element_pattern*>(in));
    if (params.GetSize() < 9) { throw STEP::TypeError("expected 9 arguments to solid_with_rectangular_pattern"); }    do { // convert the 'row_count' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::solid_with_rectangular_pattern, 4>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->row_count, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to solid_with_rectangular_pattern to be a `positive_integer`")); }
    } while (0);
    do { // convert the 'column_count' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::solid_with_rectangular_pattern, 4>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->column_count, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to solid_with_rectangular_pattern to be a `positive_integer`")); }
    } while (0);
    do { // convert the 'row_spacing' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::solid_with_rectangular_pattern, 4>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->row_spacing, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to solid_with_rectangular_pattern to be a `length_measure`")); }
    } while (0);
    do { // convert the 'column_spacing' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::solid_with_rectangular_pattern, 4>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->column_spacing, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 8 to solid_with_rectangular_pattern to be a `length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_incomplete_rectangular_pattern>(const DB& db, const LIST& params, solid_with_incomplete_rectangular_pattern* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_rectangular_pattern*>(in));
    if (params.GetSize() < 9) { throw STEP::TypeError("expected 9 arguments to solid_with_incomplete_rectangular_pattern"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_rectangular_pocket>(const DB& db, const LIST& params, solid_with_rectangular_pocket* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_pocket*>(in));
    if (params.GetSize() < 10) { throw STEP::TypeError("expected 10 arguments to solid_with_rectangular_pocket"); }    do { // convert the 'pocket_length' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->pocket_length, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to solid_with_rectangular_pocket to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'pocket_width' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->pocket_width, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 8 to solid_with_rectangular_pocket to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'corner_radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->corner_radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 9 to solid_with_rectangular_pocket to be a `non_negative_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_rectangular_protrusion>(const DB& db, const LIST& params, solid_with_rectangular_protrusion* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_protrusion*>(in));
    if (params.GetSize() < 9) { throw STEP::TypeError("expected 9 arguments to solid_with_rectangular_protrusion"); }    do { // convert the 'protrusion_length' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->protrusion_length, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to solid_with_rectangular_protrusion to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'protrusion_width' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->protrusion_width, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to solid_with_rectangular_protrusion to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'protrusion_corner_radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->protrusion_corner_radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 8 to solid_with_rectangular_protrusion to be a `non_negative_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_single_offset_chamfer>(const DB& db, const LIST& params, solid_with_single_offset_chamfer* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_chamfered_edges*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to solid_with_single_offset_chamfer"); }    do { // convert the 'offset_distance' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->offset_distance, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to solid_with_single_offset_chamfer to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_spherical_bottom_round_hole>(const DB& db, const LIST& params, solid_with_spherical_bottom_round_hole* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_stepped_round_hole*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to solid_with_spherical_bottom_round_hole"); }    do { // convert the 'sphere_radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->sphere_radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to solid_with_spherical_bottom_round_hole to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_stepped_round_hole_and_conical_transitions>(const DB& db, const LIST& params, solid_with_stepped_round_hole_and_conical_transitions* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_stepped_round_hole*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to solid_with_stepped_round_hole_and_conical_transitions"); }    do { // convert the 'conical_transitions' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->conical_transitions, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to solid_with_stepped_round_hole_and_conical_transitions to be a `SET [1:?] OF conical_stepped_hole_transition`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_straight_slot>(const DB& db, const LIST& params, solid_with_straight_slot* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_slot*>(in));
    if (params.GetSize() < 8) { throw STEP::TypeError("expected 8 arguments to solid_with_straight_slot"); }    do { // convert the 'slot_length' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->slot_length, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to solid_with_straight_slot to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_tee_section_slot>(const DB& db, const LIST& params, solid_with_tee_section_slot* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_slot*>(in));
    if (params.GetSize() < 9) { throw STEP::TypeError("expected 9 arguments to solid_with_tee_section_slot"); }    do { // convert the 'tee_section_width' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->tee_section_width, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to solid_with_tee_section_slot to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'collar_depth' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->collar_depth, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 8 to solid_with_tee_section_slot to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_through_depression>(const DB& db, const LIST& params, solid_with_through_depression* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_depression*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to solid_with_through_depression"); }    do { // convert the 'exit_faces' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->exit_faces, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to solid_with_through_depression to be a `SET [1:?] OF face_surface`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_trapezoidal_section_slot>(const DB& db, const LIST& params, solid_with_trapezoidal_section_slot* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_with_slot*>(in));
    if (params.GetSize() < 9) { throw STEP::TypeError("expected 9 arguments to solid_with_trapezoidal_section_slot"); }    do { // convert the 'draft_angle' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->draft_angle, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to solid_with_trapezoidal_section_slot to be a `plane_angle_measure`")); }
    } while (0);
    do { // convert the 'floor_fillet_radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->floor_fillet_radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 8 to solid_with_trapezoidal_section_slot to be a `non_negative_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_with_variable_radius_edge_blend>(const DB& db, const LIST& params, solid_with_variable_radius_edge_blend* in)
{
    size_t base = 0;
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to solid_with_variable_radius_edge_blend"); }    do { // convert the 'point_list' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->point_list, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to solid_with_variable_radius_edge_blend to be a `LIST [2:?] OF point`")); }
    } while (0);
    do { // convert the 'radius_list' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->radius_list, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to solid_with_variable_radius_edge_blend to be a `LIST [2:?] OF positive_length_measure`")); }
    } while (0);
    do { // convert the 'edge_function_list' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->edge_function_list, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to solid_with_variable_radius_edge_blend to be a `LIST [1:?] OF blend_radius_variation_type`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<source_for_requirement>(const DB& db, const LIST& params, source_for_requirement* in)
{
    size_t base = GenericFill(db, params, static_cast<group_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to source_for_requirement"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to source_for_requirement to be a `SET [1:1] OF requirement_source_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<sourced_requirement>(const DB& db, const LIST& params, sourced_requirement* in)
{
    size_t base = GenericFill(db, params, static_cast<group_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to sourced_requirement"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to sourced_requirement to be a `SET [1:1] OF product_definition`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<specification_definition>(const DB& db, const LIST& params, specification_definition* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to specification_definition"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<specified_higher_usage_occurrence>(const DB& db, const LIST& params, specified_higher_usage_occurrence* in)
{
    size_t base = GenericFill(db, params, static_cast<assembly_component_usage*>(in));
    if (params.GetSize() < 8) { throw STEP::TypeError("expected 8 arguments to specified_higher_usage_occurrence"); }    do { // convert the 'upper_usage' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->upper_usage, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to specified_higher_usage_occurrence to be a `assembly_component_usage`")); }
    } while (0);
    do { // convert the 'next_usage' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->next_usage, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to specified_higher_usage_occurrence to be a `next_assembly_usage_occurrence`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<sphere>(const DB& db, const LIST& params, sphere* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to sphere"); }    do { // convert the 'radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to sphere to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'centre' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->centre, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to sphere to be a `point`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<spherical_surface>(const DB& db, const LIST& params, spherical_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<elementary_surface*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to spherical_surface"); }    do { // convert the 'radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to spherical_surface to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<start_request>(const DB& db, const LIST& params, start_request* in)
{
    size_t base = GenericFill(db, params, static_cast<action_request_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to start_request"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to start_request to be a `SET [1:?] OF start_request_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<start_work>(const DB& db, const LIST& params, start_work* in)
{
    size_t base = GenericFill(db, params, static_cast<action_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to start_work"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to start_work to be a `SET [1:?] OF work_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<straightness_tolerance>(const DB& db, const LIST& params, straightness_tolerance* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_tolerance*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to straightness_tolerance"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<structured_dimension_callout>(const DB& db, const LIST& params, structured_dimension_callout* in)
{
    size_t base = GenericFill(db, params, static_cast<draughting_callout*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to structured_dimension_callout"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<structured_text_composition>(const DB& db, const LIST& params, structured_text_composition* in)
{
    size_t base = GenericFill(db, params, static_cast<compound_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to structured_text_composition"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<structured_text_representation>(const DB& db, const LIST& params, structured_text_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to structured_text_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<subedge>(const DB& db, const LIST& params, subedge* in)
{
    size_t base = GenericFill(db, params, static_cast<edge*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to subedge"); }    do { // convert the 'parent_edge' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->parent_edge, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to subedge to be a `edge`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<subface>(const DB& db, const LIST& params, subface* in)
{
    size_t base = GenericFill(db, params, static_cast<face*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to subface"); }    do { // convert the 'parent_face' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->parent_face, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to subface to be a `face`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<supplied_part_relationship>(const DB& db, const LIST& params, supplied_part_relationship* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition_relationship*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to supplied_part_relationship"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_condition_callout>(const DB& db, const LIST& params, surface_condition_callout* in)
{
    size_t base = GenericFill(db, params, static_cast<draughting_callout*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to surface_condition_callout"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<swept_surface>(const DB& db, const LIST& params, swept_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<surface*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to swept_surface"); }    do { // convert the 'swept_curve' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::swept_surface, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->swept_curve, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to swept_surface to be a `curve`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_of_linear_extrusion>(const DB& db, const LIST& params, surface_of_linear_extrusion* in)
{
    size_t base = GenericFill(db, params, static_cast<swept_surface*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to surface_of_linear_extrusion"); }    do { // convert the 'extrusion_axis' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->extrusion_axis, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to surface_of_linear_extrusion to be a `vector`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_of_revolution>(const DB& db, const LIST& params, surface_of_revolution* in)
{
    size_t base = GenericFill(db, params, static_cast<swept_surface*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to surface_of_revolution"); }    do { // convert the 'axis_position' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->axis_position, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to surface_of_revolution to be a `axis1_placement`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_patch>(const DB& db, const LIST& params, surface_patch* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to surface_patch"); }    do { // convert the 'parent_surface' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->parent_surface, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to surface_patch to be a `bounded_surface`")); }
    } while (0);
    do { // convert the 'u_transition' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->u_transition, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to surface_patch to be a `transition_code`")); }
    } while (0);
    do { // convert the 'v_transition' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->v_transition, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to surface_patch to be a `transition_code`")); }
    } while (0);
    do { // convert the 'u_sense' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->u_sense, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to surface_patch to be a `BOOLEAN`")); }
    } while (0);
    do { // convert the 'v_sense' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->v_sense, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to surface_patch to be a `BOOLEAN`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_profile_tolerance>(const DB& db, const LIST& params, surface_profile_tolerance* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_tolerance*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to surface_profile_tolerance"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_replica>(const DB& db, const LIST& params, surface_replica* in)
{
    size_t base = GenericFill(db, params, static_cast<surface*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to surface_replica"); }    do { // convert the 'parent_surface' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->parent_surface, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to surface_replica to be a `surface`")); }
    } while (0);
    do { // convert the 'transformation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->transformation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to surface_replica to be a `cartesian_transformation_operator_3d`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_side_style>(const DB& db, const LIST& params, surface_side_style* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to surface_side_style"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to surface_side_style to be a `label`")); }
    } while (0);
    do { // convert the 'styles' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->styles, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to surface_side_style to be a `SET [1:7] OF surface_style_element_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_style_boundary>(const DB& db, const LIST& params, surface_style_boundary* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to surface_style_boundary"); }    do { // convert the 'style_of_boundary' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->style_of_boundary, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to surface_style_boundary to be a `curve_or_render`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_style_control_grid>(const DB& db, const LIST& params, surface_style_control_grid* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to surface_style_control_grid"); }    do { // convert the 'style_of_control_grid' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->style_of_control_grid, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to surface_style_control_grid to be a `curve_or_render`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_style_fill_area>(const DB& db, const LIST& params, surface_style_fill_area* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to surface_style_fill_area"); }    do { // convert the 'fill_area' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->fill_area, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to surface_style_fill_area to be a `fill_area_style`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_style_parameter_line>(const DB& db, const LIST& params, surface_style_parameter_line* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to surface_style_parameter_line"); }    do { // convert the 'style_of_parameter_lines' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->style_of_parameter_lines, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to surface_style_parameter_line to be a `curve_or_render`")); }
    } while (0);
    do { // convert the 'direction_counts' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->direction_counts, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to surface_style_parameter_line to be a `SET [1:2] OF direction_count_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_style_reflectance_ambient>(const DB& db, const LIST& params, surface_style_reflectance_ambient* in)
{
    size_t base = 0;
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to surface_style_reflectance_ambient"); }    do { // convert the 'ambient_reflectance' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::surface_style_reflectance_ambient, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->ambient_reflectance, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to surface_style_reflectance_ambient to be a `REAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_style_reflectance_ambient_diffuse>(const DB& db, const LIST& params, surface_style_reflectance_ambient_diffuse* in)
{
    size_t base = GenericFill(db, params, static_cast<surface_style_reflectance_ambient*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to surface_style_reflectance_ambient_diffuse"); }    do { // convert the 'diffuse_reflectance' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::surface_style_reflectance_ambient_diffuse, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->diffuse_reflectance, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to surface_style_reflectance_ambient_diffuse to be a `REAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_style_reflectance_ambient_diffuse_specular>(const DB& db, const LIST& params, surface_style_reflectance_ambient_diffuse_specular* in)
{
    size_t base = GenericFill(db, params, static_cast<surface_style_reflectance_ambient_diffuse*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to surface_style_reflectance_ambient_diffuse_specular"); }    do { // convert the 'specular_reflectance' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->specular_reflectance, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to surface_style_reflectance_ambient_diffuse_specular to be a `REAL`")); }
    } while (0);
    do { // convert the 'specular_exponent' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->specular_exponent, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to surface_style_reflectance_ambient_diffuse_specular to be a `REAL`")); }
    } while (0);
    do { // convert the 'specular_colour' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->specular_colour, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to surface_style_reflectance_ambient_diffuse_specular to be a `colour`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_style_rendering>(const DB& db, const LIST& params, surface_style_rendering* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to surface_style_rendering"); }    do { // convert the 'rendering_method' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::surface_style_rendering, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->rendering_method, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to surface_style_rendering to be a `shading_surface_method`")); }
    } while (0);
    do { // convert the 'surface_colour' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::surface_style_rendering, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->surface_colour, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to surface_style_rendering to be a `colour`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_style_rendering_with_properties>(const DB& db, const LIST& params, surface_style_rendering_with_properties* in)
{
    size_t base = GenericFill(db, params, static_cast<surface_style_rendering*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to surface_style_rendering_with_properties"); }    do { // convert the 'properties' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->properties, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to surface_style_rendering_with_properties to be a `SET [1:2] OF rendering_properties_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_style_segmentation_curve>(const DB& db, const LIST& params, surface_style_segmentation_curve* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to surface_style_segmentation_curve"); }    do { // convert the 'style_of_segmentation_curve' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->style_of_segmentation_curve, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to surface_style_segmentation_curve to be a `curve_or_render`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_style_silhouette>(const DB& db, const LIST& params, surface_style_silhouette* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to surface_style_silhouette"); }    do { // convert the 'style_of_silhouette' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->style_of_silhouette, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to surface_style_silhouette to be a `curve_or_render`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_style_usage>(const DB& db, const LIST& params, surface_style_usage* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to surface_style_usage"); }    do { // convert the 'side' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->side, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to surface_style_usage to be a `surface_side`")); }
    } while (0);
    do { // convert the 'style' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->style, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to surface_style_usage to be a `surface_side_style_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface_texture_representation>(const DB& db, const LIST& params, surface_texture_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to surface_texture_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surfaced_open_shell>(const DB& db, const LIST& params, surfaced_open_shell* in)
{
    size_t base = GenericFill(db, params, static_cast<open_shell*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to surfaced_open_shell"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<swept_disk_solid>(const DB& db, const LIST& params, swept_disk_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_model*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to swept_disk_solid"); }    do { // convert the 'directrix' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->directrix, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to swept_disk_solid to be a `curve`")); }
    } while (0);
    do { // convert the 'radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to swept_disk_solid to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'inner_radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->inner_radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to swept_disk_solid to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'start_param' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->start_param, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to swept_disk_solid to be a `REAL`")); }
    } while (0);
    do { // convert the 'end_param' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->end_param, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to swept_disk_solid to be a `REAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<symbol>(const DB& db, const LIST& params, symbol* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to symbol"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<symbol_representation_map>(const DB& db, const LIST& params, symbol_representation_map* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_map*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to symbol_representation_map"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<symbol_style>(const DB& db, const LIST& params, symbol_style* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to symbol_style"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to symbol_style to be a `label`")); }
    } while (0);
    do { // convert the 'style_of_symbol' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->style_of_symbol, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to symbol_style to be a `symbol_style_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<symbol_target>(const DB& db, const LIST& params, symbol_target* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to symbol_target"); }    do { // convert the 'placement' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->placement, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to symbol_target to be a `axis2_placement`")); }
    } while (0);
    do { // convert the 'x_scale' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->x_scale, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to symbol_target to be a `positive_ratio_measure`")); }
    } while (0);
    do { // convert the 'y_scale' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->y_scale, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to symbol_target to be a `positive_ratio_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<symmetric_shape_aspect>(const DB& db, const LIST& params, symmetric_shape_aspect* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_aspect*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to symmetric_shape_aspect"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<symmetry_tolerance>(const DB& db, const LIST& params, symmetry_tolerance* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_tolerance_with_datum_reference*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to symmetry_tolerance"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<table_representation_item>(const DB& db, const LIST& params, table_representation_item* in)
{
    size_t base = GenericFill(db, params, static_cast<compound_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to table_representation_item"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<tactile_appearance_representation>(const DB& db, const LIST& params, tactile_appearance_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to tactile_appearance_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<tagged_text_format>(const DB& db, const LIST& params, tagged_text_format* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_context*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to tagged_text_format"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<tagged_text_item>(const DB& db, const LIST& params, tagged_text_item* in)
{
    size_t base = GenericFill(db, params, static_cast<descriptive_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to tagged_text_item"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<tangent>(const DB& db, const LIST& params, tangent* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_shape_aspect*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to tangent"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<text_literal_with_associated_curves>(const DB& db, const LIST& params, text_literal_with_associated_curves* in)
{
    size_t base = GenericFill(db, params, static_cast<text_literal*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to text_literal_with_associated_curves"); }    do { // convert the 'associated_curves' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->associated_curves, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to text_literal_with_associated_curves to be a `SET [1:?] OF curve`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<text_literal_with_blanking_box>(const DB& db, const LIST& params, text_literal_with_blanking_box* in)
{
    size_t base = GenericFill(db, params, static_cast<text_literal*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to text_literal_with_blanking_box"); }    do { // convert the 'blanking' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->blanking, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to text_literal_with_blanking_box to be a `planar_box`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<text_literal_with_extent>(const DB& db, const LIST& params, text_literal_with_extent* in)
{
    size_t base = GenericFill(db, params, static_cast<text_literal*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to text_literal_with_extent"); }    do { // convert the 'extent' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->extent, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to text_literal_with_extent to be a `planar_extent`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<text_string_representation>(const DB& db, const LIST& params, text_string_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to text_string_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<text_style>(const DB& db, const LIST& params, text_style* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to text_style"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::text_style, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to text_style to be a `label`")); }
    } while (0);
    do { // convert the 'character_appearance' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::text_style, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->character_appearance, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to text_style to be a `character_style_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<text_style_with_box_characteristics>(const DB& db, const LIST& params, text_style_with_box_characteristics* in)
{
    size_t base = GenericFill(db, params, static_cast<text_style*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to text_style_with_box_characteristics"); }    do { // convert the 'characteristics' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->characteristics, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to text_style_with_box_characteristics to be a `SET [1:4] OF box_characteristic_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<text_style_with_mirror>(const DB& db, const LIST& params, text_style_with_mirror* in)
{
    size_t base = GenericFill(db, params, static_cast<text_style*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to text_style_with_mirror"); }    do { // convert the 'mirror_placement' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->mirror_placement, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to text_style_with_mirror to be a `axis2_placement`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<text_style_with_spacing>(const DB& db, const LIST& params, text_style_with_spacing* in)
{
    size_t base = GenericFill(db, params, static_cast<text_style*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to text_style_with_spacing"); }    do { // convert the 'character_spacing' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->character_spacing, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to text_style_with_spacing to be a `character_spacing_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<thermal_resistance_measure_with_unit>(const DB& db, const LIST& params, thermal_resistance_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to thermal_resistance_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<thermal_resistance_unit>(const DB& db, const LIST& params, thermal_resistance_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to thermal_resistance_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<thermodynamic_temperature_measure_with_unit>(const DB& db, const LIST& params, thermodynamic_temperature_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to thermodynamic_temperature_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<thermodynamic_temperature_unit>(const DB& db, const LIST& params, thermodynamic_temperature_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<named_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to thermodynamic_temperature_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<thickened_face_solid>(const DB& db, const LIST& params, thickened_face_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_model*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to thickened_face_solid"); }    do { // convert the 'base_element' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->base_element, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to thickened_face_solid to be a `generalized_surface_select`")); }
    } while (0);
    do { // convert the 'offset1' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->offset1, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to thickened_face_solid to be a `length_measure`")); }
    } while (0);
    do { // convert the 'offset2' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->offset2, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to thickened_face_solid to be a `length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<thickness_laminate_definition>(const DB& db, const LIST& params, thickness_laminate_definition* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to thickness_laminate_definition"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<thickness_laminate_table>(const DB& db, const LIST& params, thickness_laminate_table* in)
{
    size_t base = GenericFill(db, params, static_cast<zone_structural_makeup*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to thickness_laminate_table"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<time_interval>(const DB& db, const LIST& params, time_interval* in)
{
    size_t base = 0;
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to time_interval"); }    do { // convert the 'id' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::time_interval, 3>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->id, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to time_interval to be a `identifier`")); }
    } while (0);
    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::time_interval, 3>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to time_interval to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::time_interval, 3>::aux_is_derived[2] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to time_interval to be a `text`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<time_interval_based_effectivity>(const DB& db, const LIST& params, time_interval_based_effectivity* in)
{
    size_t base = GenericFill(db, params, static_cast<effectivity*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to time_interval_based_effectivity"); }    do { // convert the 'effectivity_period' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->effectivity_period, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to time_interval_based_effectivity to be a `time_interval`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<time_interval_with_bounds>(const DB& db, const LIST& params, time_interval_with_bounds* in)
{
    size_t base = GenericFill(db, params, static_cast<time_interval*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to time_interval_with_bounds"); }    do { // convert the 'primary_bound' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->primary_bound, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to time_interval_with_bounds to be a `date_time_or_event_occurrence`")); }
    } while (0);
    do { // convert the 'secondary_bound' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->secondary_bound, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to time_interval_with_bounds to be a `date_time_or_event_occurrence`")); }
    } while (0);
    do { // convert the 'duration' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->duration, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to time_interval_with_bounds to be a `time_measure_with_unit`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<time_measure_with_unit>(const DB& db, const LIST& params, time_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to time_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<time_unit>(const DB& db, const LIST& params, time_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<named_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to time_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<tolerance_zone>(const DB& db, const LIST& params, tolerance_zone* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_aspect*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to tolerance_zone"); }    do { // convert the 'defining_tolerance' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->defining_tolerance, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to tolerance_zone to be a `SET [1:?] OF geometric_tolerance`")); }
    } while (0);
    do { // convert the 'form' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->form, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to tolerance_zone to be a `tolerance_zone_form`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<torus>(const DB& db, const LIST& params, torus* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to torus"); }    do { // convert the 'position' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->position, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to torus to be a `axis1_placement`")); }
    } while (0);
    do { // convert the 'major_radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->major_radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to torus to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'minor_radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->minor_radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to torus to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<total_runout_tolerance>(const DB& db, const LIST& params, total_runout_tolerance* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_tolerance_with_datum_reference*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to total_runout_tolerance"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<track_blended_solid>(const DB& db, const LIST& params, track_blended_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<edge_blended_solid*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to track_blended_solid"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<track_blended_solid_with_end_conditions>(const DB& db, const LIST& params, track_blended_solid_with_end_conditions* in)
{
    size_t base = GenericFill(db, params, static_cast<track_blended_solid*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to track_blended_solid_with_end_conditions"); }    do { // convert the 'end_conditions' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->end_conditions, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to track_blended_solid_with_end_conditions to be a `LIST [2:2] OF blend_end_condition_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<trimmed_curve>(const DB& db, const LIST& params, trimmed_curve* in)
{
    size_t base = GenericFill(db, params, static_cast<bounded_curve*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to trimmed_curve"); }    do { // convert the 'basis_curve' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->basis_curve, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to trimmed_curve to be a `curve`")); }
    } while (0);
    do { // convert the 'trim_1' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->trim_1, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to trimmed_curve to be a `SET [1:2] OF trimming_select`")); }
    } while (0);
    do { // convert the 'trim_2' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->trim_2, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to trimmed_curve to be a `SET [1:2] OF trimming_select`")); }
    } while (0);
    do { // convert the 'sense_agreement' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->sense_agreement, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to trimmed_curve to be a `BOOLEAN`")); }
    } while (0);
    do { // convert the 'master_representation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->master_representation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to trimmed_curve to be a `trimming_preference`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<two_direction_repeat_factor>(const DB& db, const LIST& params, two_direction_repeat_factor* in)
{
    size_t base = GenericFill(db, params, static_cast<one_direction_repeat_factor*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to two_direction_repeat_factor"); }    do { // convert the 'second_repeat_factor' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->second_repeat_factor, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to two_direction_repeat_factor to be a `vector`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<unary_generic_expression>(const DB& db, const LIST& params, unary_generic_expression* in)
{
    size_t base = GenericFill(db, params, static_cast<generic_expression*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to unary_generic_expression"); }    do { // convert the 'operand' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->operand, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to unary_generic_expression to be a `generic_expression`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<unary_numeric_expression>(const DB& db, const LIST& params, unary_numeric_expression* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<uncertainty_assigned_representation>(const DB& db, const LIST& params, uncertainty_assigned_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to uncertainty_assigned_representation"); }    do { // convert the 'uncertainty' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->uncertainty, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to uncertainty_assigned_representation to be a `SET [1:?] OF uncertainty_measure_with_unit`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<uncertainty_measure_with_unit>(const DB& db, const LIST& params, uncertainty_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to uncertainty_measure_with_unit"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to uncertainty_measure_with_unit to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to uncertainty_measure_with_unit to be a `text`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<uniform_curve>(const DB& db, const LIST& params, uniform_curve* in)
{
    size_t base = GenericFill(db, params, static_cast<b_spline_curve*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to uniform_curve"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<uniform_resource_identifier>(const DB& db, const LIST& params, uniform_resource_identifier* in)
{
    size_t base = GenericFill(db, params, static_cast<descriptive_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to uniform_resource_identifier"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<uniform_surface>(const DB& db, const LIST& params, uniform_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<b_spline_surface*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to uniform_surface"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<usage_association>(const DB& db, const LIST& params, usage_association* in)
{
    size_t base = GenericFill(db, params, static_cast<action_method_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to usage_association"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<user_defined_curve_font>(const DB& db, const LIST& params, user_defined_curve_font* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<user_defined_marker>(const DB& db, const LIST& params, user_defined_marker* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<user_defined_terminator_symbol>(const DB& db, const LIST& params, user_defined_terminator_symbol* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<user_selected_shape_elements>(const DB& db, const LIST& params, user_selected_shape_elements* in)
{
    size_t base = GenericFill(db, params, static_cast<user_selected_elements*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to user_selected_shape_elements"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<value_range>(const DB& db, const LIST& params, value_range* in)
{
    size_t base = GenericFill(db, params, static_cast<compound_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to value_range"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<value_representation_item>(const DB& db, const LIST& params, value_representation_item* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to value_representation_item"); }    do { // convert the 'value_component' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->value_component, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to value_representation_item to be a `measure_value`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<variable_semantics>(const DB& db, const LIST& params, variable_semantics* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<variational_representation_item>(const DB& db, const LIST& params, variational_representation_item* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to variational_representation_item"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<vector>(const DB& db, const LIST& params, vector* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to vector"); }    do { // convert the 'orientation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->orientation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to vector to be a `direction`")); }
    } while (0);
    do { // convert the 'magnitude' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->magnitude, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to vector to be a `length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<vector_style>(const DB& db, const LIST& params, vector_style* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<velocity_measure_with_unit>(const DB& db, const LIST& params, velocity_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to velocity_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<velocity_unit>(const DB& db, const LIST& params, velocity_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to velocity_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<vertex>(const DB& db, const LIST& params, vertex* in)
{
    size_t base = GenericFill(db, params, static_cast<topological_representation_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to vertex"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<vertex_loop>(const DB& db, const LIST& params, vertex_loop* in)
{
    size_t base = GenericFill(db, params, static_cast<loop*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to vertex_loop"); }    do { // convert the 'loop_vertex' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->loop_vertex, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to vertex_loop to be a `vertex`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<vertex_point>(const DB& db, const LIST& params, vertex_point* in)
{
    size_t base = 0;
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to vertex_point"); }    do { // convert the 'vertex_geometry' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->vertex_geometry, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to vertex_point to be a `point`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<vertex_shell>(const DB& db, const LIST& params, vertex_shell* in)
{
    size_t base = GenericFill(db, params, static_cast<topological_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to vertex_shell"); }    do { // convert the 'vertex_shell_extent' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->vertex_shell_extent, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to vertex_shell to be a `vertex_loop`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<view_volume>(const DB& db, const LIST& params, view_volume* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 9) { throw STEP::TypeError("expected 9 arguments to view_volume"); }    do { // convert the 'projection_type' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->projection_type, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to view_volume to be a `central_or_parallel`")); }
    } while (0);
    do { // convert the 'projection_point' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->projection_point, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to view_volume to be a `cartesian_point`")); }
    } while (0);
    do { // convert the 'view_plane_distance' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->view_plane_distance, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to view_volume to be a `length_measure`")); }
    } while (0);
    do { // convert the 'front_plane_distance' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->front_plane_distance, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to view_volume to be a `length_measure`")); }
    } while (0);
    do { // convert the 'front_plane_clipping' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->front_plane_clipping, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to view_volume to be a `BOOLEAN`")); }
    } while (0);
    do { // convert the 'back_plane_distance' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->back_plane_distance, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to view_volume to be a `length_measure`")); }
    } while (0);
    do { // convert the 'back_plane_clipping' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->back_plane_clipping, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to view_volume to be a `BOOLEAN`")); }
    } while (0);
    do { // convert the 'view_volume_sides_clipping' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->view_volume_sides_clipping, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to view_volume to be a `BOOLEAN`")); }
    } while (0);
    do { // convert the 'view_window' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->view_window, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 8 to view_volume to be a `planar_box`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<visual_appearance_representation>(const DB& db, const LIST& params, visual_appearance_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to visual_appearance_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<volume_measure_with_unit>(const DB& db, const LIST& params, volume_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to volume_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<volume_unit>(const DB& db, const LIST& params, volume_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to volume_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<week_of_year_and_day_date>(const DB& db, const LIST& params, week_of_year_and_day_date* in)
{
    size_t base = GenericFill(db, params, static_cast<date*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to week_of_year_and_day_date"); }    do { // convert the 'week_component' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->week_component, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to week_of_year_and_day_date to be a `week_in_year_number`")); }
    } while (0);
    do { // convert the 'day_component' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->day_component, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to week_of_year_and_day_date to be a `day_in_week_number`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<wire_shell>(const DB& db, const LIST& params, wire_shell* in)
{
    size_t base = GenericFill(db, params, static_cast<topological_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to wire_shell"); }    do { // convert the 'wire_shell_extent' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->wire_shell_extent, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to wire_shell to be a `SET [1:?] OF loop`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<year_month>(const DB& db, const LIST& params, year_month* in)
{
    size_t base = GenericFill(db, params, static_cast<date*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to year_month"); }    do { // convert the 'month_component' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->month_component, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to year_month to be a `month_in_year_number`")); }
    } while (0);
    return base;
}

}
}
