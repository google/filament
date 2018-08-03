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

/** @file  BlenderDNA.h
 *  @brief Blender `DNA` (file format specification embedded in
 *    blend file itself) loader.
 */
#ifndef INCLUDED_AI_BLEND_DNA_H
#define INCLUDED_AI_BLEND_DNA_H

#include "BaseImporter.h"
#include "StreamReader.h"
#include <assimp/DefaultLogger.hpp>
#include <stdint.h>
#include <memory>
#include <map>

// enable verbose log output. really verbose, so be careful.
#ifdef ASSIMP_BUILD_DEBUG
#   define ASSIMP_BUILD_BLENDER_DEBUG
#endif

// #define ASSIMP_BUILD_BLENDER_NO_STATS

namespace Assimp    {

template <bool,bool> class StreamReader;
typedef StreamReader<true,true> StreamReaderAny;

namespace Blender {

class  FileDatabase;
struct FileBlockHead;

template <template <typename> class TOUT>
class ObjectCache;

// -------------------------------------------------------------------------------
/** Exception class used by the blender loader to selectively catch exceptions
 *  thrown in its own code (DeadlyImportErrors thrown in general utility
 *  functions are untouched then). If such an exception is not caught by
 *  the loader itself, it will still be caught by Assimp due to its
 *  ancestry. */
// -------------------------------------------------------------------------------
struct Error : DeadlyImportError {
    Error (const std::string& s)
    : DeadlyImportError(s) {
        // empty
    }
};

// -------------------------------------------------------------------------------
/** The only purpose of this structure is to feed a virtual dtor into its
 *  descendents. It serves as base class for all data structure fields. */
// -------------------------------------------------------------------------------
struct ElemBase {
    ElemBase()
    : dna_type(nullptr)
    {
        // empty
    }

    virtual ~ElemBase() {
        // empty
    }

    /** Type name of the element. The type
     * string points is the `c_str` of the `name` attribute of the
     * corresponding `Structure`, that is, it is only valid as long
     * as the DNA is not modified. The dna_type is only set if the
     * data type is not static, i.e. a std::shared_ptr<ElemBase>
     * in the scene description would have its type resolved
     * at runtime, so this member is always set. */
    const char* dna_type;
};

// -------------------------------------------------------------------------------
/** Represents a generic pointer to a memory location, which can be either 32
 *  or 64 bits. These pointers are loaded from the BLEND file and finally
 *  fixed to point to the real, converted representation of the objects
 *  they used to point to.*/
// -------------------------------------------------------------------------------
struct Pointer {
    Pointer()
    : val() {
        // empty
    }
    uint64_t val;
};

// -------------------------------------------------------------------------------
/** Represents a generic offset within a BLEND file */
// -------------------------------------------------------------------------------
struct FileOffset {
    FileOffset()
    : val() {
        // empty
    }
    uint64_t val;
};

// -------------------------------------------------------------------------------
/** Dummy derivate of std::vector to be able to use it in templates simultaenously
 *  with std::shared_ptr, which takes only one template argument
 *  while std::vector takes three. Also we need to provide some special member
 *  functions of shared_ptr */
// -------------------------------------------------------------------------------
template <typename T>
class vector : public std::vector<T> {
public:
    using std::vector<T>::resize;
    using std::vector<T>::empty;

    void reset() {
        resize(0);
    }

    operator bool () const {
        return !empty();
    }
};

// -------------------------------------------------------------------------------
/** Mixed flags for use in #Field */
// -------------------------------------------------------------------------------
enum FieldFlags {
    FieldFlag_Pointer = 0x1,
    FieldFlag_Array   = 0x2
};

// -------------------------------------------------------------------------------
/** Represents a single member of a data structure in a BLEND file */
// -------------------------------------------------------------------------------
struct Field {
    std::string name;
    std::string type;

    size_t size;
    size_t offset;

    /** Size of each array dimension. For flat arrays,
     *  the second dimension is set to 1. */
    size_t array_sizes[2];

