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

/** @file  STEPFileReader.cpp
 *  @brief Implementation of the STEP file parser, which fills a
 *     STEP::DB with data read from a file.
 */

#include "STEPFileReader.h"
#include "STEPFileEncoding.h"
#include <assimp/TinyFormatter.h>
#include <assimp/fast_atof.h>
#include <memory>


using namespace Assimp;
namespace EXPRESS = STEP::EXPRESS;

#include <functional>

// ------------------------------------------------------------------------------------------------
std::string AddLineNumber(const std::string& s,uint64_t line /*= LINE_NOT_SPECIFIED*/, const std::string& prefix = "")
{
    return line == STEP::SyntaxError::LINE_NOT_SPECIFIED ? prefix+s : static_cast<std::string>( (Formatter::format(),prefix,"(line ",line,") ",s) );
}


// ------------------------------------------------------------------------------------------------
std::string AddEntityID(const std::string& s,uint64_t entity /*= ENTITY_NOT_SPECIFIED*/, const std::string& prefix = "")
{
    return entity == STEP::TypeError::ENTITY_NOT_SPECIFIED ? prefix+s : static_cast<std::string>( (Formatter::format(),prefix,"(entity #",entity,") ",s));
}


// ------------------------------------------------------------------------------------------------
STEP::SyntaxError::SyntaxError (const std::string& s,uint64_t line /* = LINE_NOT_SPECIFIED */)
: DeadlyImportError(AddLineNumber(s,line))
{

}

// ------------------------------------------------------------------------------------------------
STEP::TypeError::TypeError (const std::string& s,uint64_t entity /* = ENTITY_NOT_SPECIFIED */,uint64_t line /*= LINE_NOT_SPECIFIED*/)
: DeadlyImportError(AddLineNumber(AddEntityID(s,entity),line))
{

}

static const char *ISO_Token         = "ISO-10303-21;";
static const char *FILE_SCHEMA_Token = "FILE_SCHEMA";
// ------------------------------------------------------------------------------------------------
STEP::DB* STEP::ReadFileHeader(std::shared_ptr<IOStream> stream) {
    std::shared_ptr<StreamReaderLE> reader = std::shared_ptr<StreamReaderLE>(new StreamReaderLE(stream));
    std::unique_ptr<STEP::DB> db = std::unique_ptr<STEP::DB>(new STEP::DB(reader));

    LineSplitter &splitter = db->GetSplitter();
    if (!splitter || *splitter != ISO_Token ) {
        throw STEP::SyntaxError("expected magic token: " + std::string( ISO_Token ), 1);
    }

    HeaderInfo& head = db->GetHeader();
    for(++splitter; splitter; ++splitter) {
        const std::string& s = *splitter;
        if (s == "DATA;") {
            // here we go, header done, start of data section
            ++splitter;
            break;
        }

        // want one-based line numbers for human readers, so +1
        const uint64_t line = splitter.get_index()+1;

        if (s.substr(0,11) == FILE_SCHEMA_Token) {
            const char* sz = s.c_str()+11;
            SkipSpaces(sz,&sz);
            std::shared_ptr< const EXPRESS::DataType > schema = EXPRESS::DataType::Parse(sz);

            // the file schema should be a regular list entity, although it usually contains exactly one entry
            // since the list itself is contained in a regular parameter list, we actually have
            // two nested lists.
            const EXPRESS::LIST* list = dynamic_cast<const EXPRESS::LIST*>(schema.get());
            if (list && list->GetSize()) {
                list = dynamic_cast<const EXPRESS::LIST*>( (*list)[0].get() );
                if (!list) {
                    throw STEP::SyntaxError("expected FILE_SCHEMA to be a list",line);
                }

                // XXX need support for multiple schemas?
                if (list->GetSize() > 1)    {
                    ASSIMP_LOG_WARN(AddLineNumber("multiple schemas currently not supported",line));
                }
                const EXPRESS::STRING* string( nullptr );
                if (!list->GetSize() || !(string=dynamic_cast<const EXPRESS::STRING*>( (*list)[0].get() ))) {
                    throw STEP::SyntaxError("expected FILE_SCHEMA to contain a single string literal",line);
                }
                head.fileSchema =  *string;
            }
        }

        // XXX handle more header fields
    }

    return db.release();
}


