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
#include <openddlparser/OpenDDLExport.h>
#include <openddlparser/DDLNode.h>
#include <openddlparser/Value.h>
#include <openddlparser/OpenDDLParser.h>

#include <sstream>

BEGIN_ODDLPARSER_NS

struct DDLNodeIterator {
    const DDLNode::DllNodeList &m_childs;
    size_t m_idx;

    DDLNodeIterator( const DDLNode::DllNodeList &childs ) 
    : m_childs( childs )
    , m_idx( 0 ) {
        // empty
    }

    ~DDLNodeIterator() {
        // empty
    }

    bool getNext( DDLNode **node ) {
        if( m_childs.size() > (m_idx+1) ) {
            m_idx++;
            *node = m_childs[ m_idx ];
            return true;
        }

        return false;
    }

private:
    DDLNodeIterator() ddl_no_copy;
    DDLNodeIterator &operator = ( const DDLNodeIterator & ) ddl_no_copy;
};

static void writeLineEnd( std::string &statement ) {
    statement += "\n";
}

OpenDDLExport::OpenDDLExport( IOStreamBase *stream )
: m_stream( stream ) {
    if (ddl_nullptr == m_stream) {
        m_stream = new IOStreamBase();
    }
}

OpenDDLExport::~OpenDDLExport() {
    if (ddl_nullptr != m_stream) {
        m_stream->close();
    }
    delete m_stream;
}

bool OpenDDLExport::exportContext( Context *ctx, const std::string &filename ) {
    if( ddl_nullptr == ctx ) {
        return false;
    }

    DDLNode *root( ctx->m_root );
    if ( ddl_nullptr == root ) {
        return true;
    }

    if (!filename.empty()) {
        if (!m_stream->open( filename )) {
            return false;
        }
    }

    const bool retValue( handleNode( root ) );
    
    return retValue;
}

bool OpenDDLExport::handleNode( DDLNode *node ) {
    if( ddl_nullptr == node ) {
        return true;
    }

    const DDLNode::DllNodeList &childs = node->getChildNodeList();
    if( childs.empty() ) {
        return true;
    }
    DDLNode *current( ddl_nullptr );
    DDLNodeIterator it( childs );
    std::string statement;
    bool success( true );
    while( it.getNext( &current ) ) {
        if( ddl_nullptr != current ) {
            success |= writeNode( current, statement );
            if( !handleNode( current ) ) {
                success = false;
            }
        }
    }

    return success;
}

bool OpenDDLExport::writeToStream( const std::string &statement ) {
    if (ddl_nullptr == m_stream ) {
        return false;
    }

    if ( !statement.empty()) {
        m_stream->write( statement );
    }

    return true;
}

bool OpenDDLExport::writeNode( DDLNode *node, std::string &statement ) {
    writeNodeHeader( node, statement );
    if (node->hasProperties()) {
        writeProperties( node, statement );
    }
    writeLineEnd( statement );

    statement = "}";
    DataArrayList *al( node->getDataArrayList() );
    if ( ddl_nullptr != al ) {
        writeValueType( al->m_dataList->m_type, al->m_numItems, statement );
        writeValueArray( al, statement );
    }
    Value *v( node->getValue() );
    if (ddl_nullptr != v ) {
        writeValueType( v->m_type, 1, statement );
        statement = "{";
        writeLineEnd( statement );
        writeValue( v, statement );
        statement = "}";
        writeLineEnd( statement );
    }
    statement = "}";
    writeLineEnd( statement );

    writeToStream( statement );

    return true;
}

bool OpenDDLExport::writeNodeHeader( DDLNode *node, std::string &statement ) {
    if (ddl_nullptr == node) {
        return false;
    }

    statement += node->getType();
    const std::string &name( node->getName() );
    if ( !name.empty() ) {
        statement += " ";
        statement += "$";
        statement += name;
    }

    return true;
}

bool OpenDDLExport::writeProperties( DDLNode *node, std::string &statement ) {
    if ( ddl_nullptr == node ) {
        return false;
    }

    Property *prop( node->getProperties() );
    // if no properties are there, return
    if ( ddl_nullptr == prop ) {
        return true;
    }

    if ( ddl_nullptr != prop ) {
        // for instance (attrib = "position", bla=2)
        statement += "(";
        bool first( true );
        while ( ddl_nullptr != prop ) {
            if (!first) {
                statement += ", ";
            } else {
                first = false;
            }
            statement += std::string( prop->m_key->m_buffer );
            statement += " = ";
            writeValue( prop->m_value, statement );
            prop = prop->m_next;
        }

        statement += ")";
    }

    return true;
}

