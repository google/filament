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

/** @file FileSystemFilter.h
 *  Implements a filter system to filter calls to Exists() and Open()
 *  in order to improve the success rate of file opening ...
 */
#ifndef AI_FILESYSTEMFILTER_H_INC
#define AI_FILESYSTEMFILTER_H_INC

#include "../include/assimp/IOSystem.hpp"
#include "../include/assimp/DefaultLogger.hpp"
#include "fast_atof.h"
#include "ParsingUtils.h"

namespace Assimp    {

inline bool IsHex(char s) {
    return (s>='0' && s<='9') || (s>='a' && s<='f') || (s>='A' && s<='F');
}

// ---------------------------------------------------------------------------
/** File system filter
 */
class FileSystemFilter : public IOSystem
{
public:
    /** Constructor. */
    FileSystemFilter(const std::string& file, IOSystem* old)
        : wrapped  (old)
        , src_file (file)
        , sep(wrapped->getOsSeparator())
    {
        ai_assert(NULL != wrapped);

        // Determine base directory
        base = src_file;
        std::string::size_type ss2;
        if (std::string::npos != (ss2 = base.find_last_of("\\/")))  {
            base.erase(ss2,base.length()-ss2);
        }
        else {
            base = "";
        //  return;
        }

        // make sure the directory is terminated properly
        char s;

        if (base.length() == 0) {
            base = ".";
            base += getOsSeparator();
        }
        else if ((s = *(base.end()-1)) != '\\' && s != '/') {
            base += getOsSeparator();
        }

        DefaultLogger::get()->info("Import root directory is \'" + base + "\'");
    }

    /** Destructor. */
    ~FileSystemFilter()
    {
        // haha
    }

    // -------------------------------------------------------------------
    /** Tests for the existence of a file at the given path. */
    bool Exists( const char* pFile) const
    {
        std::string tmp = pFile;

        // Currently this IOSystem is also used to open THE ONE FILE.
        if (tmp != src_file)    {
            BuildPath(tmp);
            Cleanup(tmp);
        }

        return wrapped->Exists(tmp);
    }

    // -------------------------------------------------------------------
    /** Returns the directory separator. */
    char getOsSeparator() const
    {
        return sep;
    }

    // -------------------------------------------------------------------
    /** Open a new file with a given path. */
    IOStream* Open( const char* pFile, const char* pMode = "rb")
    {
        ai_assert(pFile);
        ai_assert(pMode);

        // First try the unchanged path
        IOStream* s = wrapped->Open(pFile,pMode);

        if (!s) {
            std::string tmp = pFile;

            // Try to convert between absolute and relative paths
            BuildPath(tmp);
            s = wrapped->Open(tmp,pMode);

            if (!s) {
                // Finally, look for typical issues with paths
                // and try to correct them. This is our last
                // resort.
                tmp = pFile;
                Cleanup(tmp);
                BuildPath(tmp);
                s = wrapped->Open(tmp,pMode);
            }
        }

        return s;
    }

    // -------------------------------------------------------------------
    /** Closes the given file and releases all resources associated with it. */
    void Close( IOStream* pFile)
    {
        return wrapped->Close(pFile);
    }

    // -------------------------------------------------------------------
    /** Compare two paths */
    bool ComparePaths (const char* one, const char* second) const
    {
        return wrapped->ComparePaths (one,second);
    }

private:

    // -------------------------------------------------------------------
    /** Build a valid path from a given relative or absolute path.
     */
    void BuildPath (std::string& in) const
    {
        // if we can already access the file, great.
        if (in.length() < 3 || wrapped->Exists(in)) {
            return;
        }

        // Determine whether this is a relative path (Windows-specific - most assets are packaged on Windows).
        if (in[1] != ':') {

            // append base path and try
            const std::string tmp = base + in;
            if (wrapped->Exists(tmp)) {
                in = tmp;
                return;
            }
        }

        // Chop of the file name and look in the model directory, if
        // this fails try all sub paths of the given path, i.e.
        // if the given path is foo/bar/something.lwo, try
        // <base>/something.lwo
        // <base>/bar/something.lwo
        // <base>/foo/bar/something.lwo
        std::string::size_type pos = in.rfind('/');
        if (std::string::npos == pos) {
            pos = in.rfind('\\');
        }

        if (std::string::npos != pos)   {
            std::string tmp;
            std::string::size_type last_dirsep = std::string::npos;

            while(true) {
                tmp = base;
                tmp += sep;

                std::string::size_type dirsep = in.rfind('/', last_dirsep);
                if (std::string::npos == dirsep) {
                    dirsep = in.rfind('\\', last_dirsep);
                }

                if (std::string::npos == dirsep || dirsep == 0) {
                    // we did try this already.
                    break;
                }

                last_dirsep = dirsep-1;

                tmp += in.substr(dirsep+1, in.length()-pos);
                if (wrapped->Exists(tmp)) {
                    in = tmp;
                    return;
                }
            }
        }

        // hopefully the underlying file system has another few tricks to access this file ...
    }

    // -------------------------------------------------------------------
    /** Cleanup the given path
     */
    void Cleanup (std::string& in) const
    {
        char last = 0;
        if(in.empty()) {
            return;
        }

        // Remove a very common issue when we're parsing file names: spaces at the
        // beginning of the path.
        std::string::iterator it = in.begin();
        while (IsSpaceOrNewLine( *it ))++it;
        if (it != in.begin()) {
            in.erase(in.begin(),it+1);
        }

        const char sep = getOsSeparator();
        for (it = in.begin(); it != in.end(); ++it) {
            // Exclude :// and \\, which remain untouched.
            // https://sourceforge.net/tracker/?func=detail&aid=3031725&group_id=226462&atid=1067632
            if ( !strncmp(&*it, "://", 3 )) {
                it += 3;
                continue;
            }
            if (it == in.begin() && !strncmp(&*it, "\\\\", 2)) {
                it += 2;
                continue;
            }

            // Cleanup path delimiters
            if (*it == '/' || (*it) == '\\') {
                *it = sep;

                // And we're removing double delimiters, frequent issue with
                // incorrectly composited paths ...
                if (last == *it) {
                    it = in.erase(it);
                    --it;
                }
            }
            else if (*it == '%' && in.end() - it > 2) {

                // Hex sequence in URIs
                if( IsHex((&*it)[0]) && IsHex((&*it)[1]) ) {
                    *it = HexOctetToDecimal(&*it);
                    it = in.erase(it+1,it+2);
                    --it;
                }
            }

            last = *it;
        }
    }

private:
    IOSystem* wrapped;
    std::string src_file, base;
    char sep;
};

} //!ns Assimp

#endif //AI_DEFAULTIOSYSTEM_H_INC