namespace {

// ------------------------------------------------------------------------------------------------
// check whether the given line contains an entity definition (i.e. starts with "#<number>=")
bool IsEntityDef(const std::string& snext)
{
    if (snext[0] == '#') {
        // it is only a new entity if it has a '=' after the
        // entity ID.
        for(std::string::const_iterator it = snext.begin()+1; it != snext.end(); ++it) {
            if (*it == '=') {
                return true;
            }
            if ((*it < '0' || *it > '9') && *it != ' ') {
                break;
            }
        }
    }
    return false;
}

}


// ------------------------------------------------------------------------------------------------
void STEP::ReadFile(DB& db,const EXPRESS::ConversionSchema& scheme,
    const char* const* types_to_track, size_t len,
    const char* const* inverse_indices_to_track, size_t len2)
{
    db.SetSchema(scheme);
    db.SetTypesToTrack(types_to_track,len);
    db.SetInverseIndicesToTrack(inverse_indices_to_track,len2);

    const DB::ObjectMap& map = db.GetObjects();
    LineSplitter& splitter = db.GetSplitter();

    while (splitter) {
        bool has_next = false;
        std::string s = *splitter;
        if (s == "ENDSEC;") {
            break;
        }
        s.erase(std::remove(s.begin(), s.end(), ' '), s.end());

        // want one-based line numbers for human readers, so +1
        const uint64_t line = splitter.get_index()+1;
        // LineSplitter already ignores empty lines
        ai_assert(s.length());
        if (s[0] != '#') {
            ASSIMP_LOG_WARN(AddLineNumber("expected token \'#\'",line));
            ++splitter;
            continue;
        }
        // ---
        // extract id, entity class name and argument string,
        // but don't create the actual object yet.
        // ---
        const std::string::size_type n0 = s.find_first_of('=');
        if (n0 == std::string::npos) {
            ASSIMP_LOG_WARN(AddLineNumber("expected token \'=\'",line));
            ++splitter;
            continue;
        }

        const uint64_t id = strtoul10_64(s.substr(1,n0-1).c_str());
        if (!id) {
            ASSIMP_LOG_WARN(AddLineNumber("expected positive, numeric entity id",line));
            ++splitter;
            continue;
        }
        std::string::size_type n1 = s.find_first_of('(',n0);
        if (n1 == std::string::npos) {
            has_next = true;
            bool ok = false;
            for( ++splitter; splitter; ++splitter) {
                const std::string& snext = *splitter;
                if (snext.empty()) {
                    continue;
                }

                // the next line doesn't start an entity, so maybe it is
                // just a continuation  for this line, keep going
                if (!IsEntityDef(snext)) {
                    s.append(snext);
                    n1 = s.find_first_of('(',n0);
                    ok = (n1 != std::string::npos);
                }
                else {
                    break;
                }
            }

            if(!ok) {
                ASSIMP_LOG_WARN(AddLineNumber("expected token \'(\'",line));
                continue;
            }
        }

        std::string::size_type n2 = s.find_last_of(')');
        if (n2 == std::string::npos || n2 < n1 || n2 == s.length() - 1 || s[n2 + 1] != ';') {

            has_next = true;
            bool ok = false;
            for( ++splitter; splitter; ++splitter) {
                const std::string& snext = *splitter;
                if (snext.empty()) {
                    continue;
                }
                // the next line doesn't start an entity, so maybe it is
                // just a continuation  for this line, keep going
                if (!IsEntityDef(snext)) {
                    s.append(snext);
                    n2 = s.find_last_of(')');
                    ok = !(n2 == std::string::npos || n2 < n1 || n2 == s.length() - 1 || s[n2 + 1] != ';');
                }
                else {
                    break;
                }
            }
            if(!ok) {
                ASSIMP_LOG_WARN(AddLineNumber("expected token \')\'",line));
                continue;
            }
        }

        if (map.find(id) != map.end()) {
            ASSIMP_LOG_WARN(AddLineNumber((Formatter::format(),"an object with the id #",id," already exists"),line));
        }

        std::string::size_type ns = n0;
        do ++ns; while( IsSpace(s.at(ns)));
        std::string::size_type ne = n1;
        do --ne; while( IsSpace(s.at(ne)));
        std::string type = s.substr(ns,ne-ns+1);
        std::transform( type.begin(), type.end(), type.begin(), &Assimp::ToLower<char>  );
        const char* sz = scheme.GetStaticStringForToken(type);
        if(sz) {
            const std::string::size_type len = n2-n1+1;
            char* const copysz = new char[len+1];
            std::copy(s.c_str()+n1,s.c_str()+n2+1,copysz);
            copysz[len] = '\0';
            db.InternInsert(new LazyObject(db,id,line,sz,copysz));
        }
        if(!has_next) {
            ++splitter;
        }
    }

    if (!splitter) {
        ASSIMP_LOG_WARN("STEP: ignoring unexpected EOF");
    }

    if ( !DefaultLogger::isNullLogger()){
        ASSIMP_LOG_DEBUG((Formatter::format(),"STEP: got ",map.size()," object records with ",
            db.GetRefs().size()," inverse index entries"));
    }
}

