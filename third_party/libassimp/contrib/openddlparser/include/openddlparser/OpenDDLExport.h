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
#include <openddlparser/OpenDDLStream.h>
#include <openddlparser/Value.h>

BEGIN_ODDLPARSER_NS

// Forward declarations
class IOStreamBase;

//-------------------------------------------------------------------------------------------------
///
/// @ingroup    OpenDDLParser
///	@brief      This class represents the OpenDDLExporter.
///
//-------------------------------------------------------------------------------------------------
class DLL_ODDLPARSER_EXPORT OpenDDLExport {
public:
    ///	@brief  The class constructor
    OpenDDLExport( IOStreamBase *stream = ddl_nullptr );

    ///	@brief  The class destructor.
    ~OpenDDLExport();

    ///	@brief  Export the data of a parser context.
    /// @param  ctx         [in] Pointer to the context.
    /// @param  filename    [in] The filename for the export.
    /// @return True in case of success, false in case of an error.
    bool exportContext( Context *ctx, const std::string &filename );

    ///	@brief  Handles a node export.
    /// @param  node        [in] The node to handle with.
    /// @return True in case of success, false in case of an error.
    bool handleNode( DDLNode *node );

    ///	@brief  Writes the statement to the stream.
    /// @param  statement   [in]  The content to write.
    /// @return True in case of success, false in case of an error.
    bool writeToStream( const std::string &statement );

protected:
    bool writeNode( DDLNode *node, std::string &statement );
    bool writeNodeHeader( DDLNode *node, std::string &statement );
    bool writeProperties( DDLNode *node, std::string &statement );
    bool writeValueType( Value::ValueType type, size_t numItems, std::string &statement );
    bool writeValue( Value *val, std::string &statement );
    bool writeValueArray( DataArrayList *al, std::string &statement );

private:
    OpenDDLExport( const OpenDDLExport & ) ddl_no_copy;
    OpenDDLExport &operator = ( const OpenDDLExport  & ) ddl_no_copy;

private:
    IOStreamBase *m_stream;
};

END_ODDLPARSER_NS