    /** Any of the #FieldFlags enumerated values */
    unsigned int flags;
};

// -------------------------------------------------------------------------------
/** Range of possible behaviours for fields absend in the input file. Some are
 *  mission critical so we need them, while others can silently be default
 *  initialized and no animations are harmed. */
// -------------------------------------------------------------------------------
enum ErrorPolicy {
    /** Substitute default value and ignore */
    ErrorPolicy_Igno,
    /** Substitute default value and write to log */
    ErrorPolicy_Warn,
    /** Substitute a massive error message and crash the whole matrix. Its time for another zion */
    ErrorPolicy_Fail
};

#ifdef ASSIMP_BUILD_BLENDER_DEBUG
#   define ErrorPolicy_Igno ErrorPolicy_Warn
#endif

// -------------------------------------------------------------------------------
/** Represents a data structure in a BLEND file. A Structure defines n fields
 *  and their locatios and encodings the input stream. Usually, every
 *  Structure instance pertains to one equally-named data structure in the
 *  BlenderScene.h header. This class defines various utilities to map a
 *  binary `blob` read from the file to such a structure instance with
 *  meaningful contents. */
// -------------------------------------------------------------------------------
class Structure {
    template <template <typename> class> friend class ObjectCache;

public:
    Structure()
    : cache_idx(static_cast<size_t>(-1) ){
        // empty
    }

public:

    // publicly accessible members
    std::string name;
    vector< Field > fields;
    std::map<std::string, size_t> indices;

    size_t size;

public:

    // --------------------------------------------------------
    /** Access a field of the structure by its canonical name. The pointer version
     *  returns NULL on failure while the reference version raises an import error. */
    inline const Field& operator [] (const std::string& ss) const;
    inline const Field* Get (const std::string& ss) const;

    // --------------------------------------------------------
    /** Access a field of the structure by its index */
    inline const Field& operator [] (const size_t i) const;

    // --------------------------------------------------------
    inline bool operator== (const Structure& other) const {
        return name == other.name; // name is meant to be an unique identifier
    }

    // --------------------------------------------------------
    inline bool operator!= (const Structure& other) const {
        return name != other.name;
    }

public:

    // --------------------------------------------------------
    /** Try to read an instance of the structure from the stream
     *  and attempt to convert to `T`. This is done by
     *  an appropriate specialization. If none is available,
     *  a compiler complain is the result.
     *  @param dest Destination value to be written
     *  @param db File database, including input stream. */
    template <typename T> void Convert (T& dest, const FileDatabase& db) const;

    // --------------------------------------------------------
    // generic converter
    template <typename T>
    void Convert(std::shared_ptr<ElemBase> in,const FileDatabase& db) const;

    // --------------------------------------------------------
    // generic allocator
    template <typename T> std::shared_ptr<ElemBase> Allocate() const;



    // --------------------------------------------------------
    // field parsing for 1d arrays
    template <int error_policy, typename T, size_t M>
    void ReadFieldArray(T (& out)[M], const char* name,
        const FileDatabase& db) const;

    // --------------------------------------------------------
    // field parsing for 2d arrays
    template <int error_policy, typename T, size_t M, size_t N>
    void ReadFieldArray2(T (& out)[M][N], const char* name,
        const FileDatabase& db) const;

    // --------------------------------------------------------
    // field parsing for pointer or dynamic array types
    // (std::shared_ptr)
    // The return value indicates whether the data was already cached.
    template <int error_policy, template <typename> class TOUT, typename T>
    bool ReadFieldPtr(TOUT<T>& out, const char* name,
        const FileDatabase& db,
        bool non_recursive = false) const;

    // --------------------------------------------------------
    // field parsing for static arrays of pointer or dynamic
    // array types (std::shared_ptr[])
    // The return value indicates whether the data was already cached.
    template <int error_policy, template <typename> class TOUT, typename T, size_t N>
    bool ReadFieldPtr(TOUT<T> (&out)[N], const char* name,
        const FileDatabase& db) const;

    // --------------------------------------------------------
    // field parsing for `normal` values
    // The return value indicates whether the data was already cached.
    template <int error_policy, typename T>
    void ReadField(T& out, const char* name,
        const FileDatabase& db) const;

private:

    // --------------------------------------------------------
    template <template <typename> class TOUT, typename T>
    bool ResolvePointer(TOUT<T>& out, const Pointer & ptrval,
        const FileDatabase& db, const Field& f,
        bool non_recursive = false) const;

    // --------------------------------------------------------
    template <template <typename> class TOUT, typename T>
    bool ResolvePointer(vector< TOUT<T> >& out, const Pointer & ptrval,
        const FileDatabase& db, const Field& f, bool) const;

    // --------------------------------------------------------
    bool ResolvePointer( std::shared_ptr< FileOffset >& out, const Pointer & ptrval,
        const FileDatabase& db, const Field& f, bool) const;