// ------------------------------------------------------------------------------------------------
std::shared_ptr<const EXPRESS::DataType> EXPRESS::DataType::Parse(const char*& inout,uint64_t line, const EXPRESS::ConversionSchema* schema /*= NULL*/)
{
    const char* cur = inout;
    SkipSpaces(&cur);
    if (*cur == ',' || IsSpaceOrNewLine(*cur)) {
        throw STEP::SyntaxError("unexpected token, expected parameter",line);
    }

    // just skip over constructions such as IFCPLANEANGLEMEASURE(0.01) and read only the value
    if (schema) {
        bool ok = false;
        for(const char* t = cur; *t && *t != ')' && *t != ','; ++t) {
            if (*t=='(') {
                if (!ok) {
                    break;
                }
                for(--t;IsSpace(*t);--t);
                std::string s(cur,static_cast<size_t>(t-cur+1));
                std::transform(s.begin(),s.end(),s.begin(),&ToLower<char> );
                if (schema->IsKnownToken(s)) {
                    for(cur = t+1;*cur++ != '(';);
                    const std::shared_ptr<const EXPRESS::DataType> dt = Parse(cur);
                    inout = *cur ? cur+1 : cur;
                    return dt;
                }
                break;
            }
            else if (!IsSpace(*t)) {
                ok = true;
            }
        }
    }

    if (*cur == '*' ) {
        inout = cur+1;
        return std::make_shared<EXPRESS::ISDERIVED>();
    }
    else if (*cur == '$' ) {
        inout = cur+1;
        return std::make_shared<EXPRESS::UNSET>();
    }
    else if (*cur == '(' ) {
        // start of an aggregate, further parsing is done by the LIST factory constructor
        inout = cur;
        return EXPRESS::LIST::Parse(inout,line,schema);
    }
    else if (*cur == '.' ) {
        // enum (includes boolean)
        const char* start = ++cur;
        for(;*cur != '.';++cur) {
            if (*cur == '\0') {
                throw STEP::SyntaxError("enum not closed",line);
            }
        }
        inout = cur+1;
        return std::make_shared<EXPRESS::ENUMERATION>(std::string(start, static_cast<size_t>(cur-start) ));
    }
    else if (*cur == '#' ) {
        // object reference
        return std::make_shared<EXPRESS::ENTITY>(strtoul10_64(++cur,&inout));
    }
    else if (*cur == '\'' ) {
        // string literal
        const char* start = ++cur;

        for(;*cur != '\'';++cur)    {
            if (*cur == '\0')   {
                throw STEP::SyntaxError("string literal not closed",line);
            }
        }

        if (cur[1] == '\'') {
            // Vesanen: more than 2 escaped ' in one literal!
            do  {
                for(cur += 2;*cur != '\'';++cur)    {
                    if (*cur == '\0')   {
                        throw STEP::SyntaxError("string literal not closed",line);
                    }
                }
            }
            while(cur[1] == '\'');
        }

        inout = cur + 1;

        // assimp is supposed to output UTF8 strings, so we have to deal
        // with foreign encodings.
        std::string stemp = std::string(start, static_cast<size_t>(cur - start));
        if(!StringToUTF8(stemp)) {
            // TODO: route this to a correct logger with line numbers etc., better error messages
            ASSIMP_LOG_ERROR("an error occurred reading escape sequences in ASCII text");
        }

        return std::make_shared<EXPRESS::STRING>(stemp);
    }
    else if (*cur == '\"' ) {
        throw STEP::SyntaxError("binary data not supported yet",line);
    }

    // else -- must be a number. if there is a decimal dot in it,
    // parse it as real value, otherwise as integer.
    const char* start = cur;
    for(;*cur  && *cur != ',' && *cur != ')' && !IsSpace(*cur);++cur) {
        if (*cur == '.') {
            double f;
            inout = fast_atoreal_move<double>(start,f);
            return std::make_shared<EXPRESS::REAL>(f);
        }
    }

    bool neg = false;
    if (*start == '-') {
        neg = true;
        ++start;
    }
    else if (*start == '+') {
        ++start;
    }
    int64_t num = static_cast<int64_t>( strtoul10_64(start,&inout) );
    return std::make_shared<EXPRESS::INTEGER>(neg?-num:num);
}


