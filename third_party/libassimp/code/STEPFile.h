/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2018, assimp team


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

#ifndef INCLUDED_AI_STEPFILE_H
#define INCLUDED_AI_STEPFILE_H

#include <bitset>
#include <memory>
#include <typeinfo>
#include <vector>
#include <map>
#include <set>

#include "FBXDocument.h" //ObjectMap::value_type
#include <assimp/DefaultLogger.hpp>

//
#if _MSC_VER > 1500 || (defined __GNUC___)
#   define ASSIMP_STEP_USE_UNORDERED_MULTIMAP
#else
#   define step_unordered_map map
#   define step_unordered_multimap multimap
#endif

#ifdef ASSIMP_STEP_USE_UNORDERED_MULTIMAP
#   include <unordered_map>
#   if _MSC_VER > 1600
#       define step_unordered_map unordered_map
#       define step_unordered_multimap unordered_multimap
#   else
#       define step_unordered_map tr1::unordered_map
#       define step_unordered_multimap tr1::unordered_multimap
#   endif
#endif

#include <assimp/LineSplitter.h>

// uncomment this to have the loader evaluate all entities upon loading.
// this is intended as stress test - by default, entities are evaluated
// lazily and therefore not unless needed.

//#define ASSIMP_IFC_TEST

namespace Assimp {

// ********************************************************************************
// before things get complicated, this is the basic outline:


namespace STEP {

    namespace EXPRESS {

        // base data types known by EXPRESS schemata - any custom data types will derive one of those
        class DataType;
            class UNSET;        /*: public DataType */
            class ISDERIVED;    /*: public DataType */
        //  class REAL;         /*: public DataType */
            class ENUM;         /*: public DataType */
        //  class STRING;       /*: public DataType */
        //  class INTEGER;      /*: public DataType */
            class ENTITY;       /*: public DataType */
            class LIST;         /*: public DataType */
        //  class SELECT;       /*: public DataType */

        // a conversion schema is not exactly an EXPRESS schema, rather it
        // is a list of pointers to conversion functions to build up the
        // object tree from an input file.
        class ConversionSchema;
    }

    struct HeaderInfo;
    class Object;
    class LazyObject;
    class DB;


    typedef Object* (*ConvertObjectProc)(const DB& db, const EXPRESS::LIST& params);
}

// ********************************************************************************

namespace STEP {

    // -------------------------------------------------------------------------------
    /** Exception class used by the STEP loading & parsing code. It is typically
     *  coupled with a line number. */
    // -------------------------------------------------------------------------------
    struct SyntaxError : DeadlyImportError {
        enum {
            LINE_NOT_SPECIFIED = 0xffffffffffffffffLL
        };

        SyntaxError (const std::string& s,uint64_t line = LINE_NOT_SPECIFIED);
    };


    // -------------------------------------------------------------------------------
    /** Exception class used by the STEP loading & parsing code when a type
     *  error (i.e. an entity expects a string but receives a bool) occurs.
     *  It is typically coupled with both an entity id and a line number.*/
    // -------------------------------------------------------------------------------
    struct TypeError : DeadlyImportError {
        enum {
            ENTITY_NOT_SPECIFIED = 0xffffffffffffffffLL,
            ENTITY_NOT_SPECIFIED_32 = 0x00000000ffffffff
        };

        TypeError (const std::string& s,uint64_t entity = ENTITY_NOT_SPECIFIED, uint64_t line = SyntaxError::LINE_NOT_SPECIFIED);
    };


    // hack to make a given member template-dependent
    template <typename T, typename T2>
    T2& Couple(T2& in) {
        return in;
    }


    namespace EXPRESS {

        // -------------------------------------------------------------------------------
        //** Base class for all STEP data types */
        // -------------------------------------------------------------------------------
        class DataType
        {
        public:
            typedef std::shared_ptr<const DataType> Out;

        public:

            virtual ~DataType() {
            }

        public:

            template <typename T>
            const T& To() const {
                return dynamic_cast<const T&>(*this);
            }

            template <typename T>
            T& To() {
                return dynamic_cast<T&>(*this);
            }


            template <typename T>
            const T* ToPtr() const {
                return dynamic_cast<const T*>(this);
            }

            template <typename T>
            T* ToPtr() {
                return dynamic_cast<T*>(this);
            }

