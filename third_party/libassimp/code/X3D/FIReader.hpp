/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team


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
/// \file   FIReader.hpp
/// \brief  Reader for Fast Infoset encoded binary XML files.
/// \date   2017
/// \author Patrick Daehne

#ifndef INCLUDED_AI_FI_READER_H
#define INCLUDED_AI_FI_READER_H

#ifndef ASSIMP_BUILD_NO_X3D_IMPORTER

//#include <wchar.h>
#include <string>
#include <memory>
#include <cerrno>
#include <cwchar>
#include <vector>
//#include <stdio.h>
//#include <cstdint>
#ifdef ASSIMP_USE_HUNTER
#  include <irrXML/irrXML.h>
#else
#  include <irrXML.h>
#endif

namespace Assimp {

struct FIValue {
    virtual const std::string &toString() const = 0;
    virtual ~FIValue() {}
};

struct FIStringValue: public FIValue {
    std::string value;
    static std::shared_ptr<FIStringValue> create(std::string &&value);
};

struct FIByteValue: public FIValue {
    std::vector<uint8_t> value;
};

struct FIHexValue: public FIByteValue {
    static std::shared_ptr<FIHexValue> create(std::vector<uint8_t> &&value);
};

struct FIBase64Value: public FIByteValue {
    static std::shared_ptr<FIBase64Value> create(std::vector<uint8_t> &&value);
};

struct FIShortValue: public FIValue {
    std::vector<int16_t> value;
    static std::shared_ptr<FIShortValue> create(std::vector<int16_t> &&value);
};

struct FIIntValue: public FIValue {
    std::vector<int32_t> value;
    static std::shared_ptr<FIIntValue> create(std::vector<int32_t> &&value);
};

struct FILongValue: public FIValue {
    std::vector<int64_t> value;
    static std::shared_ptr<FILongValue> create(std::vector<int64_t> &&value);
};

struct FIBoolValue: public FIValue {
    std::vector<bool> value;
    static std::shared_ptr<FIBoolValue> create(std::vector<bool> &&value);
};

struct FIFloatValue: public FIValue {
    std::vector<float> value;
    static std::shared_ptr<FIFloatValue> create(std::vector<float> &&value);
};

struct FIDoubleValue: public FIValue {
    std::vector<double> value;
    static std::shared_ptr<FIDoubleValue> create(std::vector<double> &&value);
};

struct FIUUIDValue: public FIByteValue {
    static std::shared_ptr<FIUUIDValue> create(std::vector<uint8_t> &&value);
};

struct FICDATAValue: public FIStringValue {
    static std::shared_ptr<FICDATAValue> create(std::string &&value);
};

struct FIDecoder {
    virtual std::shared_ptr<const FIValue> decode(const uint8_t *data, size_t len) = 0;
    virtual ~FIDecoder() {}
};

struct FIQName {
    const char *name;
    const char *prefix;
    const char *uri;
};

struct FIVocabulary {
    const char **restrictedAlphabetTable;
    size_t restrictedAlphabetTableSize;
    const char **encodingAlgorithmTable;
    size_t encodingAlgorithmTableSize;
    const char **prefixTable;
    size_t prefixTableSize;
    const char **namespaceNameTable;
    size_t namespaceNameTableSize;
    const char **localNameTable;
    size_t localNameTableSize;
    const char **otherNCNameTable;
    size_t otherNCNameTableSize;
    const char **otherURITable;
    size_t otherURITableSize;
    const std::shared_ptr<const FIValue> *attributeValueTable;
    size_t attributeValueTableSize;
    const std::shared_ptr<const FIValue> *charactersTable;
    size_t charactersTableSize;
    const std::shared_ptr<const FIValue> *otherStringTable;
    size_t otherStringTableSize;
    const FIQName *elementNameTable;
    size_t elementNameTableSize;
    const FIQName *attributeNameTable;
    size_t attributeNameTableSize;
};

class IOStream;

class FIReader: public irr::io::IIrrXMLReader<char, irr::io::IXMLBase> {
public:
	virtual ~FIReader();
    virtual std::shared_ptr<const FIValue> getAttributeEncodedValue(int idx) const = 0;

    virtual std::shared_ptr<const FIValue> getAttributeEncodedValue(const char *name) const = 0;

    virtual void registerDecoder(const std::string &algorithmUri, std::unique_ptr<FIDecoder> decoder) = 0;

    virtual void registerVocabulary(const std::string &vocabularyUri, const FIVocabulary *vocabulary) = 0;

    static std::unique_ptr<FIReader> create(IOStream *stream);

};// class IFIReader

inline
FIReader::~FIReader() {
	// empty
}

}// namespace Assimp

#endif // #ifndef ASSIMP_BUILD_NO_X3D_IMPORTER

#endif // INCLUDED_AI_FI_READER_H