    // --------------------------------------------------------
    inline const FileBlockHead* LocateFileBlockForAddress(
        const Pointer & ptrval,
        const FileDatabase& db) const;

private:

    // ------------------------------------------------------------------------------
    template <typename T> T* _allocate(std::shared_ptr<T>& out, size_t& s) const {
        out = std::shared_ptr<T>(new T());
        s = 1;
        return out.get();
    }

    template <typename T> T* _allocate(vector<T>& out, size_t& s) const {
        out.resize(s);
        return s ? &out.front() : NULL;
    }

    // --------------------------------------------------------
    template <int error_policy>
    struct _defaultInitializer {

        template <typename T, unsigned int N>
        void operator ()(T (& out)[N], const char* = NULL) {
            for (unsigned int i = 0; i < N; ++i) {
                out[i] = T();
            }
        }

        template <typename T, unsigned int N, unsigned int M>
        void operator ()(T (& out)[N][M], const char* = NULL) {
            for (unsigned int i = 0; i < N; ++i) {
                for (unsigned int j = 0; j < M; ++j) {
                    out[i][j] = T();
                }
            }
        }

        template <typename T>
        void operator ()(T& out, const char* = NULL) {
            out = T();
        }
    };

private:

    mutable size_t cache_idx;
};

// --------------------------------------------------------
template <>  struct Structure :: _defaultInitializer<ErrorPolicy_Warn> {

    template <typename T>
    void operator ()(T& out, const char* reason = "<add reason>") {
        DefaultLogger::get()->warn(reason);

        // ... and let the show go on
        _defaultInitializer<0 /*ErrorPolicy_Igno*/>()(out);
    }
};

template <> struct Structure :: _defaultInitializer<ErrorPolicy_Fail> {

    template <typename T>
    void operator ()(T& /*out*/,const char* = "") {
        // obviously, it is crucial that _DefaultInitializer is used
        // only from within a catch clause.
        throw;
    }
};

// -------------------------------------------------------------------------------------------------------
template <> inline bool Structure :: ResolvePointer<std::shared_ptr,ElemBase>(std::shared_ptr<ElemBase>& out,
    const Pointer & ptrval,
    const FileDatabase& db,
    const Field& f,
    bool
    ) const;


// -------------------------------------------------------------------------------
/** Represents the full data structure information for a single BLEND file.
 *  This data is extracted from the DNA1 chunk in the file.
 *  #DNAParser does the reading and represents currently the only place where
 *  DNA is altered.*/
// -------------------------------------------------------------------------------
class DNA
{
public:

    typedef void (Structure::*ConvertProcPtr) (
        std::shared_ptr<ElemBase> in,
        const FileDatabase&
    ) const;

    typedef std::shared_ptr<ElemBase> (
        Structure::*AllocProcPtr) () const;

    typedef std::pair< AllocProcPtr, ConvertProcPtr > FactoryPair;

public:

    std::map<std::string, FactoryPair > converters;
    vector<Structure > structures;
    std::map<std::string, size_t> indices;

public:

    // --------------------------------------------------------
    /** Access a structure by its canonical name, the pointer version returns NULL on failure
      * while the reference version raises an error. */
    inline const Structure& operator [] (const std::string& ss) const;
    inline const Structure* Get (const std::string& ss) const;

    // --------------------------------------------------------
    /** Access a structure by its index */
    inline const Structure& operator [] (const size_t i) const;

public:

    // --------------------------------------------------------
    /** Add structure definitions for all the primitive types,
     *  i.e. integer, short, char, float */
    void AddPrimitiveStructures();

    // --------------------------------------------------------
    /** Fill the @c converters member with converters for all
     *  known data types. The implementation of this method is
     *  in BlenderScene.cpp and is machine-generated.
     *  Converters are used to quickly handle objects whose
     *  exact data type is a runtime-property and not yet
     *  known at compile time (consier Object::data).*/
    void RegisterConverters();


    // --------------------------------------------------------
    /** Take an input blob from the stream, interpret it according to
     *  a its structure name and convert it to the intermediate
     *  representation.
     *  @param structure Destination structure definition
     *  @param db File database.
     *  @return A null pointer if no appropriate converter is available.*/
    std::shared_ptr< ElemBase > ConvertBlobToStructure(
        const Structure& structure,
        const FileDatabase& db
        ) const;

