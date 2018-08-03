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

/** @file  BlenderDNA.cpp
 *  @brief Implementation of the Blender `DNA`, that is its own
 *    serialized set of data structures.
 */


#ifndef ASSIMP_BUILD_NO_BLEND_IMPORTER
#include "BlenderDNA.h"
#include "StreamReader.h"
#include "fast_atof.h"
#include "TinyFormatter.h"

using namespace Assimp;
using namespace Assimp::Blender;
using namespace Assimp::Formatter;

static bool match4(StreamReaderAny& stream, const char* string) {
    ai_assert( nullptr != string );
    char tmp[] = {
        (const char)(stream).GetI1(),
        (const char)(stream).GetI1(),
        (const char)(stream).GetI1(),
        (const char)(stream).GetI1()
    };
    return (tmp[0]==string[0] && tmp[1]==string[1] && tmp[2]==string[2] && tmp[3]==string[3]);
}

struct Type {
    size_t size;
    std::string name;
};

// ------------------------------------------------------------------------------------------------
void DNAParser::Parse ()
{
    StreamReaderAny& stream = *db.reader.get();
    DNA& dna = db.dna;

    if(!match4(stream,"SDNA")) {
        throw DeadlyImportError("BlenderDNA: Expected SDNA chunk");
    }

    // name dictionary
    if(!match4(stream,"NAME")) {
        throw DeadlyImportError("BlenderDNA: Expected NAME field");
    }

    std::vector<std::string> names (stream.GetI4());
    for(std::string& s : names) {
        while (char c = stream.GetI1()) {
            s += c;
        }
    }

    // type dictionary
    for (;stream.GetCurrentPos() & 0x3; stream.GetI1());
    if(!match4(stream,"TYPE")) {
        throw DeadlyImportError("BlenderDNA: Expected TYPE field");
    }

    std::vector<Type> types (stream.GetI4());
    for(Type& s : types) {
        while (char c = stream.GetI1()) {
            s.name += c;
        }
    }

    // type length dictionary
    for (;stream.GetCurrentPos() & 0x3; stream.GetI1());
    if(!match4(stream,"TLEN")) {
        throw DeadlyImportError("BlenderDNA: Expected TLEN field");
    }

    for(Type& s : types) {
        s.size = stream.GetI2();
    }

    // structures dictionary
    for (;stream.GetCurrentPos() & 0x3; stream.GetI1());
    if(!match4(stream,"STRC")) {
        throw DeadlyImportError("BlenderDNA: Expected STRC field");
    }

    size_t end = stream.GetI4(), fields = 0;

    dna.structures.reserve(end);
    for(size_t i = 0; i != end; ++i) {

        uint16_t n = stream.GetI2();
        if (n >= types.size()) {
            throw DeadlyImportError((format(),
                "BlenderDNA: Invalid type index in structure name" ,n,
                " (there are only ", types.size(), " entries)"
            ));
        }

        // maintain separate indexes
        dna.indices[types[n].name] = dna.structures.size();

        dna.structures.push_back(Structure());
        Structure& s = dna.structures.back();
        s.name  = types[n].name;
        //s.index = dna.structures.size()-1;

        n = stream.GetI2();
        s.fields.reserve(n);

        size_t offset = 0;
        for (size_t m = 0; m < n; ++m, ++fields) {

            uint16_t j = stream.GetI2();
            if (j >= types.size()) {
                throw DeadlyImportError((format(),
                    "BlenderDNA: Invalid type index in structure field ", j,
                    " (there are only ", types.size(), " entries)"
                ));
            }
            s.fields.push_back(Field());
            Field& f = s.fields.back();
            f.offset = offset;

            f.type = types[j].name;
            f.size = types[j].size;

            j = stream.GetI2();
            if (j >= names.size()) {
                throw DeadlyImportError((format(),
                    "BlenderDNA: Invalid name index in structure field ", j,
                    " (there are only ", names.size(), " entries)"
                ));
            }

            f.name = names[j];
            f.flags = 0u;

            // pointers always specify the size of the pointee instead of their own.
            // The pointer asterisk remains a property of the lookup name.
            if (f.name[0] == '*') {
                f.size = db.i64bit ? 8 : 4;
                f.flags |= FieldFlag_Pointer;
            }

            // arrays, however, specify the size of a single element so we
            // need to parse the (possibly multi-dimensional) array declaration
            // in order to obtain the actual size of the array in the file.
            // Also we need to alter the lookup name to include no array
            // brackets anymore or size fixup won't work (if our size does
            // not match the size read from the DNA).
            if (*f.name.rbegin() == ']') {
                const std::string::size_type rb = f.name.find('[');
                if (rb == std::string::npos) {
                    throw DeadlyImportError((format(),
                        "BlenderDNA: Encountered invalid array declaration ",
                        f.name
                    ));
                }

                f.flags |= FieldFlag_Array;
                DNA::ExtractArraySize(f.name,f.array_sizes);
                f.name = f.name.substr(0,rb);

                f.size *= f.array_sizes[0] * f.array_sizes[1];
            }

            // maintain separate indexes
            s.indices[f.name] = s.fields.size()-1;
            offset += f.size;
        }
        s.size = offset;
    }

    DefaultLogger::get()->debug((format(),"BlenderDNA: Got ",dna.structures.size(),
        " structures with totally ",fields," fields"));

#ifdef ASSIMP_BUILD_BLENDER_DEBUG
    dna.DumpToFile();
#endif

    dna.AddPrimitiveStructures();
    dna.RegisterConverters();
}


