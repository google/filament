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

#include <openddlparser/OpenDDLCommon.h>

#include <string>

BEGIN_ODDLPARSER_NS

// Forward declarations
struct ValueAllocator;

class IOStreamBase;

///------------------------------------------------------------------------------------------------
///	@brief  This class implements a value.
///
///	Values are used to store data types like boolean, integer, floats, double and many mode. To get
///	an overview please check the enum VylueType ( @see Value::ValueType ).
/// Values can be single items or lists of items. They are implemented as linked lists.
///------------------------------------------------------------------------------------------------
class DLL_ODDLPARSER_EXPORT Value {
    friend struct ValueAllocator;

public:
    ///	@brief  This class implements an iterator through a Value list.
    ///
    /// When getting a new value you need to know how to iterate through it. The Value::Iterator
    /// will help you here:
    ///	@code
    /// Value *val = node->getValue();
    /// Value::Iterator it( val );
    /// while( it.hasNext() ) {
    ///     Value v( it.getNext );
    /// }
    /// @endcode
    class DLL_ODDLPARSER_EXPORT Iterator {
    public:
        ///	@brief  The default class constructor.
        Iterator();

        ///	@brief  The class constructor with the start value.
        /// @param  start   [in] The first value for iteration,
        Iterator( Value *start );

        Iterator( const Iterator &rhs );

        ///	@brief  The class destructor.
        ~Iterator();

        ///	@brief  Will return true, if another value is in the list.
        /// @return true if another value is there.
        bool hasNext() const;

        ///	@brief  Returns the next item and moves the iterator to it.
        ///	@return The next value, is ddl_nullptr in case of being the last item.
        Value *getNext();

        ///	@brief  The post-increment operator.
        const Iterator operator++( int );

        ///	@brief  The pre-increment operator.
        Iterator &operator++( );

        ///	@brief  The compare operator.
        /// @param  rhs [in] The instance to compare.
        /// @return true if equal.
        bool operator == ( const Iterator &rhs ) const;

        /// @brief  The * operator.
        /// @return The instance or ddl_nullptr if end of list is reached.
        Value *operator->( ) const;

    private:
        Value *m_start;
        Value *m_current;

    private:
        Iterator &operator = ( const Iterator & );
    };

    ///	@brief  This enum describes the data type stored in the value.
    enum ValueType {
        ddl_none = -1,          ///< Nothing specified
        ddl_bool = 0,           ///< A boolean type
        ddl_int8,               ///< Integer type, 8 bytes
        ddl_int16,              ///< Integer type, 16 bytes
        ddl_int32,              ///< Integer type, 32 bytes
        ddl_int64,              ///< Integer type, 64 bytes
        ddl_unsigned_int8,      ///< Unsigned integer type, 8 bytes
        ddl_unsigned_int16,     ///< Unsigned integer type, 16 bytes
        ddl_unsigned_int32,     ///< Unsigned integer type, 32 bytes
        ddl_unsigned_int64,     ///< Unsigned integer type, 64 bytes
        ddl_half,               ///< Half data type.
        ddl_float,              ///< float data type
        ddl_double,             ///< Double data type.
        ddl_string,             ///< String data type.
        ddl_ref,                ///< Reference, used to define references to other data definitions.
        ddl_types_max           ///< Upper limit.
    };

    ///	@brief  The class constructor.
    /// @param  type        [in] The value type.
    Value( ValueType type );

    ///	@brief  The class destructor.
    ~Value();

    ///	@brief  Assigns a boolean to the value.
    /// @param  value       [in9 The value.
    void setBool( bool value );

    ///	@brief  Returns the boolean value.
    /// @return The boolean value.
    bool getBool();

    ///	@brief  Assigns a int8 to the value.
    /// @param  value       [in] The value.
    void setInt8( int8 value );

    ///	@brief  Returns the int8 value.
    /// @return The int8 value.
    int8 getInt8();