    // --------------------------------------------------------
    /** Find a suitable conversion function for a given Structure.
     *  Such a converter function takes a blob from the input
     *  stream, reads as much as it needs, and builds up a
     *  complete object in intermediate representation.
     *  @param structure Destination structure definition
     *  @param db File database.
     *  @return A null pointer in .first if no appropriate converter is available.*/
    FactoryPair GetBlobToStructureConverter(
        const Structure& structure,
        const FileDatabase& db
        ) const;


#ifdef ASSIMP_BUILD_BLENDER_DEBUG
    // --------------------------------------------------------
    /** Dump the DNA to a text file. This is for debugging purposes.
     *  The output file is `dna.txt` in the current working folder*/
    void DumpToFile();
#endif

    // --------------------------------------------------------
    /** Extract array dimensions from a C array declaration, such
     *  as `...[4][6]`. Returned string would be `...[][]`.
     *  @param out
     *  @param array_sizes Receive maximally two array dimensions,
     *    the second element is set to 1 if the array is flat.
     *    Both are set to 1 if the input is not an array.
     *  @throw DeadlyImportError if more than 2 dimensions are
     *    encountered. */
    static void ExtractArraySize(
        const std::string& out,
        size_t array_sizes[2]
    );
};

// special converters for primitive types
template <> inline void Structure :: Convert<int>       (int& dest,const FileDatabase& db) const;
template <> inline void Structure :: Convert<short>     (short& dest,const FileDatabase& db) const;
template <> inline void Structure :: Convert<char>      (char& dest,const FileDatabase& db) const;
template <> inline void Structure :: Convert<float>     (float& dest,const FileDatabase& db) const;
template <> inline void Structure :: Convert<double>    (double& dest,const FileDatabase& db) const;
template <> inline void Structure :: Convert<Pointer>   (Pointer& dest,const FileDatabase& db) const;

// -------------------------------------------------------------------------------
/** Describes a master file block header. Each master file sections holds n
 *  elements of a certain SDNA structure (or otherwise unspecified data). */
// -------------------------------------------------------------------------------
struct FileBlockHead
{
    // points right after the header of the file block
    StreamReaderAny::pos start;

    std::string id;
    size_t size;

    // original memory address of the data
    Pointer address;

    // index into DNA
    unsigned int dna_index;

    // number of structure instances to follow
    size_t num;



    // file blocks are sorted by address to quickly locate specific memory addresses
    bool operator < (const FileBlockHead& o) const {
        return address.val < o.address.val;
    }

    // for std::upper_bound
    operator const Pointer& () const {
        return address;
    }
};

// for std::upper_bound
inline bool operator< (const Pointer& a, const Pointer& b) {
    return a.val < b.val;
}

// -------------------------------------------------------------------------------
/** Utility to read all master file blocks in turn. */
// -------------------------------------------------------------------------------
class SectionParser
{
public:

    // --------------------------------------------------------
    /** @param stream Inout stream, must point to the
     *  first section in the file. Call Next() once
     *  to have it read.
     *  @param ptr64 Pointer size in file is 64 bits? */
    SectionParser(StreamReaderAny& stream,bool ptr64)
        : stream(stream)
        , ptr64(ptr64)
    {
        current.size = current.start = 0;
    }

public:

    // --------------------------------------------------------
    const FileBlockHead& GetCurrent() const {
        return current;
    }


public:

    // --------------------------------------------------------
    /** Advance to the next section.
     *  @throw DeadlyImportError if the last chunk was passed. */
    void Next();

public:

    FileBlockHead current;
    StreamReaderAny& stream;
    bool ptr64;
};


#ifndef ASSIMP_BUILD_BLENDER_NO_STATS
// -------------------------------------------------------------------------------
/** Import statistics, i.e. number of file blocks read*/
// -------------------------------------------------------------------------------
class Statistics {

public:

    Statistics ()
        : fields_read       ()
        , pointers_resolved ()
        , cache_hits        ()
//      , blocks_read       ()
        , cached_objects    ()
    {}

public:

    /** total number of fields we read */
    unsigned int fields_read;

    /** total number of resolved pointers */
    unsigned int pointers_resolved;

    /** number of pointers resolved from the cache */
    unsigned int cache_hits;

    /** number of blocks (from  FileDatabase::entries)
      we did actually read from. */
    // unsigned int blocks_read;