bool OpenDDLExport::writeValueType( Value::ValueType type, size_t numItems, std::string &statement ) {
    if ( Value::ddl_types_max == type) {
        return false;
    }

    const std::string typeStr( getTypeToken( type ) );
    statement += typeStr;
    // if we have an array to write
    if ( numItems > 1 ) {
        statement += "[";
        char buffer[ 256 ];
        ::memset( buffer, '\0', 256 * sizeof( char ) );
        sprintf( buffer, "%d", static_cast<int>( numItems ) );
        statement += buffer;
        statement += "]";
    }

    return true;
}

bool OpenDDLExport::writeValue( Value *val, std::string &statement ) {
    if (ddl_nullptr == val) {
        return false;
    }

    switch ( val->m_type ) {
        case Value::ddl_bool:
            if ( true == val->getBool() ) {
                statement += "true";
            } else {
                statement += "false";
            }
            break;
        case Value::ddl_int8: 
            {
                std::stringstream stream;
                const int i = static_cast<int>( val->getInt8() );
                stream << i;
                statement += stream.str();
            }
            break;
        case Value::ddl_int16:
            {
                std::stringstream stream;
                char buffer[ 256 ];
                ::memset( buffer, '\0', 256 * sizeof( char ) );
                sprintf( buffer, "%d", val->getInt16() );
                statement += buffer;
        }
            break;
        case Value::ddl_int32:
            {
                std::stringstream stream;
                char buffer[ 256 ];
                ::memset( buffer, '\0', 256 * sizeof( char ) );
                const int i = static_cast< int >( val->getInt32() );
                sprintf( buffer, "%d", i );
                statement += buffer;
            }
            break;
        case Value::ddl_int64:
            {
                std::stringstream stream;
                const int i = static_cast< int >( val->getInt64() );
                stream << i;
                statement += stream.str();
            }
            break;
        case Value::ddl_unsigned_int8:
            {
                std::stringstream stream;
                const int i = static_cast< unsigned int >( val->getUnsignedInt8() );
                stream << i;
                statement += stream.str();
            }
            break;
        case Value::ddl_unsigned_int16:
            {
                std::stringstream stream;
                const int i = static_cast< unsigned int >( val->getUnsignedInt16() );
                stream << i;
                statement += stream.str();
            }
            break;
        case Value::ddl_unsigned_int32:
            {
                std::stringstream stream;
                const int i = static_cast< unsigned int >( val->getUnsignedInt32() );
                stream << i;
                statement += stream.str();
            }
            break;
        case Value::ddl_unsigned_int64:
            {
                std::stringstream stream;
                const int i = static_cast< unsigned int >( val->getUnsignedInt64() );
                stream << i;
                statement += stream.str();
            }
            break;
        case Value::ddl_half:
            break;
        case Value::ddl_float:
            {
                std::stringstream stream;
                stream << val->getFloat();
                statement += stream.str();
            }
            break;
        case Value::ddl_double:
            {
                std::stringstream stream;
                stream << val->getDouble();
                statement += stream.str();
            }
            break;
        case Value::ddl_string:
            {
                std::stringstream stream;
                stream << val->getString();
                statement += "\"";
                statement += stream.str();
                statement += "\"";
             }
            break;
        case Value::ddl_ref:
            break;
        case Value::ddl_none:
        case Value::ddl_types_max:
        default:
            break;
    }

    return true;
}

bool OpenDDLExport::writeValueArray( DataArrayList *al, std::string &statement ) {
    if (ddl_nullptr == al) {
        return false;
    }

    if (0 == al->m_numItems) {
        return true;
    }

    DataArrayList *nextDataArrayList = al ;
    while (ddl_nullptr != nextDataArrayList) {
        if (ddl_nullptr != nextDataArrayList) {
            statement += "{ ";
            Value *nextValue( nextDataArrayList->m_dataList );
            size_t idx( 0 );
            while (ddl_nullptr != nextValue) {
                if (idx > 0) {
                    statement += ", ";
                }
                writeValue( nextValue, statement );
                nextValue = nextValue->m_next;
                idx++;
            }
            statement += " }";
        }
        nextDataArrayList = nextDataArrayList->m_next;
    }

    return true;
}

END_ODDLPARSER_NS

