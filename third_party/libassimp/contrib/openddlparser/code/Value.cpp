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
#include <openddlparser/Value.h>

#include <iostream>
#include <cassert>

BEGIN_ODDLPARSER_NS

static Value::Iterator end( ddl_nullptr );

Value::Iterator::Iterator()
: m_start( ddl_nullptr )
, m_current( ddl_nullptr ) {
    // empty
}

Value::Iterator::Iterator( Value *start )
: m_start( start )
, m_current( start ) {
    // empty
}

Value::Iterator::Iterator( const Iterator &rhs )
: m_start( rhs.m_start )
, m_current( rhs.m_current ) {
    // empty
}

Value::Iterator::~Iterator() {
    // empty
}

bool Value::Iterator::hasNext() const {
    if( ddl_nullptr == m_current ) {
        return false;
    }
    return ( ddl_nullptr != m_current->getNext() );
}

Value *Value::Iterator::getNext() {
    if( !hasNext() ) {
        return ddl_nullptr;
    }

    Value *v( m_current->getNext() );
    m_current = v;

    return v;
}

const Value::Iterator Value::Iterator::operator++( int ) {
    if( ddl_nullptr == m_current ) {
        return end;
    }

    m_current = m_current->getNext();
    Iterator inst( m_current );

    return inst;
}

Value::Iterator &Value::Iterator::operator++( ) {
    if( ddl_nullptr == m_current ) {
        return end;
    }

    m_current = m_current->getNext();

    return *this;
}

bool Value::Iterator::operator == ( const Iterator &rhs ) const {
    return ( m_current == rhs.m_current );
}

Value *Value::Iterator::operator->( ) const {
    if(ddl_nullptr == m_current ) {
        return ddl_nullptr;
    }
    return m_current;
}

Value::Value( ValueType type )
: m_type( type )
, m_size( 0 )
, m_data( ddl_nullptr )
, m_next( ddl_nullptr ) {
    // empty
}

Value::~Value() {
    if(m_data!=ddl_nullptr) {
        if (m_type == ddl_ref ) {
            Reference *tmp = (Reference *) m_data;
            if (tmp != ddl_nullptr)
                delete tmp;
        }else
            delete[] m_data;

    }
    if(m_next!=ddl_nullptr)
        delete m_next;
}

void Value::setBool( bool value ) {
    assert( ddl_bool == m_type );
    ::memcpy( m_data, &value, m_size );
}

bool Value::getBool() {
    assert( ddl_bool == m_type );
    return ( *m_data == 1 );
}

void Value::setInt8( int8 value ) {
    assert( ddl_int8 == m_type );
    ::memcpy( m_data, &value, m_size );
}

int8 Value::getInt8() {
    assert( ddl_int8 == m_type );
    return ( int8 ) ( *m_data );
}

void Value::setInt16( int16 value ) {
    assert( ddl_int16 == m_type );
    ::memcpy( m_data, &value, m_size );
}

int16 Value::getInt16() {
    assert( ddl_int16 == m_type );
    int16 i;
    ::memcpy( &i, m_data, m_size );
    return i;
}

void Value::setInt32( int32 value ) {
    assert( ddl_int32 == m_type );
    ::memcpy( m_data, &value, m_size );
}

int32 Value::getInt32() {
    assert( ddl_int32 == m_type );
    int32 i;
    ::memcpy( &i, m_data, m_size );
    return i;
}

void Value::setInt64( int64 value ) {
    assert( ddl_int64 == m_type );
    ::memcpy( m_data, &value, m_size );
}

int64 Value::getInt64() {
    assert( ddl_int64 == m_type );
    int64 i;
    ::memcpy( &i, m_data, m_size );
    return i;
}

void Value::setUnsignedInt8( uint8 value ) {
    assert( ddl_unsigned_int8 == m_type );
    ::memcpy( m_data, &value, m_size );
}

uint8 Value::getUnsignedInt8() const {
    assert( ddl_unsigned_int8 == m_type );
    uint8 i;
    ::memcpy( &i, m_data, m_size );
    return i;
}

void Value::setUnsignedInt16( uint16 value ) {
    assert( ddl_unsigned_int16 == m_type );
    ::memcpy( m_data, &value, m_size );
}

uint16 Value::getUnsignedInt16() const {
    assert( ddl_unsigned_int16 == m_type );
    uint16 i;
    ::memcpy( &i, m_data, m_size );
    return i;
}

void Value::setUnsignedInt32( uint32 value ) {
    assert( ddl_unsigned_int32 == m_type );
    ::memcpy( m_data, &value, m_size );
}

uint32 Value::getUnsignedInt32() const {
    assert( ddl_unsigned_int32 == m_type );
    uint32 i;
    ::memcpy( &i, m_data, m_size );
    return i;
}

void Value::setUnsignedInt64( uint64 value ) {
    assert( ddl_unsigned_int64 == m_type );
    ::memcpy( m_data, &value, m_size );
}

uint64 Value::getUnsignedInt64() const {
    assert( ddl_unsigned_int64 == m_type );
    uint64 i;
    ::memcpy( &i, m_data, m_size );
    return i;
}

void Value::setFloat( float value ) {
    assert( ddl_float == m_type );
    ::memcpy( m_data, &value, m_size );
}