// ------------------------------------------------------------------------------------------------
std::shared_ptr<const EXPRESS::LIST> EXPRESS::LIST::Parse(const char*& inout,uint64_t line, const EXPRESS::ConversionSchema* schema /*= NULL*/)
{
    const std::shared_ptr<EXPRESS::LIST> list = std::make_shared<EXPRESS::LIST>();
    EXPRESS::LIST::MemberList& members = list->members;

    const char* cur = inout;
    if (*cur++ != '(') {
        throw STEP::SyntaxError("unexpected token, expected \'(\' token at beginning of list",line);
    }

    // estimate the number of items upfront - lists can grow large
    size_t count = 1;
    for(const char* c=cur; *c && *c != ')'; ++c) {
        count += (*c == ',' ? 1 : 0);
    }

    members.reserve(count);

    for(;;++cur) {
        if (!*cur) {
            throw STEP::SyntaxError("unexpected end of line while reading list");
        }
        SkipSpaces(cur,&cur);
        if (*cur == ')') {
            break;
        }

        members.push_back( EXPRESS::DataType::Parse(cur,line,schema));
        SkipSpaces(cur,&cur);

        if (*cur != ',') {
            if (*cur == ')') {
                break;
            }
            throw STEP::SyntaxError("unexpected token, expected \',\' or \')\' token after list element",line);
        }
    }

    inout = cur+1;
    return list;
}


// ------------------------------------------------------------------------------------------------
STEP::LazyObject::LazyObject(DB& db, uint64_t id,uint64_t /*line*/, const char* const type,const char* args)
    : id(id)
    , type(type)
    , db(db)
    , args(args)
    , obj()
{
    // find any external references and store them in the database.
    // this helps us emulate STEPs INVERSE fields.
    if (db.KeepInverseIndicesForType(type)) {
        const char* a  = args;

        // do a quick scan through the argument tuple and watch out for entity references
        int64_t skip_depth = 0;
        while(*a) {
            if (*a == '(') {
                ++skip_depth;
            }
            else if (*a == ')') {
                --skip_depth;
            }

			if (skip_depth >= 1 && *a=='#') {
				if (*(a + 1) != '#')
				{
					const char* tmp;
					const int64_t num = static_cast<int64_t>(strtoul10_64(a + 1, &tmp));
					db.MarkRef(num, id);
				}
				else
				{
					++a;
				}
            }
            ++a;
        }

    }
}

// ------------------------------------------------------------------------------------------------
STEP::LazyObject::~LazyObject()
{
    // make sure the right dtor/operator delete get called
    if (obj) {
        delete obj;
    }
    else delete[] args;
}

// ------------------------------------------------------------------------------------------------
void STEP::LazyObject::LazyInit() const
{
    const EXPRESS::ConversionSchema& schema = db.GetSchema();
    STEP::ConvertObjectProc proc = schema.GetConverterProc(type);

    if (!proc) {
        throw STEP::TypeError("unknown object type: " + std::string(type),id);
    }

    const char* acopy = args;
    std::shared_ptr<const EXPRESS::LIST> conv_args = EXPRESS::LIST::Parse(acopy,STEP::SyntaxError::LINE_NOT_SPECIFIED,&db.GetSchema());
    delete[] args;
    args = NULL;

    // if the converter fails, it should throw an exception, but it should never return NULL
    try {
        obj = proc(db,*conv_args);
    }
    catch(const TypeError& t) {
        // augment line and entity information
        throw TypeError(t.what(),id);
    }
    ++db.evaluated_count;
    ai_assert(obj);

    // store the original id in the object instance
    obj->SetID(id);
}