            // utilities to deal with SELECT entities, which currently lack automatic
            // conversion support.
            template <typename T>
            const T& ResolveSelect(const DB& db) const {
                return Couple<T>(db).MustGetObject(To<EXPRESS::ENTITY>())->template To<T>();
            }

            template <typename T>
            const T* ResolveSelectPtr(const DB& db) const {
                const EXPRESS::ENTITY* e = ToPtr<EXPRESS::ENTITY>();
                return e?Couple<T>(db).MustGetObject(*e)->template ToPtr<T>():(const T*)0;
            }

        public:

            /** parse a variable from a string and set 'inout' to the character
             *  behind the last consumed character. An optional schema enables,
             *  if specified, automatic conversion of custom data types.
             *
             *  @throw SyntaxError
             */
            static std::shared_ptr<const EXPRESS::DataType> Parse(const char*& inout,
                uint64_t line                           = SyntaxError::LINE_NOT_SPECIFIED,
                const EXPRESS::ConversionSchema* schema = NULL);

        public:
        };

        typedef DataType SELECT;
        typedef DataType LOGICAL;

        // -------------------------------------------------------------------------------
        /** Sentinel class to represent explicitly unset (optional) fields ($) */
        // -------------------------------------------------------------------------------
        class UNSET : public DataType
        {
        public:
        private:
        };

        // -------------------------------------------------------------------------------
        /** Sentinel class to represent explicitly derived fields (*) */
        // -------------------------------------------------------------------------------
        class ISDERIVED : public DataType
        {
        public:
        private:
        };

        // -------------------------------------------------------------------------------
        /** Shared implementation for some of the primitive data type, i.e. int, float */
        // -------------------------------------------------------------------------------
        template <typename T>
        class PrimitiveDataType : public DataType
        {
        public:

            // This is the type that will cd ultimatively be used to
            // expose this data type to the user.
            typedef T Out;

        public:

            PrimitiveDataType() {}
            PrimitiveDataType(const T& val)
                : val(val)
            {}

            PrimitiveDataType(const PrimitiveDataType& o) {
                (*this) = o;
            }


        public:

            operator const T& () const {
                return val;
            }

            PrimitiveDataType& operator=(const PrimitiveDataType& o) {
                val = o.val;
                return *this;
            }

        protected:
            T val;

        };

        typedef PrimitiveDataType<int64_t>          INTEGER;
        typedef PrimitiveDataType<double>           REAL;
        typedef PrimitiveDataType<double>           NUMBER;
        typedef PrimitiveDataType<std::string>      STRING;



        // -------------------------------------------------------------------------------
        /** Generic base class for all enumerated types */
        // -------------------------------------------------------------------------------
        class ENUMERATION : public STRING
        {
        public:

            ENUMERATION (const std::string& val)
                : STRING(val)
            {}

        private:
        };

        typedef ENUMERATION BOOLEAN;

        // -------------------------------------------------------------------------------
        /** This is just a reference to an entity/object somewhere else */
        // -------------------------------------------------------------------------------
        class ENTITY : public PrimitiveDataType<uint64_t>
        {
        public:

            ENTITY(uint64_t val)
                : PrimitiveDataType<uint64_t>(val)
            {
                ai_assert(val!=0);
            }

            ENTITY()
                : PrimitiveDataType<uint64_t>(TypeError::ENTITY_NOT_SPECIFIED)
            {
            }

        private:
        };

        // -------------------------------------------------------------------------------
        /** Wrap any STEP aggregate: LIST, SET, ... */
        // -------------------------------------------------------------------------------
        class LIST : public DataType
        {
        public:

            // access a particular list index, throw std::range_error for wrong indices
            std::shared_ptr<const DataType> operator[] (size_t index) const {
                return members[index];
            }

            size_t GetSize() const {
                return members.size();
            }

        public:

            /** @see DaraType::Parse */
            static std::shared_ptr<const EXPRESS::LIST> Parse(const char*& inout,
                uint64_t line                           = SyntaxError::LINE_NOT_SPECIFIED,
                const EXPRESS::ConversionSchema* schema = NULL);


        private:
            typedef std::vector< std::shared_ptr<const DataType> > MemberList;
            MemberList members;
        };

        class BINARY : public PrimitiveDataType<uint32_t> {
        public:
            BINARY(uint32_t val)
            : PrimitiveDataType<uint32_t>(val) {
                // empty
            }

            BINARY()
            : PrimitiveDataType<uint32_t>(TypeError::ENTITY_NOT_SPECIFIED_32) {
                // empty
            }
        };

