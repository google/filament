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
    template <> size_t GenericFill<classification_assignment>(const DB& db, const LIST& params, classification_assignment* in)
    {
        size_t base = 0;
        if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to classification_assignment"); }    do { // convert the 'assigned_class' argument
            std::shared_ptr<const DataType> arg = params[base++];
            if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::classification_assignment, 2>::aux_is_derived[0] = true; break; }
            try { GenericConvert(in->assigned_class, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to classification_assignment to be a `group`")); }
        } while (0);
        do { // convert the 'role' argument
            std::shared_ptr<const DataType> arg = params[base++];
            if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::classification_assignment, 2>::aux_is_derived[1] = true; break; }
            try { GenericConvert(in->role, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to classification_assignment to be a `classification_role`")); }
        } while (0);
        return base;
    }
    // -----------------------------------------------------------------------------------------------------------
    template <> size_t GenericFill<applied_classification_assignment>(const DB& db, const LIST& params, applied_classification_assignment* in)
    {
        size_t base = GenericFill(db, params, static_cast<classification_assignment*>(in));
        if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to applied_classification_assignment"); }    do { // convert the 'items' argument
            std::shared_ptr<const DataType> arg = params[base++];
            try { GenericConvert(in->items, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to applied_classification_assignment to be a `SET [1:?] OF classification_item`")); }
        } while (0);
        return base;
    }
    // -----------------------------------------------------------------------------------------------------------
    template <> size_t GenericFill<contract_assignment>(const DB& db, const LIST& params, contract_assignment* in)
    {
        size_t base = 0;
        if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to contract_assignment"); }    do { // convert the 'assigned_contract' argument
            std::shared_ptr<const DataType> arg = params[base++];
            if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::contract_assignment, 1>::aux_is_derived[0] = true; break; }
            try { GenericConvert(in->assigned_contract, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to contract_assignment to be a `contract`")); }
        } while (0);
        return base;
    }
    // -----------------------------------------------------------------------------------------------------------
    template <> size_t GenericFill<applied_contract_assignment>(const DB& db, const LIST& params, applied_contract_assignment* in)
    {
        size_t base = GenericFill(db, params, static_cast<contract_assignment*>(in));
        if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to applied_contract_assignment"); }    do { // convert the 'items' argument
            std::shared_ptr<const DataType> arg = params[base++];
            try { GenericConvert(in->items, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to applied_contract_assignment to be a `SET [1:?] OF contract_item`")); }
        } while (0);
        return base;
    }
    // -----------------------------------------------------------------------------------------------------------
    template <> size_t GenericFill<date_and_time_assignment>(const DB& db, const LIST& params, date_and_time_assignment* in)
    {
        size_t base = 0;
        if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to date_and_time_assignment"); }    do { // convert the 'assigned_date_and_time' argument
            std::shared_ptr<const DataType> arg = params[base++];
            if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::date_and_time_assignment, 2>::aux_is_derived[0] = true; break; }
            try { GenericConvert(in->assigned_date_and_time, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to date_and_time_assignment to be a `date_and_time`")); }
        } while (0);
        do { // convert the 'role' argument
            std::shared_ptr<const DataType> arg = params[base++];
            if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::date_and_time_assignment, 2>::aux_is_derived[1] = true; break; }
            try { GenericConvert(in->role, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to date_and_time_assignment to be a `date_time_role`")); }
        } while (0);
        return base;
    }
    // -----------------------------------------------------------------------------------------------------------
    template <> size_t GenericFill<applied_date_and_time_assignment>(const DB& db, const LIST& params, applied_date_and_time_assignment* in)
    {
        size_t base = GenericFill(db, params, static_cast<date_and_time_assignment*>(in));
        if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to applied_date_and_time_assignment"); }    do { // convert the 'items' argument
            std::shared_ptr<const DataType> arg = params[base++];
            try { GenericConvert(in->items, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to applied_date_and_time_assignment to be a `SET [1:?] OF date_and_time_item`")); }
        } while (0);
        return base;
    }
    // -----------------------------------------------------------------------------------------------------------
    template <> size_t GenericFill<date_assignment>(const DB& db, const LIST& params, date_assignment* in)
    {
        size_t base = 0;
        if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to date_assignment"); }    do { // convert the 'assigned_date' argument
            std::shared_ptr<const DataType> arg = params[base++];
            if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::date_assignment, 2>::aux_is_derived[0] = true; break; }
            try { GenericConvert(in->assigned_date, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to date_assignment to be a `date`")); }
        } while (0);
        do { // convert the 'role' argument
            std::shared_ptr<const DataType> arg = params[base++];
            if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::date_assignment, 2>::aux_is_derived[1] = true; break; }
            try { GenericConvert(in->role, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to date_assignment to be a `date_role`")); }
        } while (0);
        return base;
    }
    // -----------------------------------------------------------------------------------------------------------
    template <> size_t GenericFill<applied_date_assignment>(const DB& db, const LIST& params, applied_date_assignment* in)
    {
        size_t base = GenericFill(db, params, static_cast<date_assignment*>(in));
        if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to applied_date_assignment"); }    do { // convert the 'items' argument
            std::shared_ptr<const DataType> arg = params[base++];
            try { GenericConvert(in->items, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to applied_date_assignment to be a `SET [1:?] OF date_item`")); }
        } while (0);
        return base;
    }
    // -----------------------------------------------------------------------------------------------------------
    template <> size_t GenericFill<document_reference>(const DB& db, const LIST& params, document_reference* in)
    {
        size_t base = 0;
        if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to document_reference"); }    do { // convert the 'assigned_document' argument
            std::shared_ptr<const DataType> arg = params[base++];
            if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::document_reference, 2>::aux_is_derived[0] = true; break; }
            try { GenericConvert(in->assigned_document, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to document_reference to be a `document`")); }
        } while (0);
        do { // convert the 'source' argument
            std::shared_ptr<const DataType> arg = params[base++];
            if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::document_reference, 2>::aux_is_derived[1] = true; break; }
            try { GenericConvert(in->source, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to document_reference to be a `label`")); }
        } while (0);
        return base;
    }
    // -----------------------------------------------------------------------------------------------------------
    template <> size_t GenericFill<applied_document_reference>(const DB& db, const LIST& params, applied_document_reference* in)
    {
        size_t base = GenericFill(db, params, static_cast<document_reference*>(in));
        if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to applied_document_reference"); }    do { // convert the 'items' argument
            std::shared_ptr<const DataType> arg = params[base++];
            try { GenericConvert(in->items, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to applied_document_reference to be a `SET [1:?] OF document_reference_item`")); }
        } while (0);
        return base;
    }
    // -----------------------------------------------------------------------------------------------------------
    template <> size_t GenericFill<document_usage_constraint_assignment>(const DB& db, const LIST& params, document_usage_constraint_assignment* in)
    {
        size_t base = 0;
        if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to document_usage_constraint_assignment"); }    do { // convert the 'assigned_document_usage' argument
            std::shared_ptr<const DataType> arg = params[base++];
            if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::document_usage_constraint_assignment, 2>::aux_is_derived[0] = true; break; }
            try { GenericConvert(in->assigned_document_usage, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to document_usage_constraint_assignment to be a `document_usage_constraint`")); }
        } while (0);
        do { // convert the 'role' argument
            std::shared_ptr<const DataType> arg = params[base++];
            if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::document_usage_constraint_assignment, 2>::aux_is_derived[1] = true; break; }
            try { GenericConvert(in->role, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to document_usage_constraint_assignment to be a `document_usage_role`")); }
        } while (0);
        return base;
    }
    // -----------------------------------------------------------------------------------------------------------
    template <> size_t GenericFill<applied_document_usage_constraint_assignment>(const DB& db, const LIST& params, applied_document_usage_constraint_assignment* in)
    {
        size_t base = GenericFill(db, params, static_cast<document_usage_constraint_assignment*>(in));
        if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to applied_document_usage_constraint_assignment"); }    do { // convert the 'items' argument
            std::shared_ptr<const DataType> arg = params[base++];
            try { GenericConvert(in->items, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to applied_document_usage_constraint_assignment to be a `SET [1:?] OF document_reference_item`")); }
        } while (0);
        return base;
    }
    // -----------------------------------------------------------------------------------------------------------
    template <> size_t GenericFill<effectivity_assignment>(const DB& db, const LIST& params, effectivity_assignment* in)
    {
        size_t base = 0;
        if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to effectivity_assignment"); }    do { // convert the 'assigned_effectivity' argument
            std::shared_ptr<const DataType> arg = params[base++];
            if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::effectivity_assignment, 1>::aux_is_derived[0] = true; break; }
            try { GenericConvert(in->assigned_effectivity, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to effectivity_assignment to be a `effectivity`")); }
        } while (0);
        return base;
    }
    // -----------------------------------------------------------------------------------------------------------
    template <> size_t GenericFill<applied_effectivity_assignment>(const DB& db, const LIST& params, applied_effectivity_assignment* in)
    {
        size_t base = GenericFill(db, params, static_cast<effectivity_assignment*>(in));
        if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to applied_effectivity_assignment"); }    do { // convert the 'items' argument
            std::shared_ptr<const DataType> arg = params[base++];
            try { GenericConvert(in->items, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to applied_effectivity_assignment to be a `SET [1:?] OF effectivity_item`")); }
        } while (0);
        return base;
    }
    // -----------------------------------------------------------------------------------------------------------
    template <> size_t GenericFill<event_occurrence_assignment>(const DB& db, const LIST& params, event_occurrence_assignment* in)
    {
        size_t base = 0;
        if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to event_occurrence_assignment"); }    do { // convert the 'assigned_event_occurrence' argument
            std::shared_ptr<const DataType> arg = params[base++];
            if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::event_occurrence_assignment, 2>::aux_is_derived[0] = true; break; }
            try { GenericConvert(in->assigned_event_occurrence, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to event_occurrence_assignment to be a `event_occurrence`")); }
        } while (0);
        do { // convert the 'role' argument
            std::shared_ptr<const DataType> arg = params[base++];
            if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::event_occurrence_assignment, 2>::aux_is_derived[1] = true; break; }
            try { GenericConvert(in->role, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to event_occurrence_assignment to be a `event_occurrence_role`")); }
        } while (0);
        return base;
    }
    // -----------------------------------------------------------------------------------------------------------
    template <> size_t GenericFill<applied_event_occurrence_assignment>(const DB& db, const LIST& params, applied_event_occurrence_assignment* in)
    {
        size_t base = GenericFill(db, params, static_cast<event_occurrence_assignment*>(in));
        if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to applied_event_occurrence_assignment"); }    do { // convert the 'items' argument
            std::shared_ptr<const DataType> arg = params[base++];
            try { GenericConvert(in->items, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to applied_event_occurrence_assignment to be a `SET [1:?] OF event_occurrence_item`")); }
        } while (0);
        return base;
    }
    // -----------------------------------------------------------------------------------------------------------
    template <> size_t GenericFill<identification_assignment>(const DB& db, const LIST& params, identification_assignment* in)
    {
        size_t base = 0;
        if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to identification_assignment"); }    do { // convert the 'assigned_id' argument
            std::shared_ptr<const DataType> arg = params[base++];
            if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::identification_assignment, 2>::aux_is_derived[0] = true; break; }
            try { GenericConvert(in->assigned_id, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to identification_assignment to be a `identifier`")); }
        } while (0);
        do { // convert the 'role' argument
            std::shared_ptr<const DataType> arg = params[base++];
            if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::identification_assignment, 2>::aux_is_derived[1] = true; break; }
            try { GenericConvert(in->role, arg, db); break; }
            catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to identification_assignment to be a `identification_role`")); }
        } while (0);
        return base;
    }
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<external_identification_assignment>(const DB& db, const LIST& params, external_identification_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<identification_assignment*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to external_identification_assignment"); }    do { // convert the 'source' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::external_identification_assignment, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->source, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to external_identification_assignment to be a `external_source`")); }
    } while (0);
    return base;
}

// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<applied_external_identification_assignment>(const DB& db, const LIST& params, applied_external_identification_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<external_identification_assignment*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to applied_external_identification_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to applied_external_identification_assignment to be a `SET [1:?] OF external_identification_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<group_assignment>(const DB& db, const LIST& params, group_assignment* in)
{
    size_t base = 0;
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to group_assignment"); }    do { // convert the 'assigned_group' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::group_assignment, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->assigned_group, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to group_assignment to be a `group`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<applied_group_assignment>(const DB& db, const LIST& params, applied_group_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<group_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to applied_group_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to applied_group_assignment to be a `SET [1:?] OF groupable_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<applied_identification_assignment>(const DB& db, const LIST& params, applied_identification_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<identification_assignment*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to applied_identification_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to applied_identification_assignment to be a `SET [1:?] OF identification_item`")); }
    } while (0);
    return base;
}

// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<name_assignment>(const DB& db, const LIST& params, name_assignment* in)
{
    size_t base = 0;
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to name_assignment"); }    do { // convert the 'assigned_name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::name_assignment, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->assigned_name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to name_assignment to be a `label`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<applied_name_assignment>(const DB& db, const LIST& params, applied_name_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<name_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to applied_name_assignment"); }    do { // convert the 'item' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->item, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to applied_name_assignment to be a `name_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<organization_assignment>(const DB& db, const LIST& params, organization_assignment* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to organization_assignment"); }    do { // convert the 'assigned_organization' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::organization_assignment, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->assigned_organization, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to organization_assignment to be a `organization`")); }
    } while (0);
    do { // convert the 'role' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::organization_assignment, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->role, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to organization_assignment to be a `organization_role`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<applied_organization_assignment>(const DB& db, const LIST& params, applied_organization_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<organization_assignment*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to applied_organization_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to applied_organization_assignment to be a `SET [1:?] OF organization_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<organizational_project_assignment>(const DB& db, const LIST& params, organizational_project_assignment* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to organizational_project_assignment"); }    do { // convert the 'assigned_organizational_project' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::organizational_project_assignment, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->assigned_organizational_project, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to organizational_project_assignment to be a `organizational_project`")); }
    } while (0);
    do { // convert the 'role' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::organizational_project_assignment, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->role, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to organizational_project_assignment to be a `organizational_project_role`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<applied_organizational_project_assignment>(const DB& db, const LIST& params, applied_organizational_project_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<organizational_project_assignment*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to applied_organizational_project_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to applied_organizational_project_assignment to be a `SET [1:?] OF project_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<person_and_organization_assignment>(const DB& db, const LIST& params, person_and_organization_assignment* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to person_and_organization_assignment"); }    do { // convert the 'assigned_person_and_organization' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::person_and_organization_assignment, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->assigned_person_and_organization, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to person_and_organization_assignment to be a `person_and_organization`")); }
    } while (0);
    do { // convert the 'role' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::person_and_organization_assignment, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->role, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to person_and_organization_assignment to be a `person_and_organization_role`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<applied_person_and_organization_assignment>(const DB& db, const LIST& params, applied_person_and_organization_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<person_and_organization_assignment*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to applied_person_and_organization_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to applied_person_and_organization_assignment to be a `SET [1:?] OF person_and_organization_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<presented_item>(const DB& db, const LIST& params, presented_item* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<applied_presented_item>(const DB& db, const LIST& params, applied_presented_item* in)
{
    size_t base = GenericFill(db, params, static_cast<presented_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to applied_presented_item"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to applied_presented_item to be a `SET [1:?] OF presented_item_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<security_classification_assignment>(const DB& db, const LIST& params, security_classification_assignment* in)
{
    size_t base = 0;
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to security_classification_assignment"); }    do { // convert the 'assigned_security_classification' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::security_classification_assignment, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->assigned_security_classification, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to security_classification_assignment to be a `security_classification`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<applied_security_classification_assignment>(const DB& db, const LIST& params, applied_security_classification_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<security_classification_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to applied_security_classification_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to applied_security_classification_assignment to be a `SET [1:?] OF security_classification_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<time_interval_assignment>(const DB& db, const LIST& params, time_interval_assignment* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to time_interval_assignment"); }    do { // convert the 'assigned_time_interval' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::time_interval_assignment, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->assigned_time_interval, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to time_interval_assignment to be a `time_interval`")); }
    } while (0);
    do { // convert the 'role' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::time_interval_assignment, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->role, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to time_interval_assignment to be a `time_interval_role`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<applied_time_interval_assignment>(const DB& db, const LIST& params, applied_time_interval_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<time_interval_assignment*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to applied_time_interval_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to applied_time_interval_assignment to be a `SET [0:?] OF time_interval_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<applied_usage_right>(const DB& db, const LIST& params, applied_usage_right* in)
{
    size_t base = GenericFill(db, params, static_cast<applied_action_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to applied_usage_right"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<area_in_set>(const DB& db, const LIST& params, area_in_set* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to area_in_set"); }    do { // convert the 'area' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::area_in_set, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->area, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to area_in_set to be a `presentation_area`")); }
    } while (0);
    do { // convert the 'in_set' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::area_in_set, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->in_set, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to area_in_set to be a `presentation_set`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<area_measure_with_unit>(const DB& db, const LIST& params, area_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to area_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<area_unit>(const DB& db, const LIST& params, area_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to area_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_definition_relationship>(const DB& db, const LIST& params, product_definition_relationship* in)
{
    size_t base = 0;
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to product_definition_relationship"); }    do { // convert the 'id' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_definition_relationship, 5>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->id, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to product_definition_relationship to be a `identifier`")); }
    } while (0);
    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_definition_relationship, 5>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to product_definition_relationship to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_definition_relationship, 5>::aux_is_derived[2] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to product_definition_relationship to be a `text`")); }
    } while (0);
    do { // convert the 'relating_product_definition' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_definition_relationship, 5>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->relating_product_definition, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to product_definition_relationship to be a `product_definition`")); }
    } while (0);
    do { // convert the 'related_product_definition' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_definition_relationship, 5>::aux_is_derived[4] = true; break; }
        try { GenericConvert(in->related_product_definition, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to product_definition_relationship to be a `product_definition`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_definition_usage>(const DB& db, const LIST& params, product_definition_usage* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition_relationship*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to product_definition_usage"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<assembly_component_usage>(const DB& db, const LIST& params, assembly_component_usage* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition_usage*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to assembly_component_usage"); }    do { // convert the 'reference_designator' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::assembly_component_usage, 1>::aux_is_derived[0] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->reference_designator, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to assembly_component_usage to be a `identifier`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<assigned_requirement>(const DB& db, const LIST& params, assigned_requirement* in)
{
    size_t base = GenericFill(db, params, static_cast<group_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to assigned_requirement"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to assigned_requirement to be a `SET [1:1] OF product_definition`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<compound_representation_item>(const DB& db, const LIST& params, compound_representation_item* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to compound_representation_item"); }    do { // convert the 'item_element' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::compound_representation_item, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->item_element, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to compound_representation_item to be a `compound_item_definition`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<atomic_formula>(const DB& db, const LIST& params, atomic_formula* in)
{
    size_t base = GenericFill(db, params, static_cast<compound_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to atomic_formula"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<attribute_assertion>(const DB& db, const LIST& params, attribute_assertion* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<attribute_language_assignment>(const DB& db, const LIST& params, attribute_language_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<attribute_classification_assignment*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to attribute_language_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to attribute_language_assignment to be a `SET [1:?] OF attribute_language_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<attribute_value_assignment>(const DB& db, const LIST& params, attribute_value_assignment* in)
{
    size_t base = 0;
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to attribute_value_assignment"); }    do { // convert the 'attribute_name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::attribute_value_assignment, 3>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->attribute_name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to attribute_value_assignment to be a `label`")); }
    } while (0);
    do { // convert the 'attribute_value' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::attribute_value_assignment, 3>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->attribute_value, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to attribute_value_assignment to be a `attribute_type`")); }
    } while (0);
    do { // convert the 'role' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::attribute_value_assignment, 3>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->role, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to attribute_value_assignment to be a `attribute_value_role`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<auxiliary_geometric_representation_item>(const DB& db, const LIST& params, auxiliary_geometric_representation_item* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<placement>(const DB& db, const LIST& params, placement* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to placement"); }    do { // convert the 'location' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::placement, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->location, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to placement to be a `cartesian_point`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<axis1_placement>(const DB& db, const LIST& params, axis1_placement* in)
{
    size_t base = GenericFill(db, params, static_cast<placement*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to axis1_placement"); }    do { // convert the 'axis' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->axis, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to axis1_placement to be a `direction`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<axis2_placement_2d>(const DB& db, const LIST& params, axis2_placement_2d* in)
{
    size_t base = GenericFill(db, params, static_cast<placement*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to axis2_placement_2d"); }    do { // convert the 'ref_direction' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->ref_direction, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to axis2_placement_2d to be a `direction`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<axis2_placement_3d>(const DB& db, const LIST& params, axis2_placement_3d* in)
{
    size_t base = GenericFill(db, params, static_cast<placement*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to axis2_placement_3d"); }    do { // convert the 'axis' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->axis, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to axis2_placement_3d to be a `direction`")); }
    } while (0);
    do { // convert the 'ref_direction' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->ref_direction, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to axis2_placement_3d to be a `direction`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<curve>(const DB& db, const LIST& params, curve* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to curve"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<bounded_curve>(const DB& db, const LIST& params, bounded_curve* in)
{
    size_t base = GenericFill(db, params, static_cast<curve*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to bounded_curve"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<b_spline_curve>(const DB& db, const LIST& params, b_spline_curve* in)
{
    size_t base = GenericFill(db, params, static_cast<bounded_curve*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to b_spline_curve"); }    do { // convert the 'degree' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::b_spline_curve, 5>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->degree, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to b_spline_curve to be a `INTEGER`")); }
    } while (0);
    do { // convert the 'control_points_list' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::b_spline_curve, 5>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->control_points_list, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to b_spline_curve to be a `LIST [2:?] OF cartesian_point`")); }
    } while (0);
    do { // convert the 'curve_form' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::b_spline_curve, 5>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->curve_form, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to b_spline_curve to be a `b_spline_curve_form`")); }
    } while (0);
    do { // convert the 'closed_curve' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::b_spline_curve, 5>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->closed_curve, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to b_spline_curve to be a `LOGICAL`")); }
    } while (0);
    do { // convert the 'self_intersect' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::b_spline_curve, 5>::aux_is_derived[4] = true; break; }
        try { GenericConvert(in->self_intersect, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to b_spline_curve to be a `LOGICAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<b_spline_curve_with_knots>(const DB& db, const LIST& params, b_spline_curve_with_knots* in)
{
    size_t base = GenericFill(db, params, static_cast<b_spline_curve*>(in));
    if (params.GetSize() < 9) { throw STEP::TypeError("expected 9 arguments to b_spline_curve_with_knots"); }    do { // convert the 'knot_multiplicities' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->knot_multiplicities, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to b_spline_curve_with_knots to be a `LIST [2:?] OF INTEGER`")); }
    } while (0);
    do { // convert the 'knots' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->knots, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to b_spline_curve_with_knots to be a `LIST [2:?] OF parameter_value`")); }
    } while (0);
    do { // convert the 'knot_spec' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->knot_spec, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 8 to b_spline_curve_with_knots to be a `knot_type`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<surface>(const DB& db, const LIST& params, surface* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to surface"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<bounded_surface>(const DB& db, const LIST& params, bounded_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<surface*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to bounded_surface"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<b_spline_surface>(const DB& db, const LIST& params, b_spline_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<bounded_surface*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to b_spline_surface"); }    do { // convert the 'u_degree' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::b_spline_surface, 6>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->u_degree, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to b_spline_surface to be a `INTEGER`")); }
    } while (0);
    do { // convert the 'v_degree' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::b_spline_surface, 6>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->v_degree, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to b_spline_surface to be a `INTEGER`")); }
    } while (0);
    do { // convert the 'surface_form' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::b_spline_surface, 6>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->surface_form, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to b_spline_surface to be a `b_spline_surface_form`")); }
    } while (0);
    do { // convert the 'u_closed' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::b_spline_surface, 6>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->u_closed, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to b_spline_surface to be a `LOGICAL`")); }
    } while (0);
    do { // convert the 'v_closed' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::b_spline_surface, 6>::aux_is_derived[4] = true; break; }
        try { GenericConvert(in->v_closed, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to b_spline_surface to be a `LOGICAL`")); }
    } while (0);
    do { // convert the 'self_intersect' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::b_spline_surface, 6>::aux_is_derived[5] = true; break; }
        try { GenericConvert(in->self_intersect, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 6 to b_spline_surface to be a `LOGICAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<b_spline_surface_with_knots>(const DB& db, const LIST& params, b_spline_surface_with_knots* in)
{
    size_t base = GenericFill(db, params, static_cast<b_spline_surface*>(in));
    if (params.GetSize() < 12) { throw STEP::TypeError("expected 12 arguments to b_spline_surface_with_knots"); }    do { // convert the 'u_multiplicities' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->u_multiplicities, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 7 to b_spline_surface_with_knots to be a `LIST [2:?] OF INTEGER`")); }
    } while (0);
    do { // convert the 'v_multiplicities' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->v_multiplicities, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 8 to b_spline_surface_with_knots to be a `LIST [2:?] OF INTEGER`")); }
    } while (0);
    do { // convert the 'u_knots' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->u_knots, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 9 to b_spline_surface_with_knots to be a `LIST [2:?] OF parameter_value`")); }
    } while (0);
    do { // convert the 'v_knots' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->v_knots, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 10 to b_spline_surface_with_knots to be a `LIST [2:?] OF parameter_value`")); }
    } while (0);
    do { // convert the 'knot_spec' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->knot_spec, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 11 to b_spline_surface_with_knots to be a `knot_type`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_definition>(const DB& db, const LIST& params, product_definition* in)
{
    size_t base = 0;
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to product_definition"); }    do { // convert the 'id' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_definition, 4>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->id, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to product_definition to be a `identifier`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_definition, 4>::aux_is_derived[1] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to product_definition to be a `text`")); }
    } while (0);
    do { // convert the 'formation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_definition, 4>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->formation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to product_definition to be a `product_definition_formation`")); }
    } while (0);
    do { // convert the 'frame_of_reference' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_definition, 4>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->frame_of_reference, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to product_definition to be a `product_definition_context`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<rule_software_definition>(const DB& db, const LIST& params, rule_software_definition* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to rule_software_definition"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<rule_definition>(const DB& db, const LIST& params, rule_definition* in)
{
    size_t base = GenericFill(db, params, static_cast<rule_software_definition*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to rule_definition"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<back_chaining_rule>(const DB& db, const LIST& params, back_chaining_rule* in)
{
    size_t base = GenericFill(db, params, static_cast<rule_definition*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to back_chaining_rule"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<back_chaining_rule_body>(const DB& db, const LIST& params, back_chaining_rule_body* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<colour>(const DB& db, const LIST& params, colour* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<background_colour>(const DB& db, const LIST& params, background_colour* in)
{
    size_t base = GenericFill(db, params, static_cast<colour*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to background_colour"); }    do { // convert the 'presentation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->presentation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to background_colour to be a `area_or_view`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<beveled_sheet_representation>(const DB& db, const LIST& params, beveled_sheet_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to beveled_sheet_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<bezier_curve>(const DB& db, const LIST& params, bezier_curve* in)
{
    size_t base = GenericFill(db, params, static_cast<b_spline_curve*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to bezier_curve"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<bezier_surface>(const DB& db, const LIST& params, bezier_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<b_spline_surface*>(in));
    if (params.GetSize() < 7) { throw STEP::TypeError("expected 7 arguments to bezier_surface"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<generic_expression>(const DB& db, const LIST& params, generic_expression* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<binary_generic_expression>(const DB& db, const LIST& params, binary_generic_expression* in)
{
    size_t base = GenericFill(db, params, static_cast<generic_expression*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to binary_generic_expression"); }    do { // convert the 'operands' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->operands, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to binary_generic_expression to be a `LIST [2:2] OF generic_expression`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<binary_numeric_expression>(const DB& db, const LIST& params, binary_numeric_expression* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<binary_representation_item>(const DB& db, const LIST& params, binary_representation_item* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to binary_representation_item"); }    do { // convert the 'binary_value' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::binary_representation_item, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->binary_value, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to binary_representation_item to be a `BINARY`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<block>(const DB& db, const LIST& params, block* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to block"); }    do { // convert the 'position' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->position, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to block to be a `axis2_placement_3d`")); }
    } while (0);
    do { // convert the 'x' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->x, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to block to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'y' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->y, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to block to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'z' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->z, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to block to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<expression>(const DB& db, const LIST& params, expression* in)
{
    size_t base = GenericFill(db, params, static_cast<generic_expression*>(in));
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<boolean_expression>(const DB& db, const LIST& params, boolean_expression* in)
{
    size_t base = GenericFill(db, params, static_cast<expression*>(in));
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<boolean_literal>(const DB& db, const LIST& params, boolean_literal* in)
{
    size_t base = 0;
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to boolean_literal"); }    do { // convert the 'the_value' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->the_value, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to boolean_literal to be a `BOOLEAN`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<boolean_representation_item>(const DB& db, const LIST& params, boolean_representation_item* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<boolean_result>(const DB& db, const LIST& params, boolean_result* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to boolean_result"); }    do { // convert the 'operator' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->operator_, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to boolean_result to be a `boolean_operator`")); }
    } while (0);
    do { // convert the 'first_operand' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->first_operand, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to boolean_result to be a `boolean_operand`")); }
    } while (0);
    do { // convert the 'second_operand' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->second_operand, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to boolean_result to be a `boolean_operand`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<composite_curve>(const DB& db, const LIST& params, composite_curve* in)
{
    size_t base = GenericFill(db, params, static_cast<bounded_curve*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to composite_curve"); }    do { // convert the 'segments' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::composite_curve, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->segments, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to composite_curve to be a `LIST [1:?] OF composite_curve_segment`")); }
    } while (0);
    do { // convert the 'self_intersect' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::composite_curve, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->self_intersect, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to composite_curve to be a `LOGICAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<composite_curve_on_surface>(const DB& db, const LIST& params, composite_curve_on_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<composite_curve*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to composite_curve_on_surface"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<boundary_curve>(const DB& db, const LIST& params, boundary_curve* in)
{
    size_t base = GenericFill(db, params, static_cast<composite_curve_on_surface*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to boundary_curve"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<bounded_pcurve>(const DB& db, const LIST& params, bounded_pcurve* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<bounded_surface_curve>(const DB& db, const LIST& params, bounded_surface_curve* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<founded_item>(const DB& db, const LIST& params, founded_item* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<box_domain>(const DB& db, const LIST& params, box_domain* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to box_domain"); }    do { // convert the 'corner' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->corner, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to box_domain to be a `cartesian_point`")); }
    } while (0);
    do { // convert the 'xlength' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->xlength, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to box_domain to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'ylength' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->ylength, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to box_domain to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'zlength' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->zlength, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to box_domain to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<half_space_solid>(const DB& db, const LIST& params, half_space_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to half_space_solid"); }    do { // convert the 'base_surface' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::half_space_solid, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->base_surface, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to half_space_solid to be a `surface`")); }
    } while (0);
    do { // convert the 'agreement_flag' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::half_space_solid, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->agreement_flag, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to half_space_solid to be a `BOOLEAN`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<boxed_half_space>(const DB& db, const LIST& params, boxed_half_space* in)
{
    size_t base = GenericFill(db, params, static_cast<half_space_solid*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to boxed_half_space"); }    do { // convert the 'enclosure' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->enclosure, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to boxed_half_space to be a `box_domain`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<breakdown_context>(const DB& db, const LIST& params, breakdown_context* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition_relationship*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to breakdown_context"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<breakdown_element_group_assignment>(const DB& db, const LIST& params, breakdown_element_group_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<group_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to breakdown_element_group_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to breakdown_element_group_assignment to be a `SET [1:1] OF product_definition_or_breakdown_element_usage`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<breakdown_element_realization>(const DB& db, const LIST& params, breakdown_element_realization* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<breakdown_element_usage>(const DB& db, const LIST& params, breakdown_element_usage* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition_relationship*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to breakdown_element_usage"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<breakdown_of>(const DB& db, const LIST& params, breakdown_of* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition_relationship*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to breakdown_of"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<solid_model>(const DB& db, const LIST& params, solid_model* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to solid_model"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<manifold_solid_brep>(const DB& db, const LIST& params, manifold_solid_brep* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_model*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to manifold_solid_brep"); }    do { // convert the 'outer' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::manifold_solid_brep, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->outer, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to manifold_solid_brep to be a `closed_shell`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<brep_with_voids>(const DB& db, const LIST& params, brep_with_voids* in)
{
    size_t base = GenericFill(db, params, static_cast<manifold_solid_brep*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to brep_with_voids"); }    do { // convert the 'voids' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->voids, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to brep_with_voids to be a `SET [1:?] OF oriented_closed_shell`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<bytes_representation_item>(const DB& db, const LIST& params, bytes_representation_item* in)
{
    size_t base = GenericFill(db, params, static_cast<binary_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to bytes_representation_item"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<date>(const DB& db, const LIST& params, date* in)
{
    size_t base = 0;
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to date"); }    do { // convert the 'year_component' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::date, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->year_component, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to date to be a `year_number`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<calendar_date>(const DB& db, const LIST& params, calendar_date* in)
{
    size_t base = GenericFill(db, params, static_cast<date*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to calendar_date"); }    do { // convert the 'day_component' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->day_component, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to calendar_date to be a `day_in_month_number`")); }
    } while (0);
    do { // convert the 'month_component' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->month_component, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to calendar_date to be a `month_in_year_number`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<camera_image>(const DB& db, const LIST& params, camera_image* in)
{
    size_t base = GenericFill(db, params, static_cast<mapped_item*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to camera_image"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<camera_image_3d_with_scale>(const DB& db, const LIST& params, camera_image_3d_with_scale* in)
{
    size_t base = GenericFill(db, params, static_cast<camera_image*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to camera_image_3d_with_scale"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<camera_model>(const DB& db, const LIST& params, camera_model* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to camera_model"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<camera_model_d3>(const DB& db, const LIST& params, camera_model_d3* in)
{
    size_t base = GenericFill(db, params, static_cast<camera_model*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to camera_model_d3"); }    do { // convert the 'view_reference_system' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::camera_model_d3, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->view_reference_system, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to camera_model_d3 to be a `axis2_placement_3d`")); }
    } while (0);
    do { // convert the 'perspective_of_volume' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::camera_model_d3, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->perspective_of_volume, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to camera_model_d3 to be a `view_volume`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<camera_model_d3_multi_clipping>(const DB& db, const LIST& params, camera_model_d3_multi_clipping* in)
{
    size_t base = GenericFill(db, params, static_cast<camera_model_d3*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to camera_model_d3_multi_clipping"); }    do { // convert the 'shape_clipping' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->shape_clipping, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to camera_model_d3_multi_clipping to be a `SET [1:?] OF camera_model_d3_multi_clipping_interection_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<camera_model_d3_multi_clipping_intersection>(const DB& db, const LIST& params, camera_model_d3_multi_clipping_intersection* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to camera_model_d3_multi_clipping_intersection"); }    do { // convert the 'shape_clipping' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->shape_clipping, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to camera_model_d3_multi_clipping_intersection to be a `SET [2:?] OF camera_model_d3_multi_clipping_interection_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<camera_model_d3_multi_clipping_union>(const DB& db, const LIST& params, camera_model_d3_multi_clipping_union* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to camera_model_d3_multi_clipping_union"); }    do { // convert the 'shape_clipping' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->shape_clipping, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to camera_model_d3_multi_clipping_union to be a `SET [2:?] OF camera_model_d3_multi_clipping_union_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<camera_model_d3_with_hlhsr>(const DB& db, const LIST& params, camera_model_d3_with_hlhsr* in)
{
    size_t base = GenericFill(db, params, static_cast<camera_model_d3*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to camera_model_d3_with_hlhsr"); }    do { // convert the 'hidden_line_surface_removal' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->hidden_line_surface_removal, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to camera_model_d3_with_hlhsr to be a `BOOLEAN`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<camera_model_with_light_sources>(const DB& db, const LIST& params, camera_model_with_light_sources* in)
{
    size_t base = GenericFill(db, params, static_cast<camera_model_d3*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to camera_model_with_light_sources"); }    do { // convert the 'sources' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->sources, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to camera_model_with_light_sources to be a `SET [1:?] OF light_source`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<representation_map>(const DB& db, const LIST& params, representation_map* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to representation_map"); }    do { // convert the 'mapping_origin' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::representation_map, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->mapping_origin, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to representation_map to be a `representation_item`")); }
    } while (0);
    do { // convert the 'mapped_representation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::representation_map, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->mapped_representation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to representation_map to be a `representation`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<camera_usage>(const DB& db, const LIST& params, camera_usage* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_map*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to camera_usage"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<capacitance_measure_with_unit>(const DB& db, const LIST& params, capacitance_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to capacitance_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<capacitance_unit>(const DB& db, const LIST& params, capacitance_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to capacitance_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<point>(const DB& db, const LIST& params, point* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to point"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<cartesian_point>(const DB& db, const LIST& params, cartesian_point* in)
{
    size_t base = GenericFill(db, params, static_cast<point*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to cartesian_point"); }    do { // convert the 'coordinates' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->coordinates, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to cartesian_point to be a `LIST [1:3] OF length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<cartesian_transformation_operator>(const DB& db, const LIST& params, cartesian_transformation_operator* in)
{
    size_t base = 0;
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to cartesian_transformation_operator"); }    do { // convert the 'axis1' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::cartesian_transformation_operator, 4>::aux_is_derived[0] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->axis1, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to cartesian_transformation_operator to be a `direction`")); }
    } while (0);
    do { // convert the 'axis2' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::cartesian_transformation_operator, 4>::aux_is_derived[1] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->axis2, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to cartesian_transformation_operator to be a `direction`")); }
    } while (0);
    do { // convert the 'local_origin' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::cartesian_transformation_operator, 4>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->local_origin, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to cartesian_transformation_operator to be a `cartesian_point`")); }
    } while (0);
    do { // convert the 'scale' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::cartesian_transformation_operator, 4>::aux_is_derived[3] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->scale, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to cartesian_transformation_operator to be a `REAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<cartesian_transformation_operator_2d>(const DB& db, const LIST& params, cartesian_transformation_operator_2d* in)
{
    size_t base = GenericFill(db, params, static_cast<cartesian_transformation_operator*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to cartesian_transformation_operator_2d"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<cartesian_transformation_operator_3d>(const DB& db, const LIST& params, cartesian_transformation_operator_3d* in)
{
    size_t base = GenericFill(db, params, static_cast<cartesian_transformation_operator*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to cartesian_transformation_operator_3d"); }    do { // convert the 'axis3' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->axis3, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to cartesian_transformation_operator_3d to be a `direction`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<cc_design_approval>(const DB& db, const LIST& params, cc_design_approval* in)
{
    size_t base = GenericFill(db, params, static_cast<approval_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to cc_design_approval"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to cc_design_approval to be a `SET [1:?] OF approved_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<cc_design_certification>(const DB& db, const LIST& params, cc_design_certification* in)
{
    size_t base = GenericFill(db, params, static_cast<certification_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to cc_design_certification"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to cc_design_certification to be a `SET [1:?] OF certified_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<cc_design_contract>(const DB& db, const LIST& params, cc_design_contract* in)
{
    size_t base = GenericFill(db, params, static_cast<contract_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to cc_design_contract"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to cc_design_contract to be a `SET [1:?] OF contracted_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<cc_design_date_and_time_assignment>(const DB& db, const LIST& params, cc_design_date_and_time_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<date_and_time_assignment*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to cc_design_date_and_time_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to cc_design_date_and_time_assignment to be a `SET [1:?] OF date_time_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<cc_design_person_and_organization_assignment>(const DB& db, const LIST& params, cc_design_person_and_organization_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<person_and_organization_assignment*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to cc_design_person_and_organization_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to cc_design_person_and_organization_assignment to be a `SET [1:?] OF cc_person_organization_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<cc_design_security_classification>(const DB& db, const LIST& params, cc_design_security_classification* in)
{
    size_t base = GenericFill(db, params, static_cast<security_classification_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to cc_design_security_classification"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to cc_design_security_classification to be a `SET [1:?] OF cc_classified_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<cc_design_specification_reference>(const DB& db, const LIST& params, cc_design_specification_reference* in)
{
    size_t base = GenericFill(db, params, static_cast<document_reference*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to cc_design_specification_reference"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to cc_design_specification_reference to be a `SET [1:?] OF cc_specified_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<celsius_temperature_measure_with_unit>(const DB& db, const LIST& params, celsius_temperature_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to celsius_temperature_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<centre_of_symmetry>(const DB& db, const LIST& params, centre_of_symmetry* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_shape_aspect*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to centre_of_symmetry"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<change>(const DB& db, const LIST& params, change* in)
{
    size_t base = GenericFill(db, params, static_cast<action_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to change"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to change to be a `SET [1:?] OF work_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<change_request>(const DB& db, const LIST& params, change_request* in)
{
    size_t base = GenericFill(db, params, static_cast<action_request_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to change_request"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to change_request to be a `SET [1:?] OF change_request_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<character_glyph_style_outline>(const DB& db, const LIST& params, character_glyph_style_outline* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to character_glyph_style_outline"); }    do { // convert the 'outline_style' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->outline_style, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to character_glyph_style_outline to be a `curve_style`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<character_glyph_style_stroke>(const DB& db, const LIST& params, character_glyph_style_stroke* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to character_glyph_style_stroke"); }    do { // convert the 'stroke_style' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->stroke_style, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to character_glyph_style_stroke to be a `curve_style`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<symbol_representation>(const DB& db, const LIST& params, symbol_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to symbol_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<generic_character_glyph_symbol>(const DB& db, const LIST& params, generic_character_glyph_symbol* in)
{
    size_t base = GenericFill(db, params, static_cast<symbol_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to generic_character_glyph_symbol"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<character_glyph_symbol>(const DB& db, const LIST& params, character_glyph_symbol* in)
{
    size_t base = GenericFill(db, params, static_cast<generic_character_glyph_symbol*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to character_glyph_symbol"); }    do { // convert the 'character_box' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::character_glyph_symbol, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->character_box, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to character_glyph_symbol to be a `planar_extent`")); }
    } while (0);
    do { // convert the 'baseline_ratio' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::character_glyph_symbol, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->baseline_ratio, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to character_glyph_symbol to be a `ratio_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<character_glyph_symbol_outline>(const DB& db, const LIST& params, character_glyph_symbol_outline* in)
{
    size_t base = GenericFill(db, params, static_cast<character_glyph_symbol*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to character_glyph_symbol_outline"); }    do { // convert the 'outlines' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->outlines, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to character_glyph_symbol_outline to be a `SET [1:?] OF annotation_fill_area`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<character_glyph_symbol_stroke>(const DB& db, const LIST& params, character_glyph_symbol_stroke* in)
{
    size_t base = GenericFill(db, params, static_cast<character_glyph_symbol*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to character_glyph_symbol_stroke"); }    do { // convert the 'strokes' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->strokes, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to character_glyph_symbol_stroke to be a `SET [1:?] OF curve`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<general_property>(const DB& db, const LIST& params, general_property* in)
{
    size_t base = 0;
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to general_property"); }    do { // convert the 'id' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::general_property, 3>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->id, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to general_property to be a `identifier`")); }
    } while (0);
    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::general_property, 3>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to general_property to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::general_property, 3>::aux_is_derived[2] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to general_property to be a `text`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<characteristic_data_column_header>(const DB& db, const LIST& params, characteristic_data_column_header* in)
{
    size_t base = GenericFill(db, params, static_cast<general_property*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to characteristic_data_column_header"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<general_property_relationship>(const DB& db, const LIST& params, general_property_relationship* in)
{
    size_t base = 0;
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to general_property_relationship"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::general_property_relationship, 4>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to general_property_relationship to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::general_property_relationship, 4>::aux_is_derived[1] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to general_property_relationship to be a `text`")); }
    } while (0);
    do { // convert the 'relating_property' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::general_property_relationship, 4>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->relating_property, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to general_property_relationship to be a `general_property`")); }
    } while (0);
    do { // convert the 'related_property' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::general_property_relationship, 4>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->related_property, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to general_property_relationship to be a `general_property`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<characteristic_data_column_header_link>(const DB& db, const LIST& params, characteristic_data_column_header_link* in)
{
    size_t base = GenericFill(db, params, static_cast<general_property_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to characteristic_data_column_header_link"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<characteristic_data_table_header>(const DB& db, const LIST& params, characteristic_data_table_header* in)
{
    size_t base = GenericFill(db, params, static_cast<general_property*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to characteristic_data_table_header"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<characteristic_data_table_header_decomposition>(const DB& db, const LIST& params, characteristic_data_table_header_decomposition* in)
{
    size_t base = GenericFill(db, params, static_cast<general_property_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to characteristic_data_table_header_decomposition"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<group>(const DB& db, const LIST& params, group* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to group"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::group, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to group to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::group, 2>::aux_is_derived[1] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to group to be a `text`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<characteristic_type>(const DB& db, const LIST& params, characteristic_type* in)
{
    size_t base = GenericFill(db, params, static_cast<group*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to characteristic_type"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<characterized_class>(const DB& db, const LIST& params, characterized_class* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<characterized_object>(const DB& db, const LIST& params, characterized_object* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to characterized_object"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::characterized_object, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to characterized_object to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::characterized_object, 2>::aux_is_derived[1] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to characterized_object to be a `text`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<conic>(const DB& db, const LIST& params, conic* in)
{
    size_t base = GenericFill(db, params, static_cast<curve*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to conic"); }    do { // convert the 'position' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::conic, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->position, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to conic to be a `axis2_placement`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<circle>(const DB& db, const LIST& params, circle* in)
{
    size_t base = GenericFill(db, params, static_cast<conic*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to circle"); }    do { // convert the 'radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to circle to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<circular_runout_tolerance>(const DB& db, const LIST& params, circular_runout_tolerance* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_tolerance_with_datum_reference*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to circular_runout_tolerance"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<class_by_extension>(const DB& db, const LIST& params, class_by_extension* in)
{
    size_t base = GenericFill(db, params, static_cast<class_t*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to class_by_extension"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<class_by_intension>(const DB& db, const LIST& params, class_by_intension* in)
{
    size_t base = GenericFill(db, params, static_cast<class_t*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to class_by_intension"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<class_system>(const DB& db, const LIST& params, class_system* in)
{
    size_t base = GenericFill(db, params, static_cast<group*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to class_system"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<effectivity_context_assignment>(const DB& db, const LIST& params, effectivity_context_assignment* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to effectivity_context_assignment"); }    do { // convert the 'assigned_effectivity_assignment' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::effectivity_context_assignment, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->assigned_effectivity_assignment, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to effectivity_context_assignment to be a `effectivity_assignment`")); }
    } while (0);
    do { // convert the 'role' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::effectivity_context_assignment, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->role, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to effectivity_context_assignment to be a `effectivity_context_role`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<class_usage_effectivity_context_assignment>(const DB& db, const LIST& params, class_usage_effectivity_context_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<effectivity_context_assignment*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to class_usage_effectivity_context_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to class_usage_effectivity_context_assignment to be a `SET [1:?] OF class_usage_effectivity_context_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<topological_representation_item>(const DB& db, const LIST& params, topological_representation_item* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_item*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to topological_representation_item"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<connected_face_set>(const DB& db, const LIST& params, connected_face_set* in)
{
    size_t base = GenericFill(db, params, static_cast<topological_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to connected_face_set"); }    do { // convert the 'cfs_faces' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::connected_face_set, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->cfs_faces, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to connected_face_set to be a `SET [1:?] OF face`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<closed_shell>(const DB& db, const LIST& params, closed_shell* in)
{
    size_t base = GenericFill(db, params, static_cast<connected_face_set*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to closed_shell"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<coaxiality_tolerance>(const DB& db, const LIST& params, coaxiality_tolerance* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_tolerance_with_datum_reference*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to coaxiality_tolerance"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<colour_specification>(const DB& db, const LIST& params, colour_specification* in)
{
    size_t base = GenericFill(db, params, static_cast<colour*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to colour_specification"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::colour_specification, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to colour_specification to be a `label`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<colour_rgb>(const DB& db, const LIST& params, colour_rgb* in)
{
    size_t base = GenericFill(db, params, static_cast<colour_specification*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to colour_rgb"); }    do { // convert the 'red' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->red, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to colour_rgb to be a `REAL`")); }
    } while (0);
    do { // convert the 'green' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->green, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to colour_rgb to be a `REAL`")); }
    } while (0);
    do { // convert the 'blue' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->blue, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to colour_rgb to be a `REAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<common_datum>(const DB& db, const LIST& params, common_datum* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<comparison_expression>(const DB& db, const LIST& params, comparison_expression* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<complex_clause>(const DB& db, const LIST& params, complex_clause* in)
{
    size_t base = GenericFill(db, params, static_cast<compound_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to complex_clause"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<complex_conjunctive_clause>(const DB& db, const LIST& params, complex_conjunctive_clause* in)
{
    size_t base = GenericFill(db, params, static_cast<complex_clause*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to complex_conjunctive_clause"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<complex_disjunctive_clause>(const DB& db, const LIST& params, complex_disjunctive_clause* in)
{
    size_t base = GenericFill(db, params, static_cast<complex_clause*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to complex_disjunctive_clause"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<modified_solid>(const DB& db, const LIST& params, modified_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_model*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to modified_solid"); }    do { // convert the 'rationale' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::modified_solid, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->rationale, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to modified_solid to be a `text`")); }
    } while (0);
    do { // convert the 'base_solid' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::modified_solid, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->base_solid, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to modified_solid to be a `base_solid_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<shelled_solid>(const DB& db, const LIST& params, shelled_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<modified_solid*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to shelled_solid"); }    do { // convert the 'deleted_face_set' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::shelled_solid, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->deleted_face_set, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to shelled_solid to be a `SET [1:?] OF face_surface`")); }
    } while (0);
    do { // convert the 'thickness' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::shelled_solid, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->thickness, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to shelled_solid to be a `length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<complex_shelled_solid>(const DB& db, const LIST& params, complex_shelled_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<shelled_solid*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to complex_shelled_solid"); }    do { // convert the 'thickness_list' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->thickness_list, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to complex_shelled_solid to be a `LIST [1:?] OF length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<composite_assembly_definition>(const DB& db, const LIST& params, composite_assembly_definition* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to composite_assembly_definition"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<composite_assembly_sequence_definition>(const DB& db, const LIST& params, composite_assembly_sequence_definition* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to composite_assembly_sequence_definition"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<laminate_table>(const DB& db, const LIST& params, laminate_table* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to laminate_table"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<part_laminate_table>(const DB& db, const LIST& params, part_laminate_table* in)
{
    size_t base = GenericFill(db, params, static_cast<laminate_table*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to part_laminate_table"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<composite_assembly_table>(const DB& db, const LIST& params, composite_assembly_table* in)
{
    size_t base = GenericFill(db, params, static_cast<part_laminate_table*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to composite_assembly_table"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<composite_curve_segment>(const DB& db, const LIST& params, composite_curve_segment* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to composite_curve_segment"); }    do { // convert the 'transition' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::composite_curve_segment, 3>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->transition, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to composite_curve_segment to be a `transition_code`")); }
    } while (0);
    do { // convert the 'same_sense' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::composite_curve_segment, 3>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->same_sense, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to composite_curve_segment to be a `BOOLEAN`")); }
    } while (0);
    do { // convert the 'parent_curve' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::composite_curve_segment, 3>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->parent_curve, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to composite_curve_segment to be a `curve`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<material_designation>(const DB& db, const LIST& params, material_designation* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to material_designation"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::material_designation, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to material_designation to be a `label`")); }
    } while (0);
    do { // convert the 'definitions' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::material_designation, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->definitions, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to material_designation to be a `SET [1:?] OF characterized_definition`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<composite_material_designation>(const DB& db, const LIST& params, composite_material_designation* in)
{
    size_t base = GenericFill(db, params, static_cast<material_designation*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to composite_material_designation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<composite_shape_aspect>(const DB& db, const LIST& params, composite_shape_aspect* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_aspect*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to composite_shape_aspect"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<composite_sheet_representation>(const DB& db, const LIST& params, composite_sheet_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to composite_sheet_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<composite_text>(const DB& db, const LIST& params, composite_text* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to composite_text"); }    do { // convert the 'collected_text' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::composite_text, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->collected_text, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to composite_text to be a `SET [2:?] OF text_or_character`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<composite_text_with_associated_curves>(const DB& db, const LIST& params, composite_text_with_associated_curves* in)
{
    size_t base = GenericFill(db, params, static_cast<composite_text*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to composite_text_with_associated_curves"); }    do { // convert the 'associated_curves' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->associated_curves, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to composite_text_with_associated_curves to be a `SET [1:?] OF curve`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<composite_text_with_blanking_box>(const DB& db, const LIST& params, composite_text_with_blanking_box* in)
{
    size_t base = GenericFill(db, params, static_cast<composite_text*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to composite_text_with_blanking_box"); }    do { // convert the 'blanking' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->blanking, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to composite_text_with_blanking_box to be a `planar_box`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<composite_text_with_delineation>(const DB& db, const LIST& params, composite_text_with_delineation* in)
{
    size_t base = GenericFill(db, params, static_cast<composite_text*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to composite_text_with_delineation"); }    do { // convert the 'delineation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->delineation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to composite_text_with_delineation to be a `text_delineation`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<composite_text_with_extent>(const DB& db, const LIST& params, composite_text_with_extent* in)
{
    size_t base = GenericFill(db, params, static_cast<composite_text*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to composite_text_with_extent"); }    do { // convert the 'extent' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->extent, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to composite_text_with_extent to be a `planar_extent`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<compound_shape_representation>(const DB& db, const LIST& params, compound_shape_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to compound_shape_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<concentricity_tolerance>(const DB& db, const LIST& params, concentricity_tolerance* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_tolerance_with_datum_reference*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to concentricity_tolerance"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<concept_feature_relationship>(const DB& db, const LIST& params, concept_feature_relationship* in)
{
    size_t base = 0;
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to concept_feature_relationship"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::concept_feature_relationship, 4>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to concept_feature_relationship to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::concept_feature_relationship, 4>::aux_is_derived[1] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to concept_feature_relationship to be a `text`")); }
    } while (0);
    do { // convert the 'relating_product_concept_feature' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::concept_feature_relationship, 4>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->relating_product_concept_feature, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to concept_feature_relationship to be a `product_concept_feature`")); }
    } while (0);
    do { // convert the 'related_product_concept_feature' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::concept_feature_relationship, 4>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->related_product_concept_feature, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to concept_feature_relationship to be a `product_concept_feature`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<concept_feature_relationship_with_condition>(const DB& db, const LIST& params, concept_feature_relationship_with_condition* in)
{
    size_t base = GenericFill(db, params, static_cast<concept_feature_relationship*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to concept_feature_relationship_with_condition"); }    do { // convert the 'conditional_operator' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->conditional_operator, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to concept_feature_relationship_with_condition to be a `concept_feature_operator`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_concept_feature>(const DB& db, const LIST& params, product_concept_feature* in)
{
    size_t base = 0;
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to product_concept_feature"); }    do { // convert the 'id' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_concept_feature, 3>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->id, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to product_concept_feature to be a `identifier`")); }
    } while (0);
    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_concept_feature, 3>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to product_concept_feature to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_concept_feature, 3>::aux_is_derived[2] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to product_concept_feature to be a `text`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<conditional_concept_feature>(const DB& db, const LIST& params, conditional_concept_feature* in)
{
    size_t base = GenericFill(db, params, static_cast<product_concept_feature*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to conditional_concept_feature"); }    do { // convert the 'condition' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::conditional_concept_feature, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->condition, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to conditional_concept_feature to be a `concept_feature_relationship_with_condition`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<conductance_measure_with_unit>(const DB& db, const LIST& params, conductance_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to conductance_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<conductance_unit>(const DB& db, const LIST& params, conductance_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<derived_unit*>(in));
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to conductance_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<configuration_item>(const DB& db, const LIST& params, configuration_item* in)
{
    size_t base = 0;
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to configuration_item"); }    do { // convert the 'id' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::configuration_item, 5>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->id, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to configuration_item to be a `identifier`")); }
    } while (0);
    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::configuration_item, 5>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to configuration_item to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::configuration_item, 5>::aux_is_derived[2] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to configuration_item to be a `text`")); }
    } while (0);
    do { // convert the 'item_concept' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::configuration_item, 5>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->item_concept, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to configuration_item to be a `product_concept`")); }
    } while (0);
    do { // convert the 'purpose' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::configuration_item, 5>::aux_is_derived[4] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->purpose, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to configuration_item to be a `label`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<configurable_item>(const DB& db, const LIST& params, configurable_item* in)
{
    size_t base = GenericFill(db, params, static_cast<configuration_item*>(in));
    if (params.GetSize() < 6) { throw STEP::TypeError("expected 6 arguments to configurable_item"); }    do { // convert the 'item_concept_feature' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->item_concept_feature, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 5 to configurable_item to be a `SET [1:?] OF product_concept_feature_association`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<effectivity>(const DB& db, const LIST& params, effectivity* in)
{
    size_t base = 0;
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to effectivity"); }    do { // convert the 'id' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::effectivity, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->id, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to effectivity to be a `identifier`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_definition_effectivity>(const DB& db, const LIST& params, product_definition_effectivity* in)
{
    size_t base = GenericFill(db, params, static_cast<effectivity*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to product_definition_effectivity"); }    do { // convert the 'usage' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_definition_effectivity, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->usage, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to product_definition_effectivity to be a `product_definition_relationship`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<configuration_effectivity>(const DB& db, const LIST& params, configuration_effectivity* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition_effectivity*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to configuration_effectivity"); }    do { // convert the 'configuration' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->configuration, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to configuration_effectivity to be a `configuration_design`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<configuration_item_relationship>(const DB& db, const LIST& params, configuration_item_relationship* in)
{
    size_t base = 0;
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to configuration_item_relationship"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::configuration_item_relationship, 4>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to configuration_item_relationship to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::configuration_item_relationship, 4>::aux_is_derived[1] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to configuration_item_relationship to be a `text`")); }
    } while (0);
    do { // convert the 'relating_configuration_item' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::configuration_item_relationship, 4>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->relating_configuration_item, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to configuration_item_relationship to be a `configuration_item`")); }
    } while (0);
    do { // convert the 'related_configuration_item' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::configuration_item_relationship, 4>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->related_configuration_item, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to configuration_item_relationship to be a `configuration_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<configuration_item_hierarchical_relationship>(const DB& db, const LIST& params, configuration_item_hierarchical_relationship* in)
{
    size_t base = GenericFill(db, params, static_cast<configuration_item_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to configuration_item_hierarchical_relationship"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<configuration_item_revision_sequence>(const DB& db, const LIST& params, configuration_item_revision_sequence* in)
{
    size_t base = GenericFill(db, params, static_cast<configuration_item_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to configuration_item_revision_sequence"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<configured_effectivity_assignment>(const DB& db, const LIST& params, configured_effectivity_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<effectivity_assignment*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to configured_effectivity_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to configured_effectivity_assignment to be a `SET [1:?] OF configured_effectivity_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<configured_effectivity_context_assignment>(const DB& db, const LIST& params, configured_effectivity_context_assignment* in)
{
    size_t base = GenericFill(db, params, static_cast<effectivity_context_assignment*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to configured_effectivity_context_assignment"); }    do { // convert the 'items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to configured_effectivity_context_assignment to be a `SET [1:?] OF configured_effectivity_context_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<conical_stepped_hole_transition>(const DB& db, const LIST& params, conical_stepped_hole_transition* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to conical_stepped_hole_transition"); }    do { // convert the 'transition_number' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->transition_number, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to conical_stepped_hole_transition to be a `positive_integer`")); }
    } while (0);
    do { // convert the 'cone_apex_angle' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->cone_apex_angle, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to conical_stepped_hole_transition to be a `plane_angle_measure`")); }
    } while (0);
    do { // convert the 'cone_base_radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->cone_base_radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to conical_stepped_hole_transition to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<elementary_surface>(const DB& db, const LIST& params, elementary_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<surface*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to elementary_surface"); }    do { // convert the 'position' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::elementary_surface, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->position, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to elementary_surface to be a `axis2_placement_3d`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<conical_surface>(const DB& db, const LIST& params, conical_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<elementary_surface*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to conical_surface"); }    do { // convert the 'radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to conical_surface to be a `length_measure`")); }
    } while (0);
    do { // convert the 'semi_angle' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->semi_angle, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to conical_surface to be a `plane_angle_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<connected_edge_set>(const DB& db, const LIST& params, connected_edge_set* in)
{
    size_t base = GenericFill(db, params, static_cast<topological_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to connected_edge_set"); }    do { // convert the 'ces_edges' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->ces_edges, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to connected_edge_set to be a `SET [1:?] OF edge`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<connected_face_sub_set>(const DB& db, const LIST& params, connected_face_sub_set* in)
{
    size_t base = GenericFill(db, params, static_cast<connected_face_set*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to connected_face_sub_set"); }    do { // convert the 'parent_face_set' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->parent_face_set, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to connected_face_sub_set to be a `connected_face_set`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<constructive_geometry_representation>(const DB& db, const LIST& params, constructive_geometry_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to constructive_geometry_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<representation_relationship>(const DB& db, const LIST& params, representation_relationship* in)
{
    size_t base = 0;
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to representation_relationship"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::representation_relationship, 4>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to representation_relationship to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::representation_relationship, 4>::aux_is_derived[1] = true; break; }
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to representation_relationship to be a `text`")); }
    } while (0);
    do { // convert the 'rep_1' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::representation_relationship, 4>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->rep_1, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to representation_relationship to be a `representation`")); }
    } while (0);
    do { // convert the 'rep_2' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::representation_relationship, 4>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->rep_2, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to representation_relationship to be a `representation`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<constructive_geometry_representation_relationship>(const DB& db, const LIST& params, constructive_geometry_representation_relationship* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to constructive_geometry_representation_relationship"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<contact_ratio_representation>(const DB& db, const LIST& params, contact_ratio_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to contact_ratio_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<invisibility>(const DB& db, const LIST& params, invisibility* in)
{
    size_t base = 0;
    if (params.GetSize() < 1) { throw STEP::TypeError("expected 1 arguments to invisibility"); }    do { // convert the 'invisible_items' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::invisibility, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->invisible_items, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to invisibility to be a `SET [1:?] OF invisible_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<context_dependent_invisibility>(const DB& db, const LIST& params, context_dependent_invisibility* in)
{
    size_t base = GenericFill(db, params, static_cast<invisibility*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to context_dependent_invisibility"); }    do { // convert the 'presentation_context' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->presentation_context, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to context_dependent_invisibility to be a `invisibility_context`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<over_riding_styled_item>(const DB& db, const LIST& params, over_riding_styled_item* in)
{
    size_t base = GenericFill(db, params, static_cast<styled_item*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to over_riding_styled_item"); }    do { // convert the 'over_ridden_style' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::over_riding_styled_item, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->over_ridden_style, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to over_riding_styled_item to be a `styled_item`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<context_dependent_over_riding_styled_item>(const DB& db, const LIST& params, context_dependent_over_riding_styled_item* in)
{
    size_t base = GenericFill(db, params, static_cast<over_riding_styled_item*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to context_dependent_over_riding_styled_item"); }    do { // convert the 'style_context' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::context_dependent_over_riding_styled_item, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->style_context, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to context_dependent_over_riding_styled_item to be a `LIST [1:?] OF style_context_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<context_dependent_unit>(const DB& db, const LIST& params, context_dependent_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<named_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to context_dependent_unit"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::context_dependent_unit, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to context_dependent_unit to be a `label`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<conversion_based_unit>(const DB& db, const LIST& params, conversion_based_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<named_unit*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to conversion_based_unit"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to conversion_based_unit to be a `label`")); }
    } while (0);
    do { // convert the 'conversion_factor' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->conversion_factor, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to conversion_based_unit to be a `measure_with_unit`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<csg_shape_representation>(const DB& db, const LIST& params, csg_shape_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to csg_shape_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<csg_solid>(const DB& db, const LIST& params, csg_solid* in)
{
    size_t base = GenericFill(db, params, static_cast<solid_model*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to csg_solid"); }    do { // convert the 'tree_root_expression' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->tree_root_expression, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to csg_solid to be a `csg_select`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<currency>(const DB& db, const LIST& params, currency* in)
{
    size_t base = GenericFill(db, params, static_cast<context_dependent_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to currency"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<currency_measure_with_unit>(const DB& db, const LIST& params, currency_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to currency_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<curve_bounded_surface>(const DB& db, const LIST& params, curve_bounded_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<bounded_surface*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to curve_bounded_surface"); }    do { // convert the 'basis_surface' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->basis_surface, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to curve_bounded_surface to be a `surface`")); }
    } while (0);
    do { // convert the 'boundaries' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->boundaries, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to curve_bounded_surface to be a `SET [1:?] OF boundary_curve`")); }
    } while (0);
    do { // convert the 'implicit_outer' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->implicit_outer, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to curve_bounded_surface to be a `BOOLEAN`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<curve_dimension>(const DB& db, const LIST& params, curve_dimension* in)
{
    size_t base = GenericFill(db, params, static_cast<dimension_curve_directed_callout*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to curve_dimension"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<curve_replica>(const DB& db, const LIST& params, curve_replica* in)
{
    size_t base = GenericFill(db, params, static_cast<curve*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to curve_replica"); }    do { // convert the 'parent_curve' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->parent_curve, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to curve_replica to be a `curve`")); }
    } while (0);
    do { // convert the 'transformation' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->transformation, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to curve_replica to be a `cartesian_transformation_operator`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<curve_style>(const DB& db, const LIST& params, curve_style* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to curve_style"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to curve_style to be a `label`")); }
    } while (0);
    do { // convert the 'curve_font' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->curve_font, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to curve_style to be a `curve_font_or_scaled_curve_font_select`")); }
    } while (0);
    do { // convert the 'curve_width' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->curve_width, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to curve_style to be a `size_select`")); }
    } while (0);
    do { // convert the 'curve_colour' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->curve_colour, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to curve_style to be a `colour`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<curve_style_font>(const DB& db, const LIST& params, curve_style_font* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to curve_style_font"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to curve_style_font to be a `label`")); }
    } while (0);
    do { // convert the 'pattern_list' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->pattern_list, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to curve_style_font to be a `LIST [1:?] OF curve_style_font_pattern`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<curve_style_font_and_scaling>(const DB& db, const LIST& params, curve_style_font_and_scaling* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to curve_style_font_and_scaling"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to curve_style_font_and_scaling to be a `label`")); }
    } while (0);
    do { // convert the 'curve_font' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->curve_font, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to curve_style_font_and_scaling to be a `curve_style_font_select`")); }
    } while (0);
    do { // convert the 'curve_font_scaling' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->curve_font_scaling, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to curve_style_font_and_scaling to be a `REAL`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<curve_style_font_pattern>(const DB& db, const LIST& params, curve_style_font_pattern* in)
{
    size_t base = GenericFill(db, params, static_cast<founded_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to curve_style_font_pattern"); }    do { // convert the 'visible_segment_length' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->visible_segment_length, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to curve_style_font_pattern to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'invisible_segment_length' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->invisible_segment_length, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to curve_style_font_pattern to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<curve_swept_solid_shape_representation>(const DB& db, const LIST& params, curve_swept_solid_shape_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to curve_swept_solid_shape_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<cylindrical_surface>(const DB& db, const LIST& params, cylindrical_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<elementary_surface*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to cylindrical_surface"); }    do { // convert the 'radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to cylindrical_surface to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<cylindricity_tolerance>(const DB& db, const LIST& params, cylindricity_tolerance* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_tolerance*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to cylindricity_tolerance"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<date_representation_item>(const DB& db, const LIST& params, date_representation_item* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<date_time_representation_item>(const DB& db, const LIST& params, date_time_representation_item* in)
{
    size_t base = 0;
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<dated_effectivity>(const DB& db, const LIST& params, dated_effectivity* in)
{
    size_t base = GenericFill(db, params, static_cast<effectivity*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to dated_effectivity"); }    do { // convert the 'effectivity_end_date' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const UNSET*>(&*arg)) break;
        try { GenericConvert(in->effectivity_end_date, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to dated_effectivity to be a `date_time_or_event_occurrence`")); }
    } while (0);
    do { // convert the 'effectivity_start_date' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->effectivity_start_date, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to dated_effectivity to be a `date_time_or_event_occurrence`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<datum>(const DB& db, const LIST& params, datum* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_aspect*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to datum"); }    do { // convert the 'identification' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->identification, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to datum to be a `identifier`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<datum_feature>(const DB& db, const LIST& params, datum_feature* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_aspect*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to datum_feature"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<datum_feature_callout>(const DB& db, const LIST& params, datum_feature_callout* in)
{
    size_t base = GenericFill(db, params, static_cast<draughting_callout*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to datum_feature_callout"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<datum_reference>(const DB& db, const LIST& params, datum_reference* in)
{
    size_t base = 0;
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to datum_reference"); }    do { // convert the 'precedence' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::datum_reference, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->precedence, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to datum_reference to be a `INTEGER`")); }
    } while (0);
    do { // convert the 'referenced_datum' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::datum_reference, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->referenced_datum, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to datum_reference to be a `datum`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<datum_target>(const DB& db, const LIST& params, datum_target* in)
{
    size_t base = GenericFill(db, params, static_cast<shape_aspect*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to datum_target"); }    do { // convert the 'target_id' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::datum_target, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->target_id, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to datum_target to be a `identifier`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<datum_target_callout>(const DB& db, const LIST& params, datum_target_callout* in)
{
    size_t base = GenericFill(db, params, static_cast<draughting_callout*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to datum_target_callout"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<default_tolerance_table>(const DB& db, const LIST& params, default_tolerance_table* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to default_tolerance_table"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<default_tolerance_table_cell>(const DB& db, const LIST& params, default_tolerance_table_cell* in)
{
    size_t base = GenericFill(db, params, static_cast<compound_representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to default_tolerance_table_cell"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<defined_symbol>(const DB& db, const LIST& params, defined_symbol* in)
{
    size_t base = GenericFill(db, params, static_cast<geometric_representation_item*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to defined_symbol"); }    do { // convert the 'definition' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->definition, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to defined_symbol to be a `defined_symbol_select`")); }
    } while (0);
    do { // convert the 'target' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->target, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to defined_symbol to be a `symbol_target`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<definitional_representation>(const DB& db, const LIST& params, definitional_representation* in)
{
    size_t base = GenericFill(db, params, static_cast<representation*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to definitional_representation"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<definitional_representation_relationship>(const DB& db, const LIST& params, definitional_representation_relationship* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to definitional_representation_relationship"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<definitional_representation_relationship_with_same_context>(const DB& db, const LIST& params, definitional_representation_relationship_with_same_context* in)
{
    size_t base = GenericFill(db, params, static_cast<definitional_representation_relationship*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to definitional_representation_relationship_with_same_context"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<degenerate_pcurve>(const DB& db, const LIST& params, degenerate_pcurve* in)
{
    size_t base = GenericFill(db, params, static_cast<point*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to degenerate_pcurve"); }    do { // convert the 'basis_surface' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::degenerate_pcurve, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->basis_surface, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to degenerate_pcurve to be a `surface`")); }
    } while (0);
    do { // convert the 'reference_to_curve' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::degenerate_pcurve, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->reference_to_curve, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to degenerate_pcurve to be a `definitional_representation`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<toroidal_surface>(const DB& db, const LIST& params, toroidal_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<elementary_surface*>(in));
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to toroidal_surface"); }    do { // convert the 'major_radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::toroidal_surface, 2>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->major_radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to toroidal_surface to be a `positive_length_measure`")); }
    } while (0);
    do { // convert the 'minor_radius' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::toroidal_surface, 2>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->minor_radius, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to toroidal_surface to be a `positive_length_measure`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<degenerate_toroidal_surface>(const DB& db, const LIST& params, degenerate_toroidal_surface* in)
{
    size_t base = GenericFill(db, params, static_cast<toroidal_surface*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to degenerate_toroidal_surface"); }    do { // convert the 'select_outer' argument
        std::shared_ptr<const DataType> arg = params[base++];
        try { GenericConvert(in->select_outer, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 4 to degenerate_toroidal_surface to be a `BOOLEAN`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<descriptive_representation_item>(const DB& db, const LIST& params, descriptive_representation_item* in)
{
    size_t base = GenericFill(db, params, static_cast<representation_item*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to descriptive_representation_item"); }    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::descriptive_representation_item, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to descriptive_representation_item to be a `text`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<product_definition_context>(const DB& db, const LIST& params, product_definition_context* in)
{
    size_t base = GenericFill(db, params, static_cast<application_context_element*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to product_definition_context"); }    do { // convert the 'life_cycle_stage' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::product_definition_context, 1>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->life_cycle_stage, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to product_definition_context to be a `label`")); }
    } while (0);
    return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<design_context>(const DB& db, const LIST& params, design_context* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition_context*>(in));
    if (params.GetSize() < 3) { throw STEP::TypeError("expected 3 arguments to design_context"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<design_make_from_relationship>(const DB& db, const LIST& params, design_make_from_relationship* in)
{
    size_t base = GenericFill(db, params, static_cast<product_definition_relationship*>(in));
    if (params.GetSize() < 5) { throw STEP::TypeError("expected 5 arguments to design_make_from_relationship"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<diameter_dimension>(const DB& db, const LIST& params, diameter_dimension* in)
{
    size_t base = GenericFill(db, params, static_cast<dimension_curve_directed_callout*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to diameter_dimension"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<ratio_measure_with_unit>(const DB& db, const LIST& params, ratio_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to ratio_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<dielectric_constant_measure_with_unit>(const DB& db, const LIST& params, dielectric_constant_measure_with_unit* in)
{
    size_t base = GenericFill(db, params, static_cast<ratio_measure_with_unit*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to dielectric_constant_measure_with_unit"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<dimension_callout>(const DB& db, const LIST& params, dimension_callout* in)
{
    size_t base = GenericFill(db, params, static_cast<draughting_callout*>(in));
    if (params.GetSize() < 2) { throw STEP::TypeError("expected 2 arguments to dimension_callout"); }	return base;
}
// -----------------------------------------------------------------------------------------------------------
template <> size_t GenericFill<draughting_callout_relationship>(const DB& db, const LIST& params, draughting_callout_relationship* in)
{
    size_t base = 0;
    if (params.GetSize() < 4) { throw STEP::TypeError("expected 4 arguments to draughting_callout_relationship"); }    do { // convert the 'name' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::draughting_callout_relationship, 4>::aux_is_derived[0] = true; break; }
        try { GenericConvert(in->name, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 0 to draughting_callout_relationship to be a `label`")); }
    } while (0);
    do { // convert the 'description' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::draughting_callout_relationship, 4>::aux_is_derived[1] = true; break; }
        try { GenericConvert(in->description, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 1 to draughting_callout_relationship to be a `text`")); }
    } while (0);
    do { // convert the 'relating_draughting_callout' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::draughting_callout_relationship, 4>::aux_is_derived[2] = true; break; }
        try { GenericConvert(in->relating_draughting_callout, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 2 to draughting_callout_relationship to be a `draughting_callout`")); }
    } while (0);
    do { // convert the 'related_draughting_callout' argument
        std::shared_ptr<const DataType> arg = params[base++];
        if (dynamic_cast<const ISDERIVED*>(&*arg)) { in->ObjectHelper<Assimp::StepFile::draughting_callout_relationship, 4>::aux_is_derived[3] = true; break; }
        try { GenericConvert(in->related_draughting_callout, arg, db); break; }
        catch (const TypeError& t) { throw TypeError(t.what() + std::string(" - expected argument 3 to draughting_callout_relationship to be a `draughting_callout`")); }
    } while (0);
    return base;
}

}
}