    ///	@brief  Assigns a int16 to the value.
    /// @param  value       [in] The value.
    void setInt16( int16 value );

    ///	@brief  Returns the int16 value.
    /// @return The int16 value.
    int16 getInt16();

    ///	@brief  Assigns a int32 to the value.
    /// @param  value       [in] The value.
    void setInt32( int32 value );

    ///	@brief  Returns the int16 value.
    /// @return The int32 value.
    int32 getInt32();

    ///	@brief  Assigns a int64 to the value.
    /// @param  value       [in] The value.
    void setInt64( int64 value );

    ///	@brief  Returns the int16 value.
    /// @return The int64 value.
    int64 getInt64();

    ///	@brief  Assigns a unsigned int8 to the value.
    /// @param  value       [in] The value.
    void setUnsignedInt8( uint8 value );

    ///	@brief  Returns the unsigned int8 value.
    /// @return The unsigned int8 value.
    uint8 getUnsignedInt8() const;

    ///	@brief  Assigns a unsigned int16 to the value.
    /// @param  value       [in] The value.
    void setUnsignedInt16( uint16 value );

    ///	@brief  Returns the unsigned int16 value.
    /// @return The unsigned int16 value.
    uint16 getUnsignedInt16() const;

    ///	@brief  Assigns a unsigned int32 to the value.
    /// @param  value       [in] The value.
    void setUnsignedInt32( uint32 value );

    ///	@brief  Returns the unsigned int8 value.
    /// @return The unsigned int32 value.
    uint32 getUnsignedInt32() const;

    ///	@brief  Assigns a unsigned int64 to the value.
    /// @param  value       [in] The value.
    void setUnsignedInt64( uint64 value );

    ///	@brief  Returns the unsigned int64 value.
    /// @return The unsigned int64 value.
    uint64 getUnsignedInt64() const;

    ///	@brief  Assigns a float to the value.
    /// @param  value       [in] The value.
    void setFloat( float value );

    ///	@brief  Returns the float value.
    /// @return The float value.
    float getFloat() const;

    ///	@brief  Assigns a double to the value.
    /// @param  value       [in] The value.
    void setDouble( double value );

    ///	@brief  Returns the double value.
    /// @return The double value.
    double getDouble() const;

    ///	@brief  Assigns a std::string to the value.
    /// @param  str         [in] The value.
    void setString( const std::string &str );

    ///	@brief  Returns the std::string value.
    /// @return The std::string value.
    const char *getString() const;

    /// @brief  Set the reference.
    /// @param  ref     [in] Pointer showing to the reference.
    void setRef( Reference *ref );

    /// @brief  Returns the pointer showing to the reference.
    /// @return Pointer showing to the reference.
    Reference *getRef() const;

    ///	@brief  Dumps the value.
    /// @param  stream  [in] The stream to write in.
    void dump( IOStreamBase &stream );

    ///	@brief  Assigns the next value.
    ///	@param  next        [n] The next value.
    void setNext( Value *next );

    ///	@brief  Returns the next value.
    /// @return The next value.s
    Value *getNext() const;

    /// @brief  Gets the length of the array.
    /// @return The number of items in the array.
    size_t size() const;

    ValueType m_type;
    size_t m_size;
    unsigned char *m_data;
    Value *m_next;

private:
    Value &operator =( const Value & ) ddl_no_copy;
    Value( const Value  & ) ddl_no_copy;
};

///------------------------------------------------------------------------------------------------
///	@brief  This class implements the value allocator.
///------------------------------------------------------------------------------------------------
struct DLL_ODDLPARSER_EXPORT ValueAllocator {
    static Value *allocPrimData( Value::ValueType type, size_t len = 1 );
    static void releasePrimData( Value **data );

private:
    ValueAllocator() ddl_no_copy;
    ValueAllocator( const ValueAllocator  & ) ddl_no_copy;
    ValueAllocator &operator = ( const ValueAllocator & ) ddl_no_copy;
};

END_ODDLPARSER_NS