        // -------------------------------------------------------------------------------
        /* Not exactly a full EXPRESS schema but rather a list of conversion functions
         * to extract valid C++ objects out of a STEP file. Those conversion functions
         * may, however, perform further schema validations. */
        // -------------------------------------------------------------------------------
        class ConversionSchema {
        public:
            struct SchemaEntry {
                SchemaEntry( const char *name, ConvertObjectProc func )
                : mName( name )
                , mFunc(func) {
                    // empty
                }

                const char* mName;
                ConvertObjectProc mFunc;
            };

            typedef std::map<std::string,ConvertObjectProc> ConverterMap;

            template <size_t N>
            explicit ConversionSchema( const SchemaEntry (& schemas)[N]) {
                *this = schemas;
            }

            ConversionSchema() {

            }

            ConvertObjectProc GetConverterProc(const std::string& name) const {
                ConverterMap::const_iterator it = converters.find(name);
                return it == converters.end() ? nullptr : (*it).second;
            }

            bool IsKnownToken(const std::string& name) const {
                return converters.find(name) != converters.end();
            }

            const char* GetStaticStringForToken(const std::string& token) const {
                ConverterMap::const_iterator it = converters.find(token);
                return it == converters.end() ? nullptr : (*it).first.c_str();
            }


            template <size_t N>
            const ConversionSchema& operator=( const SchemaEntry (& schemas)[N]) {
                for(size_t i = 0; i < N; ++i ) {
                    const SchemaEntry& schema = schemas[i];
                    converters[schema.mName] = schema.mFunc;
                }
                return *this;
            }

        private:
            ConverterMap converters;
        };
    }



    // ------------------------------------------------------------------------------
    /** Bundle all the relevant info from a STEP header, parts of which may later
     *  be plainly dumped to the logfile, whereas others may help the caller pick an
     *  appropriate loading strategy.*/
    // ------------------------------------------------------------------------------
    struct HeaderInfo
    {
        std::string timestamp;
        std::string app;
        std::string fileSchema;
    };


    // ------------------------------------------------------------------------------
    /** Base class for all concrete object instances */
    // ------------------------------------------------------------------------------
    class Object {
    public:
        Object(const char* classname = "unknown")
        : id( 0 )
        , classname(classname) {
            // empty
        }

        virtual ~Object() {
            // empty
        }

        // utilities to simplify casting to concrete types
        template <typename T>
        const T& To() const {
            return dynamic_cast<const T&>(*this);
        }

        template <typename T>
        T& To() {
            return dynamic_cast<T&>(*this);
        }

        template <typename T>
        const T* ToPtr() const {
            return dynamic_cast<const T*>(this);
        }

        template <typename T>
        T* ToPtr() {
            return dynamic_cast<T*>(this);
        }

        uint64_t GetID() const {
            return id;
        }

        std::string GetClassName() const {
            return classname;
        }

        void SetID(uint64_t newval) {
            id = newval;
        }

    private:
        uint64_t id;
        const char* const classname;
    };

    template <typename T>
    size_t GenericFill(const STEP::DB& db, const EXPRESS::LIST& params, T* in);
    // (intentionally undefined)


    // ------------------------------------------------------------------------------
    /** CRTP shared base class for use by concrete entity implementation classes */
    // ------------------------------------------------------------------------------
    template <typename TDerived, size_t arg_count>
    struct ObjectHelper : virtual Object {
        ObjectHelper()
        : aux_is_derived(0) {
            // empty
        }

        static Object* Construct(const STEP::DB& db, const EXPRESS::LIST& params) {
            // make sure we don't leak if Fill() throws an exception
            std::unique_ptr<TDerived> impl(new TDerived());

            // GenericFill<T> is undefined so we need to have a specialization
            const size_t num_args = GenericFill<TDerived>(db,params,&*impl);
            (void)num_args;

            // the following check is commented because it will always trigger if
            // parts of the entities are generated with dummy wrapper code.
            // This is currently done to reduce the size of the loader
            // code.
            //if (num_args != params.GetSize() && impl->GetClassName() != "NotImplemented") {
            //  DefaultLogger::get()->debug("STEP: not all parameters consumed");
            //}
            return impl.release();
        }

        // note that this member always exists multiple times within the hierarchy
        // of an individual object, so any access to it must be disambiguated.
        std::bitset<arg_count> aux_is_derived;
    };

