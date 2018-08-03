/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2017, assimp team

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

/** @file  BlenderDNA.inl
 *  @brief Blender `DNA` (file format specification embedded in
 *    blend file itself) loader.
 */
#ifndef INCLUDED_AI_BLEND_DNA_INL
#define INCLUDED_AI_BLEND_DNA_INL

#include <memory>
#include "TinyFormatter.h"

namespace Assimp {
namespace Blender {

//--------------------------------------------------------------------------------
const Field& Structure :: operator [] (const std::string& ss) const
{
    std::map<std::string, size_t>::const_iterator it = indices.find(ss);
    if (it == indices.end()) {
        throw Error((Formatter::format(),
            "BlendDNA: Did not find a field named `",ss,"` in structure `",name,"`"
            ));
    }

    return fields[(*it).second];
}

//--------------------------------------------------------------------------------
const Field* Structure :: Get (const std::string& ss) const
{
    std::map<std::string, size_t>::const_iterator it = indices.find(ss);
    return it == indices.end() ? NULL : &fields[(*it).second];
}

//--------------------------------------------------------------------------------
const Field& Structure :: operator [] (const size_t i) const
{
    if (i >= fields.size()) {
        throw Error((Formatter::format(),
            "BlendDNA: There is no field with index `",i,"` in structure `",name,"`"
            ));
    }

    return fields[i];
}

//--------------------------------------------------------------------------------
template <typename T> std::shared_ptr<ElemBase> Structure :: Allocate() const
{
    return std::shared_ptr<T>(new T());
}

//--------------------------------------------------------------------------------
template <typename T> void Structure :: Convert(
    std::shared_ptr<ElemBase> in,
    const FileDatabase& db) const
{
    Convert<T> (*static_cast<T*> ( in.get() ),db);
}

//--------------------------------------------------------------------------------
template <int error_policy, typename T, size_t M>
void Structure :: ReadFieldArray(T (& out)[M], const char* name, const FileDatabase& db) const
{
    const StreamReaderAny::pos old = db.reader->GetCurrentPos();
    try {
        const Field& f = (*this)[name];
        const Structure& s = db.dna[f.type];

        // is the input actually an array?
        if (!(f.flags & FieldFlag_Array)) {
            throw Error((Formatter::format(),"Field `",name,"` of structure `",
                this->name,"` ought to be an array of size ",M
                ));
        }

        db.reader->IncPtr(f.offset);

        // size conversions are always allowed, regardless of error_policy
        unsigned int i = 0;
        for(; i < std::min(f.array_sizes[0],M); ++i) {
            s.Convert(out[i],db);
        }
        for(; i < M; ++i) {
            _defaultInitializer<ErrorPolicy_Igno>()(out[i]);
        }
    }
    catch (const Error& e) {
        _defaultInitializer<error_policy>()(out,e.what());
    }

    // and recover the previous stream position
    db.reader->SetCurrentPos(old);

#ifndef ASSIMP_BUILD_BLENDER_NO_STATS
    ++db.stats().fields_read;
#endif
}

//--------------------------------------------------------------------------------
template <int error_policy, typename T, size_t M, size_t N>
void Structure :: ReadFieldArray2(T (& out)[M][N], const char* name, const FileDatabase& db) const
{
    const StreamReaderAny::pos old = db.reader->GetCurrentPos();
    try {
        const Field& f = (*this)[name];
        const Structure& s = db.dna[f.type];

        // is the input actually an array?
        if (!(f.flags & FieldFlag_Array)) {
            throw Error((Formatter::format(),"Field `",name,"` of structure `",
                this->name,"` ought to be an array of size ",M,"*",N
                ));
        }

        db.reader->IncPtr(f.offset);

        // size conversions are always allowed, regardless of error_policy
        unsigned int i = 0;
        for(; i < std::min(f.array_sizes[0],M); ++i) {
            unsigned int j = 0;
            for(; j < std::min(f.array_sizes[1],N); ++j) {
                s.Convert(out[i][j],db);
            }
            for(; j < N; ++j) {
                _defaultInitializer<ErrorPolicy_Igno>()(out[i][j]);
            }
        }
        for(; i < M; ++i) {
            _defaultInitializer<ErrorPolicy_Igno>()(out[i]);
        }
    }
    catch (const Error& e) {
        _defaultInitializer<error_policy>()(out,e.what());
    }

    // and recover the previous stream position
    db.reader->SetCurrentPos(old);

#ifndef ASSIMP_BUILD_BLENDER_NO_STATS
    ++db.stats().fields_read;
#endif
}

//--------------------------------------------------------------------------------
template <int error_policy, template <typename> class TOUT, typename T>
bool Structure :: ReadFieldPtr(TOUT<T>& out, const char* name, const FileDatabase& db,
    bool non_recursive /*= false*/) const
{
    const StreamReaderAny::pos old = db.reader->GetCurrentPos();
    Pointer ptrval;
    const Field* f;
    try {
        f = &(*this)[name];

        // sanity check, should never happen if the genblenddna script is right
        if (!(f->flags & FieldFlag_Pointer)) {
            throw Error((Formatter::format(),"Field `",name,"` of structure `",
                this->name,"` ought to be a pointer"));
        }

        db.reader->IncPtr(f->offset);
        Convert(ptrval,db);
        // actually it is meaningless on which Structure the Convert is called
        // because the `Pointer` argument triggers a special implementation.
    }
    catch (const Error& e) {
        _defaultInitializer<error_policy>()(out,e.what());

        out.reset();
        return false;
    }

    // resolve the pointer and load the corresponding structure
    const bool res = ResolvePointer(out,ptrval,db,*f, non_recursive);

    if(!non_recursive) {
        // and recover the previous stream position
        db.reader->SetCurrentPos(old);
    }

#ifndef ASSIMP_BUILD_BLENDER_NO_STATS
    ++db.stats().fields_read;
#endif

    return res;
}

//--------------------------------------------------------------------------------
template <int error_policy, template <typename> class TOUT, typename T, size_t N>
bool Structure :: ReadFieldPtr(TOUT<T> (&out)[N], const char* name,
    const FileDatabase& db) const
{
    // XXX see if we can reduce this to call to the 'normal' ReadFieldPtr
    const StreamReaderAny::pos old = db.reader->GetCurrentPos();
    Pointer ptrval[N];
    const Field* f;
    try {
        f = &(*this)[name];

        // sanity check, should never happen if the genblenddna script is right
        if ((FieldFlag_Pointer|FieldFlag_Pointer) != (f->flags & (FieldFlag_Pointer|FieldFlag_Pointer))) {
            throw Error((Formatter::format(),"Field `",name,"` of structure `",
                this->name,"` ought to be a pointer AND an array"));
        }

        db.reader->IncPtr(f->offset);

        size_t i = 0;
        for(; i < std::min(f->array_sizes[0],N); ++i) {
            Convert(ptrval[i],db);
        }
        for(; i < N; ++i) {
            _defaultInitializer<ErrorPolicy_Igno>()(ptrval[i]);
        }

        // actually it is meaningless on which Structure the Convert is called
        // because the `Pointer` argument triggers a special implementation.
    }
    catch (const Error& e) {
        _defaultInitializer<error_policy>()(out,e.what());
        for(size_t i = 0; i < N; ++i) {
            out[i].reset();
        }
        return false;
    }

    bool res = true;
    for(size_t i = 0; i < N; ++i) {
        // resolve the pointer and load the corresponding structure
        res = ResolvePointer(out[i],ptrval[i],db,*f) && res;
    }

    // and recover the previous stream position
    db.reader->SetCurrentPos(old);

#ifndef ASSIMP_BUILD_BLENDER_NO_STATS
    ++db.stats().fields_read;
#endif
    return res;
}

//--------------------------------------------------------------------------------
template <int error_policy, typename T>
void Structure :: ReadField(T& out, const char* name, const FileDatabase& db) const
{
    const StreamReaderAny::pos old = db.reader->GetCurrentPos();
    try {
        const Field& f = (*this)[name];
        // find the structure definition pertaining to this field
        const Structure& s = db.dna[f.type];

        db.reader->IncPtr(f.offset);
        s.Convert(out,db);
    }
    catch (const Error& e) {
        _defaultInitializer<error_policy>()(out,e.what());
    }

    // and recover the previous stream position
    db.reader->SetCurrentPos(old);

#ifndef ASSIMP_BUILD_BLENDER_NO_STATS
    ++db.stats().fields_read;
#endif
}


//--------------------------------------------------------------------------------
template <template <typename> class TOUT, typename T>
bool Structure :: ResolvePointer(TOUT<T>& out, const Pointer & ptrval, const FileDatabase& db,
    const Field& f,
    bool non_recursive /*= false*/) const
{
    out.reset(); // ensure null pointers work
    if (!ptrval.val) {
        return false;
    }
    const Structure& s = db.dna[f.type];
    // find the file block the pointer is pointing to
    const FileBlockHead* block = LocateFileBlockForAddress(ptrval,db);

    // also determine the target type from the block header
    // and check if it matches the type which we expect.
    const Structure& ss = db.dna[block->dna_index];
    if (ss != s) {
        throw Error((Formatter::format(),"Expected target to be of type `",s.name,
            "` but seemingly it is a `",ss.name,"` instead"
            ));
    }

    // try to retrieve the object from the cache
    db.cache(out).get(s,out,ptrval);
    if (out) {
        return true;
    }

    // seek to this location, but save the previous stream pointer.
    const StreamReaderAny::pos pold = db.reader->GetCurrentPos();
    db.reader->SetCurrentPos(block->start+ static_cast<size_t>((ptrval.val - block->address.val) ));
    // FIXME: basically, this could cause problems with 64 bit pointers on 32 bit systems.
    // I really ought to improve StreamReader to work with 64 bit indices exclusively.

    // continue conversion after allocating the required storage
    size_t num = block->size / ss.size;
    T* o = _allocate(out,num);

    // cache the object before we convert it to avoid cyclic recursion.
    db.cache(out).set(s,out,ptrval);

    // if the non_recursive flag is set, we don't do anything but leave
    // the cursor at the correct position to resolve the object.
    if (!non_recursive) {
        for (size_t i = 0; i < num; ++i,++o) {
            s.Convert(*o,db);
        }

        db.reader->SetCurrentPos(pold);
    }

#ifndef ASSIMP_BUILD_BLENDER_NO_STATS
    if(out) {
        ++db.stats().pointers_resolved;
    }
#endif
    return false;
}


//--------------------------------------------------------------------------------
inline bool Structure :: ResolvePointer( std::shared_ptr< FileOffset >& out, const Pointer & ptrval,
    const FileDatabase& db,
    const Field&,
    bool) const
{
    // Currently used exclusively by PackedFile::data to represent
    // a simple offset into the mapped BLEND file.
    out.reset();
    if (!ptrval.val) {
        return false;
    }

    // find the file block the pointer is pointing to
    const FileBlockHead* block = LocateFileBlockForAddress(ptrval,db);

    out =  std::shared_ptr< FileOffset > (new FileOffset());
    out->val = block->start+ static_cast<size_t>((ptrval.val - block->address.val) );
    return false;
}

//--------------------------------------------------------------------------------
template <template <typename> class TOUT, typename T>
bool Structure :: ResolvePointer(vector< TOUT<T> >& out, const Pointer & ptrval,
    const FileDatabase& db,
    const Field& f,
    bool) const
{
    // This is a function overload, not a template specialization. According to
    // the partial ordering rules, it should be selected by the compiler
    // for array-of-pointer inputs, i.e. Object::mats.

    out.reset();
    if (!ptrval.val) {
        return false;
    }

    // find the file block the pointer is pointing to
    const FileBlockHead* block = LocateFileBlockForAddress(ptrval,db);
    const size_t num = block->size / (db.i64bit?8:4);

    // keep the old stream position
    const StreamReaderAny::pos pold = db.reader->GetCurrentPos();
    db.reader->SetCurrentPos(block->start+ static_cast<size_t>((ptrval.val - block->address.val) ));

    bool res = false;
    // allocate raw storage for the array
    out.resize(num);
    for (size_t i = 0; i< num; ++i) {
        Pointer val;
        Convert(val,db);

        // and resolve the pointees
        res = ResolvePointer(out[i],val,db,f) && res;
    }

    db.reader->SetCurrentPos(pold);
    return res;
}

//--------------------------------------------------------------------------------
template <> bool Structure :: ResolvePointer<std::shared_ptr,ElemBase>(std::shared_ptr<ElemBase>& out,
    const Pointer & ptrval,
    const FileDatabase& db,
    const Field&,
    bool
) const
{
    // Special case when the data type needs to be determined at runtime.
    // Less secure than in the `strongly-typed` case.

    out.reset();
    if (!ptrval.val) {
        return false;
    }

    // find the file block the pointer is pointing to
    const FileBlockHead* block = LocateFileBlockForAddress(ptrval,db);

    // determine the target type from the block header
    const Structure& s = db.dna[block->dna_index];

    // try to retrieve the object from the cache
    db.cache(out).get(s,out,ptrval);
    if (out) {
        return true;
    }

    // seek to this location, but save the previous stream pointer.
    const StreamReaderAny::pos pold = db.reader->GetCurrentPos();
    db.reader->SetCurrentPos(block->start+ static_cast<size_t>((ptrval.val - block->address.val) ));
    // FIXME: basically, this could cause problems with 64 bit pointers on 32 bit systems.
    // I really ought to improve StreamReader to work with 64 bit indices exclusively.

    // continue conversion after allocating the required storage
    DNA::FactoryPair builders = db.dna.GetBlobToStructureConverter(s,db);
    if (!builders.first) {
        // this might happen if DNA::RegisterConverters hasn't been called so far
        // or if the target type is not contained in `our` DNA.
        out.reset();
        DefaultLogger::get()->warn((Formatter::format(),
            "Failed to find a converter for the `",s.name,"` structure"
            ));
        return false;
    }

    // allocate the object hull
    out = (s.*builders.first)();

    // cache the object immediately to prevent infinite recursion in a
    // circular list with a single element (i.e. a self-referencing element).
    db.cache(out).set(s,out,ptrval);

    // and do the actual conversion
    (s.*builders.second)(out,db);
    db.reader->SetCurrentPos(pold);

    // store a pointer to the name string of the actual type
    // in the object itself. This allows the conversion code
    // to perform additional type checking.
    out->dna_type = s.name.c_str();


#ifndef ASSIMP_BUILD_BLENDER_NO_STATS
    ++db.stats().pointers_resolved;
#endif
    return false;
}

//--------------------------------------------------------------------------------
const FileBlockHead* Structure :: LocateFileBlockForAddress(const Pointer & ptrval, const FileDatabase& db) const
{
    // the file blocks appear in list sorted by
    // with ascending base addresses so we can run a
    // binary search to locate the pointee quickly.

    // NOTE: Blender seems to distinguish between side-by-side
    // data (stored in the same data block) and far pointers,
    // which are only used for structures starting with an ID.
    // We don't need to make this distinction, our algorithm
    // works regardless where the data is stored.
    vector<FileBlockHead>::const_iterator it = std::lower_bound(db.entries.begin(),db.entries.end(),ptrval);
    if (it == db.entries.end()) {
        // this is crucial, pointers may not be invalid.
        // this is either a corrupted file or an attempted attack.
        throw DeadlyImportError((Formatter::format(),"Failure resolving pointer 0x",
            std::hex,ptrval.val,", no file block falls into this address range"
            ));
    }
    if (ptrval.val >= (*it).address.val + (*it).size) {
        throw DeadlyImportError((Formatter::format(),"Failure resolving pointer 0x",
            std::hex,ptrval.val,", nearest file block starting at 0x",
            (*it).address.val," ends at 0x",
            (*it).address.val + (*it).size
            ));
    }
    return &*it;
}

// ------------------------------------------------------------------------------------------------
// NOTE: The MSVC debugger keeps showing up this annoying `a cast to a smaller data type has
// caused a loss of data`-warning. Avoid this warning by a masking with an appropriate bitmask.

template <typename T> struct signless;
template <> struct signless<char> {typedef unsigned char type;};
template <> struct signless<short> {typedef unsigned short type;};
template <> struct signless<int> {typedef unsigned int type;};
template <> struct signless<unsigned char> { typedef unsigned char type; };
template <typename T>
struct static_cast_silent {
    template <typename V>
    T operator()(V in) {
        return static_cast<T>(in & static_cast<typename signless<T>::type>(-1));
    }
};

template <> struct static_cast_silent<float> {
    template <typename V> float  operator()(V in) {
        return static_cast<float> (in);
    }
};

template <> struct static_cast_silent<double> {
    template <typename V> double operator()(V in) {
        return static_cast<double>(in);
    }
};

// ------------------------------------------------------------------------------------------------
template <typename T> inline void ConvertDispatcher(T& out, const Structure& in,const FileDatabase& db)
{
    if (in.name == "int") {
        out = static_cast_silent<T>()(db.reader->GetU4());
    }
    else if (in.name == "short") {
        out = static_cast_silent<T>()(db.reader->GetU2());
    }
    else if (in.name == "char") {
        out = static_cast_silent<T>()(db.reader->GetU1());
    }
    else if (in.name == "float") {
        out = static_cast<T>(db.reader->GetF4());
    }
    else if (in.name == "double") {
        out = static_cast<T>(db.reader->GetF8());
    }
    else {
        throw DeadlyImportError("Unknown source for conversion to primitive data type: "+in.name);
    }
}

// ------------------------------------------------------------------------------------------------
template <> inline void Structure :: Convert<int>    (int& dest,const FileDatabase& db) const
{
    ConvertDispatcher(dest,*this,db);
}

// ------------------------------------------------------------------------------------------------
template<> inline void Structure :: Convert<short>  (short& dest,const FileDatabase& db) const
{
    // automatic rescaling from short to float and vice versa (seems to be used by normals)
    if (name == "float") {
        float f = db.reader->GetF4();
        if ( f > 1.0f )
            f = 1.0f;
        dest = static_cast<short>( f * 32767.f);
        //db.reader->IncPtr(-4);
        return;
    }
    else if (name == "double") {
        dest = static_cast<short>(db.reader->GetF8() * 32767.);
        //db.reader->IncPtr(-8);
        return;
    }
    ConvertDispatcher(dest,*this,db);
}

// ------------------------------------------------------------------------------------------------
template <> inline void Structure :: Convert<char>   (char& dest,const FileDatabase& db) const
{
    // automatic rescaling from char to float and vice versa (seems useful for RGB colors)
    if (name == "float") {
        dest = static_cast<char>(db.reader->GetF4() * 255.f);
        return;
    }
    else if (name == "double") {
        dest = static_cast<char>(db.reader->GetF8() * 255.f);
        return;
    }
    ConvertDispatcher(dest,*this,db);
}

// ------------------------------------------------------------------------------------------------
template <> inline void Structure::Convert<unsigned char>(unsigned char& dest, const FileDatabase& db) const
{
	// automatic rescaling from char to float and vice versa (seems useful for RGB colors)
	if (name == "float") {
		dest = static_cast<unsigned char>(db.reader->GetF4() * 255.f);
		return;
	}
	else if (name == "double") {
		dest = static_cast<unsigned char>(db.reader->GetF8() * 255.f);
		return;
	}
	ConvertDispatcher(dest, *this, db);
}


// ------------------------------------------------------------------------------------------------
template <> inline void Structure :: Convert<float>  (float& dest,const FileDatabase& db) const
{
    // automatic rescaling from char to float and vice versa (seems useful for RGB colors)
    if (name == "char") {
        dest = db.reader->GetI1() / 255.f;
        return;
    }
    // automatic rescaling from short to float and vice versa (used by normals)
    else if (name == "short") {
        dest = db.reader->GetI2() / 32767.f;
        return;
    }
    ConvertDispatcher(dest,*this,db);
}

// ------------------------------------------------------------------------------------------------
template <> inline void Structure :: Convert<double> (double& dest,const FileDatabase& db) const
{
    if (name == "char") {
        dest = db.reader->GetI1() / 255.;
        return;
    }
    else if (name == "short") {
        dest = db.reader->GetI2() / 32767.;
        return;
    }
    ConvertDispatcher(dest,*this,db);
}

// ------------------------------------------------------------------------------------------------
template <> inline void Structure :: Convert<Pointer> (Pointer& dest,const FileDatabase& db) const
{
    if (db.i64bit) {
        dest.val = db.reader->GetU8();
        //db.reader->IncPtr(-8);
        return;
    }
    dest.val = db.reader->GetU4();
    //db.reader->IncPtr(-4);
}

//--------------------------------------------------------------------------------
const Structure& DNA :: operator [] (const std::string& ss) const
{
    std::map<std::string, size_t>::const_iterator it = indices.find(ss);
    if (it == indices.end()) {
        throw Error((Formatter::format(),
            "BlendDNA: Did not find a structure named `",ss,"`"
            ));
    }

    return structures[(*it).second];
}

//--------------------------------------------------------------------------------
const Structure* DNA :: Get (const std::string& ss) const
{
    std::map<std::string, size_t>::const_iterator it = indices.find(ss);
    return it == indices.end() ? NULL : &structures[(*it).second];
}

//--------------------------------------------------------------------------------
const Structure& DNA :: operator [] (const size_t i) const
{
    if (i >= structures.size()) {
        throw Error((Formatter::format(),
            "BlendDNA: There is no structure with index `",i,"`"
            ));
    }

    return structures[i];
}

//--------------------------------------------------------------------------------
template <template <typename> class TOUT> template <typename T> void ObjectCache<TOUT> :: get (
    const Structure& s,
    TOUT<T>& out,
    const Pointer& ptr
) const {

    if(s.cache_idx == static_cast<size_t>(-1)) {
        s.cache_idx = db.next_cache_idx++;
        caches.resize(db.next_cache_idx);
        return;
    }

    typename StructureCache::const_iterator it = caches[s.cache_idx].find(ptr);
    if (it != caches[s.cache_idx].end()) {
        out = std::static_pointer_cast<T>( (*it).second );

#ifndef ASSIMP_BUILD_BLENDER_NO_STATS
        ++db.stats().cache_hits;
#endif
    }
    // otherwise, out remains untouched
}


//--------------------------------------------------------------------------------
template <template <typename> class TOUT> template <typename T> void ObjectCache<TOUT> :: set (
    const Structure& s,
    const TOUT<T>& out,
    const Pointer& ptr
) {
    if(s.cache_idx == static_cast<size_t>(-1)) {
        s.cache_idx = db.next_cache_idx++;
        caches.resize(db.next_cache_idx);
    }
    caches[s.cache_idx][ptr] = std::static_pointer_cast<ElemBase>( out );

#ifndef ASSIMP_BUILD_BLENDER_NO_STATS
    ++db.stats().cached_objects;
#endif
}

}}
#endif
