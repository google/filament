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

/** @file  DXFHelper.h
 *  @brief Internal utilities for the DXF loader.
 */

#ifndef INCLUDED_DXFHELPER_H
#define INCLUDED_DXFHELPER_H

#include "LineSplitter.h"
#include "TinyFormatter.h"
#include "StreamReader.h"
#include "fast_atof.h"
#include <vector>
#include <assimp/DefaultLogger.hpp>

namespace Assimp {
    namespace DXF {


// read pairs of lines, parse group code and value and provide utilities
// to convert the data to the target data type.
class LineReader
{

public:

    LineReader(StreamReaderLE& reader)
         // do NOT skip empty lines. In DXF files, they count as valid data.
        : splitter(reader,false,true)
        , groupcode( 0 )
        , value()
        , end()
    {
    }

public:


    // -----------------------------------------
    bool Is(int gc, const char* what) const {
        return groupcode == gc && !strcmp(what,value.c_str());
    }

    // -----------------------------------------
    bool Is(int gc) const {
        return groupcode == gc;
    }

    // -----------------------------------------
    int GroupCode() const {
        return groupcode;
    }

    // -----------------------------------------
    const std::string& Value() const {
        return value;
    }

    // -----------------------------------------
    bool End() const {
        return !((bool)*this);
    }

public:

    // -----------------------------------------
    unsigned int ValueAsUnsignedInt() const {
        return strtoul10(value.c_str());
    }

    // -----------------------------------------
    int ValueAsSignedInt() const {
        return strtol10(value.c_str());
    }

    // -----------------------------------------
    float ValueAsFloat() const {
        return fast_atof(value.c_str());
    }

public:

    // -----------------------------------------
    /** pseudo-iterator increment to advance to the next (groupcode/value) pair */
    LineReader& operator++() {
        if (end) {
            if (end == 1) {
                ++end;
            }
            return *this;
        }

        try {
            groupcode = strtol10(splitter->c_str());
            splitter++;

            value = *splitter;
            splitter++;

            // automatically skip over {} meta blocks (these are for application use
            // and currently not relevant for Assimp).
            if (value.length() && value[0] == '{') {

                size_t cnt = 0;
                for(;splitter->length() && splitter->at(0) != '}'; splitter++, cnt++);

                splitter++;
                DefaultLogger::get()->debug((Formatter::format("DXF: skipped over control group ("),cnt," lines)"));
            }
        } catch(std::logic_error&) {
            ai_assert(!splitter);
        }
        if (!splitter) {
            end = 1;
        }
        return *this;
    }

    // -----------------------------------------
    LineReader& operator++(int) {
        return ++(*this);
    }


    // -----------------------------------------
    operator bool() const {
        return end <= 1;
    }

private:

    LineSplitter splitter;
    int groupcode;
    std::string value;
    int end;
};



// represents a POLYLINE or a LWPOLYLINE. or even a 3DFACE The data is converted as needed.
struct PolyLine
{
    PolyLine()
        : flags()
    {}

    std::vector<aiVector3D> positions;
    std::vector<aiColor4D>  colors;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> counts;
    unsigned int flags;

    std::string layer;
    std::string desc;
};


// reference to a BLOCK. Specifies its own coordinate system.
struct InsertBlock
{
    InsertBlock()
        : scale(1.f,1.f,1.f)
        , angle()
    {}

    aiVector3D pos;
    aiVector3D scale;
    float angle;

    std::string name;
};


// keeps track of all geometry in a single BLOCK.
struct Block
{
    std::vector< std::shared_ptr<PolyLine> > lines;
    std::vector<InsertBlock> insertions;

    std::string name;
    aiVector3D base;
};


struct FileData
{
    // note: the LAST block always contains the stuff from ENTITIES.
    std::vector<Block> blocks;
};





}}
#endif
