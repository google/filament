/*-----------------------------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2014-2015 Kim Kulling

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-----------------------------------------------------------------------------------------------*/
#pragma once

#include <cstddef>
#include <vector>
#include <string>

#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#  include <inttypes.h>
#endif

#if defined(_MSC_VER) && !defined( OPENDDL_STATIC_LIBARY )

#   define TAG_DLL_EXPORT __declspec(dllexport)
#   define TAG_DLL_IMPORT __declspec(dllimport )
#   ifdef OPENDDLPARSER_BUILD
#       define DLL_ODDLPARSER_EXPORT TAG_DLL_EXPORT
#   else
#       define DLL_ODDLPARSER_EXPORT TAG_DLL_IMPORT
#   endif // OPENDDLPARSER_BUILD
#   pragma warning( disable : 4251 )
#else
#   define DLL_ODDLPARSER_EXPORT
#endif // _WIN32

// Namespace declarations, override this to avoid any conflicts
#define BEGIN_ODDLPARSER_NS namespace ODDLParser {
#define END_ODDLPARSER_NS   } // namespace ODDLParser
#define USE_ODDLPARSER_NS   using namespace ODDLParser;

BEGIN_ODDLPARSER_NS

// We will use C++11 optional
#ifndef OPENDDL_NO_USE_CPP11
    // All C++11 constructs
#   define ddl_nullptr nullptr
#   define ddl_override override
#   define ddl_final final
#   define ddl_no_copy = delete
#else
    // Fall-back for older compilers
#   define ddl_nullptr NULL
#   define ddl_override
#   define ddl_final
#   define ddl_no_copy
#endif // OPENDDL_NO_USE_CPP11

// Forward declarations
class DDLNode;
class Value;

struct Name;
struct Identifier;
struct Reference;
struct Property;
struct DataArrayList;

// Platform-specific typedefs
#ifdef _WIN32
typedef signed __int64    int64_impl;
typedef unsigned __int64  uint64_impl;
#else
typedef int64_t           int64_impl;
typedef uint64_t          uint64_impl;
#endif

// OpenDDL-specific data typedefs
typedef signed char       int8;    ///< Signed integer, 1 byte
typedef signed short      int16;   ///< Signed integer, 2 byte
typedef signed int        int32;   ///< Signed integer, 4 byte
typedef int64_impl        int64;   ///< Signed integer, 8 byte
typedef unsigned char     uint8;   ///< Unsigned integer, 1 byte
typedef unsigned short    uint16;  ///< Unsigned integer, 2 byte
typedef unsigned int      uint32;  ///< Unsigned integer, 4 byte
typedef uint64_impl       uint64;  ///< Unsigned integer, 8 byte

///	@brief  Stores a text.
///
/// A text is stored in a simple character buffer. Texts buffer can be
/// greater than the number of stored characters in them.
struct DLL_ODDLPARSER_EXPORT Text {
    size_t m_capacity;  ///< The capacity of the text.
    size_t m_len;       ///< The length of the text.
    char *m_buffer;     ///< The buffer with the text.

    ///	@brief  The constructor with a given text buffer.
    /// @param  buffer      [in] The buffer.
    /// @param  numChars    [in] The number of characters in the buffer.
    Text( const char *buffer, size_t numChars );

    ///	@brief  The destructor.
    ~Text();

    ///	@brief  Clears the text.
    void clear();

    ///	@brief  Set a new text.
    /// @param  buffer      [in] The buffer.
    /// @param  numChars    [in] The number of characters in the buffer.
    void set( const char *buffer, size_t numChars );

    ///	@brief  The compare operator for std::strings.
    bool operator == ( const std::string &name ) const;

    ///	@brief  The compare operator for Texts.
    bool operator == ( const Text &rhs ) const;

private:
    Text( const Text & ) ddl_no_copy;
    Text &operator = ( const Text & ) ddl_no_copy;
};

///	@brief  Description of the type of a name.
enum NameType {
    GlobalName, ///< Name is global.
    LocalName   ///< Name is local.
};

///	@brief  Stores an OpenDDL-specific name
struct DLL_ODDLPARSER_EXPORT Name {
    NameType    m_type; ///< The type of the name ( @see NameType ).
    Text        *m_id;  ///< The id.

    ///	@brief  The constructor with the type and the id.
    ///	@param  type    [in] The name type.
    ///	@param  id      [in] The id.
    Name( NameType type, Text *id );
    Name( const Name &name );
    ///	@brief  The destructor.
    ~Name();

private:

    Name &operator = ( const Name& ) ddl_no_copy;
};

///	@brief  Stores a bundle of references.
struct DLL_ODDLPARSER_EXPORT Reference {
    size_t   m_numRefs;         ///< The number of stored references.
    Name   **m_referencedName;  ///< The reference names.

    ///	@brief  The default constructor.
    Reference();
    Reference( const Reference &ref );
    ///	@brief  The constructor with an array of ref names.
    /// @param  numrefs     [in] The number of ref names.
    /// @param  names       [in] The ref names.
    Reference( size_t numrefs, Name **names );

    ///	@brief  The destructor.
    ~Reference();

    /// @brief  Returns the size in bytes to store one deep reference copy.
    /// @return The size on bytes.
    size_t sizeInBytes();

private:
    Reference &operator = ( const Reference & ) ddl_no_copy;
};

///	@brief  Stores a property list.
struct DLL_ODDLPARSER_EXPORT Property {
    Text       *m_key;      ///< The identifier / key of the property.
    Value      *m_value;    ///< The value assigned to its key / id ( ddl_nullptr if none ).
    Reference  *m_ref;      ///< References assigned to its key / id ( ddl_nullptr if none ).
    Property   *m_next;     ///< The next property ( ddl_nullptr if none ).

    ///	@brief  The default constructor.
    Property();

    ///	@brief  The constructor for initialization.
    /// @param  id      [in] The identifier
    Property( Text *id );

    ///	@brief  The destructor.
    ~Property();

private:
    Property( const Property & ) ddl_no_copy;
    Property &operator = ( const Property & ) ddl_no_copy;
};

///	@brief  Stores a data array list.
struct DLL_ODDLPARSER_EXPORT DataArrayList {
    size_t         m_numItems;  ///< The number of items in the list.
    Value         *m_dataList;  ///< The data list ( a Value ).
    DataArrayList *m_next;      ///< The next data array list ( ddl_nullptr if last ).
    Reference     *m_refs;
    size_t         m_numRefs;

    ///	@brief  The default constructor for initialization.
    DataArrayList();

    ///	@brief  The destructor.
    ~DataArrayList();

    /// @brief  Gets the length of the array
    size_t size();

private:
    DataArrayList( const DataArrayList & ) ddl_no_copy;
    DataArrayList &operator = ( const DataArrayList & ) ddl_no_copy;
};

///	@brief  Stores the context of a parsed OpenDDL declaration.
struct DLL_ODDLPARSER_EXPORT Context {
    DDLNode *m_root;    ///< The root node of the OpenDDL node tree.

    ///	@brief  Constructor for initialization.
    Context();

    ///	@brief  Destructor.
    ~Context();

    ///	@brief  Clears the whole node tree.
    void clear();

private:
    Context( const Context & ) ddl_no_copy;
    Context &operator = ( const Context & ) ddl_no_copy;
};

END_ODDLPARSER_NS
