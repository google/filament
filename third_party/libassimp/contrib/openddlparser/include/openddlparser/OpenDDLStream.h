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

BEGIN_ODDLPARSER_NS

//-------------------------------------------------------------------------------------------------
/// @ingroup    IOStreamBase
///	@brief      This class represents the stream to write out.
//-------------------------------------------------------------------------------------------------
class DLL_ODDLPARSER_EXPORT StreamFormatterBase {
public:
    /// @brief  The class constructor.
    StreamFormatterBase();

    /// @brief  The class destructor, virtual.
    virtual ~StreamFormatterBase();

    /// @brief  Will format the sring and return the new formatted result.
    /// @param  statement   [in] The string to reformat.
    /// @return The reformatted result.
    virtual std::string format(const std::string &statement);
};

//-------------------------------------------------------------------------------------------------
/// @ingroup    IOStreamBase
///	@brief      This class represents the stream to write out.
//-------------------------------------------------------------------------------------------------
class DLL_ODDLPARSER_EXPORT IOStreamBase {
public:
    /// @brief  The class constructor with the formatter.
    /// @param  formatter   [in] The formatter to use.
    explicit IOStreamBase(StreamFormatterBase *formatter = ddl_nullptr);

    /// @brief  The class destructor, virtual.
    virtual ~IOStreamBase();

    /// @brief  Will open the stream.
    /// @param  name        [in] The name for the stream.
    /// @return true, if the stream was opened successfully, false if not.
    virtual bool open(const std::string &name);

    /// @brief  Will close the stream.
    /// @return true, if the stream was closed successfully, false if not.
    virtual bool close();

    /// @brief  Returns true, if the stream is open.
    /// @return true, if the stream is open, false if not.
    virtual bool isOpen() const;

    /// @brief  Will read a string from the stream.
    /// @param  sizeToRead  [in] The size to read in bytes.
    /// @param  statement   [out] The read statements.
    /// @return The bytes read from the stream.
    virtual size_t read( size_t sizeToRead, std::string &statement );

    /// @brief  Will write a string into the stream.
    /// @param  statement  [in] The string to write.
    /// @return The bytes written into the stream.
    virtual size_t write(const std::string &statement);

private:
    StreamFormatterBase *m_formatter;
    FILE *m_file;
};

END_ODDLPARSER_NS