    // ------------------------------------------------------------------------------
    /** Class template used to represent OPTIONAL data members in the converted schema */
    // ------------------------------------------------------------------------------
    template <typename T>
    struct Maybe {
        Maybe()
        : have() {
            // empty
        }

        explicit Maybe(const T& ptr)
        : ptr(ptr)
        , have(true) {
            // empty
        }


        void flag_invalid() {
            have = false;
        }

        void flag_valid() {
            have = true;
        }


        bool operator! () const {
            return !have;
        }

        operator bool() const {
            return have;
        }

        operator const T&() const {
            return Get();
        }

        const T& Get() const {
            ai_assert(have);
            return ptr;
        }

        Maybe& operator=(const T& _ptr) {
            ptr = _ptr;
            have = true;
            return *this;
        }

    private:
        template <typename T2> friend struct InternGenericConvert;

        operator T&() {
            return ptr;
        }

        T ptr;
        bool have;
    };

    // ------------------------------------------------------------------------------
    /** A LazyObject is created when needed. Before this happens, we just keep
       the text line that contains the object definition. */
    // -------------------------------------------------------------------------------
    class LazyObject {
        friend class DB;

    public:
        LazyObject(DB& db, uint64_t id, uint64_t line, const char* type,const char* args);
        ~LazyObject();

        Object& operator * () {
            if (!obj) {
                LazyInit();
                ai_assert(obj);
            }
            return *obj;
        }

        const Object& operator * () const {
            if (!obj) {
                LazyInit();
                ai_assert(obj);
            }
            return *obj;
        }

        template <typename T>
        const T& To() const {
            return dynamic_cast<const T&>( **this );
        }

        template <typename T>
        T& To()  {
            return dynamic_cast<T&>( **this );
        }

        template <typename T>
        const T* ToPtr() const {
            return dynamic_cast<const T*>( &**this );
        }

        template <typename T>
        T* ToPtr()  {
            return dynamic_cast<T*>( &**this );
        }

        Object* operator -> () {
            return &**this;
        }

        const Object* operator -> () const {
            return &**this;
        }

        bool operator== (const std::string& atype) const {
            return type == atype;
        }

        bool operator!= (const std::string& atype) const {
            return type != atype;
        }

        uint64_t GetID() const {
            return id;
        }

    private:
        void LazyInit() const;

    private:
        mutable uint64_t id;
        const char* const type;
        DB& db;
        mutable const char* args;
        mutable Object* obj;
    };

    template <typename T>
    inline
    bool operator==( std::shared_ptr<LazyObject> lo, T whatever ) {
        return *lo == whatever; // XXX use std::forward if we have 0x
    }

    template <typename T>
    inline
    bool operator==( const std::pair<uint64_t, std::shared_ptr<LazyObject> >& lo, T whatever ) {
        return *(lo.second) == whatever; // XXX use std::forward if we have 0x
    }

    // ------------------------------------------------------------------------------
    /** Class template used to represent lazily evaluated object references in the converted schema */
    // ------------------------------------------------------------------------------
    template <typename T>
    struct Lazy {
        typedef Lazy Out;
        Lazy(const LazyObject* obj = nullptr)
        : obj(obj) {
            // empty
        }

        operator const T*() const {
            return obj->ToPtr<T>();
        }

        operator const T&() const {
            return obj->To<T>();
        }

        const T& operator * () const {
            return obj->To<T>();
        }

        const T* operator -> () const {
            return &obj->To<T>();
        }

        const LazyObject* obj;
    };

    // ------------------------------------------------------------------------------
    /** Class template used to represent LIST and SET data members in the converted schema */
    // ------------------------------------------------------------------------------
    template <typename T, uint64_t min_cnt, uint64_t max_cnt=0uL>
    struct ListOf : public std::vector<typename T::Out> {
        typedef typename T::Out OutScalar;
        typedef ListOf Out;

        ListOf() {
            static_assert(min_cnt <= max_cnt || !max_cnt, "min_cnt <= max_cnt || !max_cnt");
        }
    };

    // ------------------------------------------------------------------------------
    template <typename TOut>
    struct PickBaseType {
        typedef EXPRESS::PrimitiveDataType<TOut> Type;
    };

    template <typename TOut>
    struct PickBaseType< Lazy<TOut> > {
        typedef EXPRESS::ENTITY Type;
    };