#ifdef ASSIMP_BUILD_BLENDER_DEBUG

#include <fstream>
// ------------------------------------------------------------------------------------------------
void DNA :: DumpToFile()
{
    // we dont't bother using the VFS here for this is only for debugging.
    // (and all your bases are belong to us).

    std::ofstream f("dna.txt");
    if (f.fail()) {
        DefaultLogger::get()->error("Could not dump dna to dna.txt");
        return;
    }
    f << "Field format: type name offset size" << "\n";
    f << "Structure format: name size" << "\n";

    for(const Structure& s : structures) {
        f << s.name << " " << s.size << "\n\n";
        for(const Field& ff : s.fields) {
            f << "\t" << ff.type << " " << ff.name << " " << ff.offset << " " << ff.size << "\n";
        }
        f << "\n";
    }
    f << std::flush;

    DefaultLogger::get()->info("BlenderDNA: Dumped dna to dna.txt");
}
#endif

// ------------------------------------------------------------------------------------------------
/*static*/ void  DNA :: ExtractArraySize(
    const std::string& out,
    size_t array_sizes[2]
)
{
    array_sizes[0] = array_sizes[1] = 1;
    std::string::size_type pos = out.find('[');
    if (pos++ == std::string::npos) {
        return;
    }
    array_sizes[0] = strtoul10(&out[pos]);

    pos = out.find('[',pos);
    if (pos++ == std::string::npos) {
        return;
    }
    array_sizes[1] = strtoul10(&out[pos]);
}

// ------------------------------------------------------------------------------------------------
std::shared_ptr< ElemBase > DNA :: ConvertBlobToStructure(
    const Structure& structure,
    const FileDatabase& db
) const
{
    std::map<std::string, FactoryPair >::const_iterator it = converters.find(structure.name);
    if (it == converters.end()) {
        return std::shared_ptr< ElemBase >();
    }

    std::shared_ptr< ElemBase > ret = (structure.*((*it).second.first))();
    (structure.*((*it).second.second))(ret,db);

    return ret;
}

// ------------------------------------------------------------------------------------------------
DNA::FactoryPair DNA :: GetBlobToStructureConverter(
    const Structure& structure,
    const FileDatabase& /*db*/
) const
{
    std::map<std::string,  FactoryPair>::const_iterator it = converters.find(structure.name);
    return it == converters.end() ? FactoryPair() : (*it).second;
}

// basing on http://www.blender.org/development/architecture/notes-on-sdna/
// ------------------------------------------------------------------------------------------------
void DNA :: AddPrimitiveStructures()
{
    // NOTE: these are just dummies. Their presence enforces
    // Structure::Convert<target_type> to be called on these
    // empty structures. These converters are special
    // overloads which scan the name of the structure and
    // perform the required data type conversion if one
    // of these special names is found in the structure
    // in question.

    indices["int"] = structures.size();
    structures.push_back( Structure() );
    structures.back().name = "int";
    structures.back().size = 4;

    indices["short"] = structures.size();
    structures.push_back( Structure() );
    structures.back().name = "short";
    structures.back().size = 2;


    indices["char"] = structures.size();
    structures.push_back( Structure() );
    structures.back().name = "char";
    structures.back().size = 1;


    indices["float"] = structures.size();
    structures.push_back( Structure() );
    structures.back().name = "float";
    structures.back().size = 4;


    indices["double"] = structures.size();
    structures.push_back( Structure() );
    structures.back().name = "double";
    structures.back().size = 8;

    // no long, seemingly.
}

// ------------------------------------------------------------------------------------------------
void SectionParser :: Next()
{
    stream.SetCurrentPos(current.start + current.size);

    const char tmp[] = {
        (const char)stream.GetI1(),
        (const char)stream.GetI1(),
        (const char)stream.GetI1(),
        (const char)stream.GetI1()
    };
    current.id = std::string(tmp,tmp[3]?4:tmp[2]?3:tmp[1]?2:1);

    current.size = stream.GetI4();
    current.address.val = ptr64 ? stream.GetU8() : stream.GetU4();

    current.dna_index = stream.GetI4();
    current.num = stream.GetI4();

    current.start = stream.GetCurrentPos();
    if (stream.GetRemainingSizeToLimit() < current.size) {
        throw DeadlyImportError("BLEND: invalid size of file block");
    }

#ifdef ASSIMP_BUILD_BLENDER_DEBUG
    DefaultLogger::get()->debug(current.id);
#endif
}



#endif
