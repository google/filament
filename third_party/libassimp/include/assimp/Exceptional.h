/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2008, assimp team
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

#ifndef INCLUDED_EXCEPTIONAL_H
#define INCLUDED_EXCEPTIONAL_H

#include <stdexcept>
#include <assimp/DefaultIOStream.h>
using std::runtime_error;

#ifdef _MSC_VER
#   pragma warning(disable : 4275)
#endif

// ---------------------------------------------------------------------------
/** FOR IMPORTER PLUGINS ONLY: Simple exception class to be thrown if an
 *  unrecoverable error occurs while importing. Loading APIs return
 *  NULL instead of a valid aiScene then.  */
class DeadlyImportError
    : public runtime_error
{
public:
    /** Constructor with arguments */
    explicit DeadlyImportError( const std::string& errorText)
        : runtime_error(errorText)
    {
    }

private:
};

typedef DeadlyImportError DeadlyExportError;

#ifdef _MSC_VER
#   pragma warning(default : 4275)
#endif

// ---------------------------------------------------------------------------
template <typename T>
struct ExceptionSwallower   {
    T operator ()() const {
        return T();
    }
};

// ---------------------------------------------------------------------------
template <typename T>
struct ExceptionSwallower<T*>   {
    T* operator ()() const {
        return NULL;
    }
};

// ---------------------------------------------------------------------------
template <>
struct ExceptionSwallower<aiReturn> {
    aiReturn operator ()() const {
        try {
            throw;
        }
        catch (std::bad_alloc&) {
            return aiReturn_OUTOFMEMORY;
        }
        catch (...) {
            return aiReturn_FAILURE;
        }
    }
};

// ---------------------------------------------------------------------------
template <>
struct ExceptionSwallower<void> {
    void operator ()() const {
        return;
    }
};

#define ASSIMP_BEGIN_EXCEPTION_REGION()\
{\
    try {

#define ASSIMP_END_EXCEPTION_REGION(type)\
    } catch(...) {\
        return ExceptionSwallower<type>()();\
    }\
}

#endif // INCLUDED_EXCEPTIONAL_H
