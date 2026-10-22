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
#include <openddlparser/OpenDDLStream.h>

BEGIN_ODDLPARSER_NS

StreamFormatterBase::StreamFormatterBase() {
    // empty
}

StreamFormatterBase::~StreamFormatterBase() {
    // empty
}

std::string StreamFormatterBase::format(const std::string &statement) {
    std::string tmp(statement);
    return tmp;
}

IOStreamBase::IOStreamBase(StreamFormatterBase *formatter)
    : m_formatter(formatter)
    , m_file(ddl_nullptr) {
    if (ddl_nullptr == m_formatter) {
        m_formatter = new StreamFormatterBase;
    }
}

IOStreamBase::~IOStreamBase() {
    delete m_formatter;
    m_formatter = ddl_nullptr;
}

bool IOStreamBase::open(const std::string &name) {
    m_file = ::fopen(name.c_str(), "a");
    if (m_file == ddl_nullptr) {
        return false;
    }

    return true;
}

bool IOStreamBase::close() {
    if (ddl_nullptr == m_file) {
        return false;
    }

    ::fclose(m_file);
    m_file = ddl_nullptr;

    return true;
}

bool IOStreamBase::isOpen() const {
    return ( ddl_nullptr != m_file );
}

size_t IOStreamBase::read( size_t sizeToRead, std::string &statement ) {
    if (ddl_nullptr == m_file) {
        return 0;
    }
    
    statement.resize(sizeToRead);
    const size_t readBytes = ::fread( &statement[0], 1, sizeToRead, m_file );

    return readBytes;
}

size_t IOStreamBase::write(const std::string &statement) {
    if (ddl_nullptr == m_file) {
        return 0;
    }
    std::string formatStatement = m_formatter->format(statement);
    return ::fwrite(formatStatement.c_str(), sizeof(char), formatStatement.size(), m_file);
}

END_ODDLPARSER_NS