    template<>
    struct PickBaseType< std::shared_ptr< const EXPRESS::DataType > >;

    // ------------------------------------------------------------------------------
    template <typename T>
    struct InternGenericConvert {
        void operator()(T& out, const std::shared_ptr< const EXPRESS::DataType >& in, const STEP::DB& /*db*/) {
            try{
                out = dynamic_cast< const typename PickBaseType<T>::Type& > ( *in );
            } catch(std::bad_cast&) {
                throw TypeError("type error reading literal field");
            }
        }
    };

    template <>
    struct InternGenericConvert< std::shared_ptr< const EXPRESS::DataType > > {
        void operator()(std::shared_ptr< const EXPRESS::DataType >& out, const std::shared_ptr< const EXPRESS::DataType >& in, const STEP::DB& /*db*/) {
            out = in;
        }
    };

    template <typename T>
    struct InternGenericConvert< Maybe<T> > {
        void operator()(Maybe<T>& out, const std::shared_ptr< const EXPRESS::DataType >& in, const STEP::DB& db) {
            GenericConvert((T&)out,in,db);
            out.flag_valid();
        }
    };

    template <typename T,uint64_t min_cnt, uint64_t max_cnt>
    struct InternGenericConvertList {
        void operator()(ListOf<T, min_cnt, max_cnt>& out, const std::shared_ptr< const EXPRESS::DataType >& inp_base, const STEP::DB& db) {

            const EXPRESS::LIST* inp = dynamic_cast<const EXPRESS::LIST*>(inp_base.get());
            if (!inp) {
                throw TypeError("type error reading aggregate");
            }

            // XXX is this really how the EXPRESS notation ([?:3],[1:3]) is intended?
            if (max_cnt && inp->GetSize() > max_cnt) {
                ASSIMP_LOG_WARN("too many aggregate elements");
            }
            else if (inp->GetSize() < min_cnt) {
                ASSIMP_LOG_WARN("too few aggregate elements");
            }

            out.reserve(inp->GetSize());
            for(size_t i = 0; i < inp->GetSize(); ++i) {

                out.push_back( typename ListOf<T, min_cnt, max_cnt>::OutScalar() );
                try{
                    GenericConvert(out.back(),(*inp)[i], db);
                }
                catch(const TypeError& t) {
                    throw TypeError(t.what() +std::string(" of aggregate"));
                }
            }
        }
    };

    template <typename T>
    struct InternGenericConvert< Lazy<T> > {
        void operator()(Lazy<T>& out, const std::shared_ptr< const EXPRESS::DataType >& in_base, const STEP::DB& db) {
            const EXPRESS::ENTITY* in = dynamic_cast<const EXPRESS::ENTITY*>(in_base.get());
            if (!in) {
                throw TypeError("type error reading entity");
            }
            out = Couple<T>(db).GetObject(*in);
        }
    };

    template <typename T1>
    inline void GenericConvert(T1& a, const std::shared_ptr< const EXPRESS::DataType >& b, const STEP::DB& db) {
        return InternGenericConvert<T1>()(a,b,db);
    }

    template <typename T1,uint64_t N1, uint64_t N2>
    inline void GenericConvert(ListOf<T1,N1,N2>& a, const std::shared_ptr< const EXPRESS::DataType >& b, const STEP::DB& db) {
        return InternGenericConvertList<T1,N1,N2>()(a,b,db);
    }

    // ------------------------------------------------------------------------------
    /** Lightweight manager class that holds the map of all objects in a
     *  STEP file. DB's are exclusively maintained by the functions in
     *  STEPFileReader.h*/
    // -------------------------------------------------------------------------------
    class DB
    {
        friend DB* ReadFileHeader(std::shared_ptr<IOStream> stream);
        friend void ReadFile(DB& db,const EXPRESS::ConversionSchema& scheme,
            const char* const* types_to_track, size_t len,
            const char* const* inverse_indices_to_track, size_t len2
        );

        friend class LazyObject;

    public:
        // objects indexed by ID - this can grow pretty large (i.e some hundred million
        // entries), so use raw pointers to avoid *any* overhead.
        typedef std::map<uint64_t,const LazyObject* > ObjectMap;

        // objects indexed by their declarative type, but only for those that we truly want
        typedef std::set< const LazyObject*> ObjectSet;
        typedef std::map<std::string, ObjectSet > ObjectMapByType;