float Value::getFloat() const {
    if( m_type == ddl_float ) {
        float v;
        ::memcpy( &v, m_data, m_size );
        return ( float ) v;
    } else {
        float tmp;
        ::memcpy( &tmp, m_data, 4 );
        return ( float ) tmp;
    }
}

void Value::setDouble( double value ) {
    assert( ddl_double == m_type );
    ::memcpy( m_data, &value, m_size );
}

double Value::getDouble() const {
    if ( m_type == ddl_double ) {
        double v;
        ::memcpy( &v, m_data, m_size );
        return ( float ) v;
    }
    else {
        double tmp;
        ::memcpy( &tmp, m_data, 4 );
        return ( double ) tmp;
    }
}

void Value::setString( const std::string &str ) {
    assert( ddl_string == m_type );
    ::memcpy( m_data, str.c_str(), str.size() );
    m_data[ str.size() ] = '\0';
}

const char *Value::getString() const {
    assert( ddl_string == m_type );
    return (const char*) m_data;
}

void Value::setRef( Reference *ref ) {
    assert( ddl_ref == m_type );

    if ( ddl_nullptr != ref ) {
        const size_t sizeInBytes( ref->sizeInBytes() );
        if ( sizeInBytes > 0 ) {
            if ( ddl_nullptr != m_data ) {
                delete [] m_data;
            }

            m_data = (unsigned char*) new Reference(*ref);
        }
    }
}

Reference *Value::getRef() const {
    assert( ddl_ref == m_type );

    return (Reference*) m_data;
}

void Value::dump( IOStreamBase &/*stream*/ ) {
    switch( m_type ) {
        case ddl_none:
            std::cout << "None" << std::endl;
            break;
        case ddl_bool:
            std::cout << getBool() << std::endl;
            break;
        case ddl_int8:
            std::cout << getInt8() << std::endl;
            break;
        case ddl_int16:
            std::cout << getInt16() << std::endl;
            break;
        case ddl_int32:
            std::cout << getInt32() << std::endl;
            break;
        case ddl_int64:
            std::cout << getInt64() << std::endl;
            break;
        case ddl_unsigned_int8:
            std::cout << "Not supported" << std::endl;
            break;
        case ddl_unsigned_int16:
            std::cout << "Not supported" << std::endl;
            break;
        case ddl_unsigned_int32:
            std::cout << "Not supported" << std::endl;
            break;
        case ddl_unsigned_int64:
            std::cout << "Not supported" << std::endl;
            break;
        case ddl_half:
            std::cout << "Not supported" << std::endl;
            break;
        case ddl_float:
            std::cout << getFloat() << std::endl;
            break;
        case ddl_double:
            std::cout << getDouble() << std::endl;
            break;
        case ddl_string:
            std::cout << getString() << std::endl;
            break;
        case ddl_ref:
            std::cout << "Not supported" << std::endl;
            break;
        default:
            break;
    }
}

void Value::setNext( Value *next ) {
    m_next = next;
}

Value *Value::getNext() const {
    return m_next;
}

size_t Value::size() const{
    size_t result=1;
    Value *n=m_next;
    while( n!=ddl_nullptr) {
        result++;
        n=n->m_next;
    }
    return result;
}

Value *ValueAllocator::allocPrimData( Value::ValueType type, size_t len ) {
    if( type == Value::ddl_none || Value::ddl_types_max == type ) {
        return ddl_nullptr;
    }

    Value *data = new Value( type );
    switch( type ) {
        case Value::ddl_bool:
            data->m_size = sizeof( bool );
            break;
        case Value::ddl_int8:
            data->m_size = sizeof( int8 );
            break;
        case Value::ddl_int16:
            data->m_size = sizeof( int16 );
            break;
        case Value::ddl_int32:
            data->m_size = sizeof( int32 );
            break;
        case Value::ddl_int64:
            data->m_size = sizeof( int64 );
            break;
        case Value::ddl_unsigned_int8:
            data->m_size = sizeof( uint8 );
            break;
        case Value::ddl_unsigned_int16:
            data->m_size = sizeof( uint16 );
            break;
        case Value::ddl_unsigned_int32:
            data->m_size = sizeof( uint32 );
            break;
        case Value::ddl_unsigned_int64:
            data->m_size = sizeof( uint64 );
            break;
        case Value::ddl_half:
            data->m_size = sizeof( short );
            break;
        case Value::ddl_float:
            data->m_size = sizeof( float );
            break;
        case Value::ddl_double:
            data->m_size = sizeof( double );
            break;
        case Value::ddl_string:
            data->m_size = sizeof( char )*(len+1);
            break;
        case Value::ddl_ref:
            data->m_size = 0;
            break;
        case Value::ddl_none:
        case Value::ddl_types_max:
        default:
            break;
    }

    if( data->m_size ) {
        data->m_data = new unsigned char[ data->m_size ];
        ::memset(data->m_data,0,data->m_size);
    }

    return data;
}

void ValueAllocator::releasePrimData( Value **data ) {
    if( !data ) {
        return;
    }

    delete *data;
    *data = ddl_nullptr;
}

END_ODDLPARSER_NS