    /** objects in FileData::cache */
    unsigned int cached_objects;
};
#endif

// -------------------------------------------------------------------------------
/** The object cache - all objects addressed by pointers are added here. This
 *  avoids circular references and avoids object duplication. */
// -------------------------------------------------------------------------------
template <template <typename> class TOUT>
class ObjectCache
{
public:

    typedef std::map< Pointer, TOUT<ElemBase> > StructureCache;

public:

    ObjectCache(const FileDatabase& db)
        : db(db)
    {
        // currently there are only ~400 structure records per blend file.
        // we read only a small part of them and don't cache objects
        // which we don't need, so this should suffice.
        caches.reserve(64);
    }

public:

    // --------------------------------------------------------
    /** Check whether a specific item is in the cache.
     *  @param s Data type of the item
     *  @param out Output pointer. Unchanged if the
     *   cache doens't know the item yet.
     *  @param ptr Item address to look for. */
    template <typename T> void get (
        const Structure& s,
        TOUT<T>& out,
        const Pointer& ptr) const;

    // --------------------------------------------------------
    /** Add an item to the cache after the item has
     * been fully read. Do not insert anything that
     * may be faulty or might cause the loading
     * to abort.
     *  @param s Data type of the item
     *  @param out Item to insert into the cache
     *  @param ptr address (cache key) of the item. */
    template <typename T> void set
        (const Structure& s,
        const TOUT<T>& out,
        const Pointer& ptr);

private:

    mutable vector<StructureCache> caches;
    const FileDatabase& db;
};

// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------
template <> class ObjectCache<Blender::vector>
{
public:

    ObjectCache(const FileDatabase&) {}

    template <typename T> void get(const Structure&, vector<T>&, const Pointer&) {}
    template <typename T> void set(const Structure&, const vector<T>&, const Pointer&) {}
};

#ifdef _MSC_VER
#   pragma warning(disable:4355)
#endif

// -------------------------------------------------------------------------------
/** Memory representation of a full BLEND file and all its dependencies. The
 *  output aiScene is constructed from an instance of this data structure. */
// -------------------------------------------------------------------------------
class FileDatabase
{
    template <template <typename> class TOUT> friend class ObjectCache;

public:
    FileDatabase()
        : _cacheArrays(*this)
        , _cache(*this)
        , next_cache_idx()
    {}

public:
    // publicly accessible fields
    bool i64bit;
    bool little;

    DNA dna;
    std::shared_ptr< StreamReaderAny > reader;
    vector< FileBlockHead > entries;

public:

    Statistics& stats() const {
        return _stats;
    }

    // For all our templates to work on both shared_ptr's and vector's
    // using the same code, a dummy cache for arrays is provided. Actually,
    // arrays of objects are never cached because we can't easily
    // ensure their proper destruction.
    template <typename T>
    ObjectCache<std::shared_ptr>& cache(std::shared_ptr<T>& /*in*/) const {
        return _cache;
    }

    template <typename T>
    ObjectCache<vector>& cache(vector<T>& /*in*/) const {
        return _cacheArrays;
    }

private:


#ifndef ASSIMP_BUILD_BLENDER_NO_STATS
    mutable Statistics _stats;
#endif

    mutable ObjectCache<vector> _cacheArrays;
    mutable ObjectCache<std::shared_ptr> _cache;

    mutable size_t next_cache_idx;
};

#ifdef _MSC_VER
#   pragma warning(default:4355)
#endif

// -------------------------------------------------------------------------------
/** Factory to extract a #DNA from the DNA1 file block in a BLEND file. */
// -------------------------------------------------------------------------------
class DNAParser
{

public:

    /** Bind the parser to a empty DNA and an input stream */
    DNAParser(FileDatabase& db)
        : db(db)
    {}

public:

    // --------------------------------------------------------
    /** Locate the DNA in the file and parse it. The input
     *  stream is expected to point to the beginning of the DN1
     *  chunk at the time this method is called and is
     *  undefined afterwards.
     *  @throw DeadlyImportError if the DNA cannot be read.
     *  @note The position of the stream pointer is undefined
     *    afterwards.*/
    void Parse ();

public:

    /** Obtain a reference to the extracted DNA information */
    const Blender::DNA& GetDNA() const {
        return db.dna;
    }

private:

    FileDatabase& db;
};

    } // end Blend
} // end Assimp

#include "BlenderDNA.inl"

#endif