        // list of types for which to keep inverse indices for all references
        // that the respective objects keep.
        // the list keeps pointers to strings in static storage
        typedef std::set<const char*> InverseWhitelist;

        // references - for each object id the ids of all objects which reference it
        // this is used to simulate STEP inverse indices for selected types.
        typedef std::step_unordered_multimap<uint64_t, uint64_t > RefMap;
        typedef std::pair<RefMap::const_iterator,RefMap::const_iterator> RefMapRange;

    private:

        DB(std::shared_ptr<StreamReaderLE> reader)
            : reader(reader)
            , splitter(*reader,true,true)
            , evaluated_count()
            , schema( nullptr )
        {}

    public:
        ~DB() {
            for(ObjectMap::value_type& o : objects) {
                delete o.second;
            }
        }

        uint64_t GetObjectCount() const {
            return objects.size();
        }

        uint64_t GetEvaluatedObjectCount() const {
            return evaluated_count;
        }

        const HeaderInfo& GetHeader() const {
            return header;
        }

        const EXPRESS::ConversionSchema& GetSchema() const {
            return *schema;
        }

        const ObjectMap& GetObjects() const {
            return objects;
        }

        const ObjectMapByType& GetObjectsByType() const {
            return objects_bytype;
        }

        const RefMap& GetRefs() const {
            return refs;
        }

        bool KeepInverseIndicesForType(const char* const type) const {
            return inv_whitelist.find(type) != inv_whitelist.end();
        }


        // get the yet unevaluated object record with a given id
        const LazyObject* GetObject(uint64_t id) const {
            const ObjectMap::const_iterator it = objects.find(id);
            if (it != objects.end()) {
                return (*it).second;
            }
            return nullptr;
        }


        // get an arbitrary object out of the soup with the only restriction being its type.
        const LazyObject* GetObject(const std::string& type) const {
            const ObjectMapByType::const_iterator it = objects_bytype.find(type);
            if (it != objects_bytype.end() && (*it).second.size()) {
                return *(*it).second.begin();
            }
            return NULL;
        }

        // same, but raise an exception if the object doesn't exist and return a reference
        const LazyObject& MustGetObject(uint64_t id) const {
            const LazyObject* o = GetObject(id);
            if (!o) {
                throw TypeError("requested entity is not present",id);
            }
            return *o;
        }

        const LazyObject& MustGetObject(const std::string& type) const {
            const LazyObject* o = GetObject(type);
            if (!o) {
                throw TypeError("requested entity of type "+type+"is not present");
            }
            return *o;
        }


#ifdef ASSIMP_IFC_TEST

        // evaluate *all* entities in the file. this is a power test for the loader
        void EvaluateAll() {
            for(ObjectMap::value_type& e :objects) {
                **e.second;
            }
            ai_assert(evaluated_count == objects.size());
        }

#endif

    private:

        // full access only offered to close friends - they should
        // use the provided getters rather than messing around with
        // the members directly.
        LineSplitter& GetSplitter() {
            return splitter;
        }

        void InternInsert(const LazyObject* lz) {
            objects[lz->GetID()] = lz;

            const ObjectMapByType::iterator it = objects_bytype.find( lz->type );
            if (it != objects_bytype.end()) {
                (*it).second.insert(lz);
            }
        }

        void SetSchema(const EXPRESS::ConversionSchema& _schema) {
            schema = &_schema;
        }


        void SetTypesToTrack(const char* const* types, size_t N) {
            for(size_t i = 0; i < N;++i) {
                objects_bytype[types[i]] = ObjectSet();
            }
        }

        void SetInverseIndicesToTrack( const char* const* types, size_t N ) {
            for(size_t i = 0; i < N;++i) {
                const char* const sz = schema->GetStaticStringForToken(types[i]);
                ai_assert(sz);
                inv_whitelist.insert(sz);
            }
        }

        HeaderInfo& GetHeader() {
            return header;
        }

        void MarkRef(uint64_t who, uint64_t by_whom) {
            refs.insert(std::make_pair(who,by_whom));
        }

    private:
        HeaderInfo header;
        ObjectMap objects;
        ObjectMapByType objects_bytype;
        RefMap refs;
        InverseWhitelist inv_whitelist;
        std::shared_ptr<StreamReaderLE> reader;
        LineSplitter splitter;
        uint64_t evaluated_count;
        const EXPRESS::ConversionSchema* schema;
    };

}

} // end Assimp

#endif // INCLUDED_AI_STEPFILE_H
