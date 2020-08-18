//
// Copyright (c) 2017-2020 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "Common.h"

bool StrRangeToPtrList(const StrRange& s, std::vector<uint64_t>& out)
{
    out.clear();
    StrRange currRange = { s.beg, nullptr };
    while(currRange.beg < s.end)
    {
        currRange.end = currRange.beg;
        while(currRange.end < s.end && *currRange.end != ' ')
        {
            ++currRange.end;
        }

        uint64_t ptr = 0;
        if(!StrRangeToPtr(currRange, ptr))
        {
            return false;
        }
        out.push_back(ptr);

        currRange.beg = currRange.end + 1;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// LineSplit class

bool LineSplit::GetNextLine(StrRange& out)
{
    if(m_NextLineBeg < m_NumBytes)
    {
        out.beg = m_Data + m_NextLineBeg;
        size_t currLineEnd = m_NextLineBeg;
        while(currLineEnd < m_NumBytes && m_Data[currLineEnd] != '\n')
            ++currLineEnd;
        out.end = m_Data + currLineEnd;
        // Ignore trailing '\r' to support Windows end of line.
        if(out.end > out.beg && *(out.end - 1) == '\r')
        {
            --out.end;
        }
        m_NextLineBeg = currLineEnd + 1; // Past '\n'
        ++m_NextLineIndex;
        return true;
    }
    else
        return false;
}

////////////////////////////////////////////////////////////////////////////////
// CsvSplit class

void CsvSplit::Set(const StrRange& line, size_t maxCount)
{
    assert(maxCount <= RANGE_COUNT_MAX);
    m_Line = line;
    const size_t strLen = line.length();
    size_t rangeIndex = 0;
    size_t charIndex = 0;
    while(charIndex < strLen && rangeIndex < maxCount)
    {
        m_Ranges[rangeIndex * 2] = charIndex;
        while(charIndex < strLen && (rangeIndex + 1 == maxCount || m_Line.beg[charIndex] != ','))
            ++charIndex;
        m_Ranges[rangeIndex * 2 + 1] = charIndex;
        ++rangeIndex;
        ++charIndex; // Past ','
    }
    m_Count = rangeIndex;
}

////////////////////////////////////////////////////////////////////////////////
// class CmdLineParser

bool CmdLineParser::ReadNextArg(std::string *OutArg)
{
	if (m_argv != NULL)
	{
		if (m_ArgIndex >= (size_t)m_argc) return false;

		*OutArg = m_argv[m_ArgIndex];
		m_ArgIndex++;
		return true;
	}
	else
	{
		if (m_ArgIndex >= m_CmdLineLength) return false;
		
		OutArg->clear();
		bool InsideQuotes = false;
		while (m_ArgIndex < m_CmdLineLength)
		{
			char Ch = m_CmdLine[m_ArgIndex];
			if (Ch == '\\')
			{
				bool FollowedByQuote = false;
				size_t BackslashCount = 1;
				size_t TmpIndex = m_ArgIndex + 1;
				while (TmpIndex < m_CmdLineLength)
				{
					char TmpCh = m_CmdLine[TmpIndex];
					if (TmpCh == '\\')
					{
						BackslashCount++;
						TmpIndex++;
					}
					else if (TmpCh == '"')
					{
						FollowedByQuote = true;
						break;
					}
					else
						break;
				}

				if (FollowedByQuote)
				{
					if (BackslashCount % 2 == 0)
					{
						for (size_t i = 0; i < BackslashCount / 2; i++)
							*OutArg += '\\';
						m_ArgIndex += BackslashCount + 1;
						InsideQuotes = !InsideQuotes;
					}
					else
					{
						for (size_t i = 0; i < BackslashCount / 2; i++)
							*OutArg += '\\';
						*OutArg += '"';
						m_ArgIndex += BackslashCount + 1;
					}
				}
				else
				{
					for (size_t i = 0; i < BackslashCount; i++)
						*OutArg += '\\';
					m_ArgIndex += BackslashCount;
				}
			}
			else if (Ch == '"')
			{
				InsideQuotes = !InsideQuotes;
				m_ArgIndex++;
			}
			else if (isspace(Ch))
			{
				if (InsideQuotes)
				{
					*OutArg += Ch;
					m_ArgIndex++;
				}
				else
				{
					m_ArgIndex++;
					break;
				}
			}
			else
			{
				*OutArg += Ch;
				m_ArgIndex++;
			}
		}

		while (m_ArgIndex < m_CmdLineLength && isspace(m_CmdLine[m_ArgIndex]))
			m_ArgIndex++;

		return true;
	}
}

CmdLineParser::SHORT_OPT * CmdLineParser::FindShortOpt(char Opt)
{
	for (size_t i = 0; i < m_ShortOpts.size(); i++)
		if (m_ShortOpts[i].Opt == Opt)
			return &m_ShortOpts[i];
	return NULL;
}

CmdLineParser::LONG_OPT * CmdLineParser::FindLongOpt(const std::string &Opt)
{
	for (size_t i = 0; i < m_LongOpts.size(); i++)
		if (m_LongOpts[i].Opt == Opt)
			return &m_LongOpts[i];
	return NULL;
}

CmdLineParser::CmdLineParser(int argc, char **argv) :
	m_argv(argv),
	m_CmdLine(NULL),
	m_argc(argc),
	m_CmdLineLength(0),
	m_ArgIndex(1),
	m_InsideMultioption(false),
	m_LastArgIndex(0),
	m_LastOptId(0)
{
	assert(argc > 0);
	assert(argv != NULL);
}

CmdLineParser::CmdLineParser(const char *CmdLine) :
	m_argv(NULL),
	m_CmdLine(CmdLine),
	m_argc(0),
	m_ArgIndex(0),
	m_InsideMultioption(false),
	m_LastArgIndex(0),
	m_LastOptId(0)
{
	assert(CmdLine != NULL);

	m_CmdLineLength = strlen(m_CmdLine);

	while (m_ArgIndex < m_CmdLineLength && isspace(m_CmdLine[m_ArgIndex]))
		m_ArgIndex++;
}

void CmdLineParser::RegisterOpt(uint32_t Id, char Opt, bool Parameter)
{
	assert(Opt != '\0');

	m_ShortOpts.push_back(SHORT_OPT(Id, Opt, Parameter));
}

void CmdLineParser::RegisterOpt(uint32_t Id, const std::string &Opt, bool Parameter)
{
	assert(!Opt.empty());
	
	m_LongOpts.push_back(LONG_OPT(Id, Opt, Parameter));
}

CmdLineParser::RESULT CmdLineParser::ReadNext()
{
	if (m_InsideMultioption)
	{
		assert(m_LastArgIndex < m_LastArg.length());
		SHORT_OPT *so = FindShortOpt(m_LastArg[m_LastArgIndex]);
		if (so == NULL)
		{
			m_LastOptId = 0;
			m_LastParameter.clear();
			return CmdLineParser::RESULT_ERROR;
		}
		if (so->Parameter)
		{
			if (m_LastArg.length() == m_LastArgIndex+1)
			{
				if (!ReadNextArg(&m_LastParameter))
				{
					m_LastOptId = 0;
					m_LastParameter.clear();
					return CmdLineParser::RESULT_ERROR;
				}
				m_InsideMultioption = false;
				m_LastOptId = so->Id;
				return CmdLineParser::RESULT_OPT;
			}
			else if (m_LastArg[m_LastArgIndex+1] == '=')
			{
				m_InsideMultioption = false;
				m_LastParameter = m_LastArg.substr(m_LastArgIndex+2);
				m_LastOptId = so->Id;
				return CmdLineParser::RESULT_OPT;
			}
			else
			{
				m_InsideMultioption = false;
				m_LastParameter = m_LastArg.substr(m_LastArgIndex+1);
				m_LastOptId = so->Id;
				return CmdLineParser::RESULT_OPT;
			}
		}
		else
		{
			if (m_LastArg.length() == m_LastArgIndex+1)
			{
				m_InsideMultioption = false;
				m_LastParameter.clear();
				m_LastOptId = so->Id;
				return CmdLineParser::RESULT_OPT;
			}
			else
			{
				m_LastArgIndex++;

				m_LastParameter.clear();
				m_LastOptId = so->Id;
				return CmdLineParser::RESULT_OPT;
			}
		}
	}
	else
	{
		if (!ReadNextArg(&m_LastArg))
		{
			m_LastParameter.clear();
			m_LastOptId = 0;
			return CmdLineParser::RESULT_END;
		}
		
		if (!m_LastArg.empty() && m_LastArg[0] == '-')
		{
			if (m_LastArg.length() > 1 && m_LastArg[1] == '-')
			{
				size_t EqualIndex = m_LastArg.find('=', 2);
				if (EqualIndex != std::string::npos)
				{
					LONG_OPT *lo = FindLongOpt(m_LastArg.substr(2, EqualIndex-2));
					if (lo == NULL || lo->Parameter == false)
					{
						m_LastOptId = 0;
						m_LastParameter.clear();
						return CmdLineParser::RESULT_ERROR;
					}
					m_LastParameter = m_LastArg.substr(EqualIndex+1);
					m_LastOptId = lo->Id;
					return CmdLineParser::RESULT_OPT;
				}
				else
				{
					LONG_OPT *lo = FindLongOpt(m_LastArg.substr(2));
					if (lo == NULL)
					{
						m_LastOptId = 0;
						m_LastParameter.clear();
						return CmdLineParser::RESULT_ERROR;
					}
					if (lo->Parameter)
					{
						if (!ReadNextArg(&m_LastParameter))
						{
							m_LastOptId = 0;
							m_LastParameter.clear();
							return CmdLineParser::RESULT_ERROR;
						}
					}
					else
						m_LastParameter.clear();
					m_LastOptId = lo->Id;
					return CmdLineParser::RESULT_OPT;
				}
			}
			else
			{
				if (m_LastArg.length() < 2)
				{
					m_LastOptId = 0;
					m_LastParameter.clear();
					return CmdLineParser::RESULT_ERROR;
				}
				SHORT_OPT *so = FindShortOpt(m_LastArg[1]);
				if (so == NULL)
				{
					m_LastOptId = 0;
					m_LastParameter.clear();
					return CmdLineParser::RESULT_ERROR;
				}
				if (so->Parameter)
				{
					if (m_LastArg.length() == 2)
					{
						if (!ReadNextArg(&m_LastParameter))
						{
							m_LastOptId = 0;
							m_LastParameter.clear();
							return CmdLineParser::RESULT_ERROR;
						}
						m_LastOptId = so->Id;
						return CmdLineParser::RESULT_OPT;
					}
					else if (m_LastArg[2] == '=')
					{
						m_LastParameter = m_LastArg.substr(3);
						m_LastOptId = so->Id;
						return CmdLineParser::RESULT_OPT;
					}
					else
					{
						m_LastParameter = m_LastArg.substr(2);
						m_LastOptId = so->Id;
						return CmdLineParser::RESULT_OPT;
					}
				}
				else
				{
					if (m_LastArg.length() == 2)
					{
						m_LastParameter.clear();
						m_LastOptId = so->Id;
						return CmdLineParser::RESULT_OPT;
					}
					else
					{
						m_InsideMultioption = true;
						m_LastArgIndex = 2;

						m_LastParameter.clear();
						m_LastOptId = so->Id;
						return CmdLineParser::RESULT_OPT;
					}
				}
			}
		}
		else if (!m_LastArg.empty() && m_LastArg[0] == '/')
		{
			size_t EqualIndex = m_LastArg.find('=', 1);
			if (EqualIndex != std::string::npos)
			{
				if (EqualIndex == 2)
				{
					SHORT_OPT *so = FindShortOpt(m_LastArg[1]);
					if (so != NULL)
					{
						if (so->Parameter == false)	
						{
							m_LastOptId = 0;
							m_LastParameter.clear();
							return CmdLineParser::RESULT_ERROR;
						}
						m_LastParameter = m_LastArg.substr(EqualIndex+1);
						m_LastOptId = so->Id;
						return CmdLineParser::RESULT_OPT;
					}
				}
				LONG_OPT *lo = FindLongOpt(m_LastArg.substr(1, EqualIndex-1));
				if (lo == NULL || lo->Parameter == false)
				{
					m_LastOptId = 0;
					m_LastParameter.clear();
					return CmdLineParser::RESULT_ERROR;
				}
				m_LastParameter = m_LastArg.substr(EqualIndex+1);
				m_LastOptId = lo->Id;
				return CmdLineParser::RESULT_OPT;
			}
			else
			{
				if (m_LastArg.length() == 2)
				{
					SHORT_OPT *so = FindShortOpt(m_LastArg[1]);
					if (so != NULL)
					{
						if (so->Parameter)
						{
							if (!ReadNextArg(&m_LastParameter))
							{
								m_LastOptId = 0;
								m_LastParameter.clear();
								return CmdLineParser::RESULT_ERROR;
							}
						}
						else
							m_LastParameter.clear();
						m_LastOptId = so->Id;
						return CmdLineParser::RESULT_OPT;
					}
				}
				LONG_OPT *lo = FindLongOpt(m_LastArg.substr(1));
				if (lo == NULL)
				{
					m_LastOptId = 0;
					m_LastParameter.clear();
					return CmdLineParser::RESULT_ERROR;
				}
				if (lo->Parameter)
				{
					if (!ReadNextArg(&m_LastParameter))
					{
						m_LastOptId = 0;
						m_LastParameter.clear();
						return CmdLineParser::RESULT_ERROR;
					}
				}
				else
					m_LastParameter.clear();
				m_LastOptId = lo->Id;
				return CmdLineParser::RESULT_OPT;
			}
		}
		else
		{
			m_LastOptId = 0;
			m_LastParameter = m_LastArg;
			return CmdLineParser::RESULT_PARAMETER;
		}
	}
}

uint32_t CmdLineParser::GetOptId()
{
	return m_LastOptId;
}

const std::string & CmdLineParser::GetParameter()
{
	return m_LastParameter;
}

////////////////////////////////////////////////////////////////////////////////
// Glolals

/*

void SetConsoleColor(CONSOLE_COLOR color)
{
    WORD attr = 0;
    switch(color)
    {
    case CONSOLE_COLOR::INFO:
        attr = FOREGROUND_INTENSITY;;
        break;
    case CONSOLE_COLOR::NORMAL:
        attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        break;
    case CONSOLE_COLOR::WARNING:
        attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        break;
    case CONSOLE_COLOR::ERROR_:
        attr = FOREGROUND_RED | FOREGROUND_INTENSITY;
        break;
    default:
        assert(0);
    }

    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(out, attr);
}

void PrintMessage(CONSOLE_COLOR color, const char* msg)
{
    if(color != CONSOLE_COLOR::NORMAL)
        SetConsoleColor(color);
    
    printf("%s\n", msg);
    
    if (color != CONSOLE_COLOR::NORMAL)
        SetConsoleColor(CONSOLE_COLOR::NORMAL);
}

void PrintMessage(CONSOLE_COLOR color, const wchar_t* msg)
{
    if(color != CONSOLE_COLOR::NORMAL)
        SetConsoleColor(color);
    
    wprintf(L"%s\n", msg);
    
    if (color != CONSOLE_COLOR::NORMAL)
        SetConsoleColor(CONSOLE_COLOR::NORMAL);
}

static const size_t CONSOLE_SMALL_BUF_SIZE = 256;

void PrintMessageV(CONSOLE_COLOR color, const char* format, va_list argList)
{
	size_t dstLen = (size_t)::_vscprintf(format, argList);
	if(dstLen)
	{
		bool useSmallBuf = dstLen < CONSOLE_SMALL_BUF_SIZE;
		char smallBuf[CONSOLE_SMALL_BUF_SIZE];
		std::vector<char> bigBuf(useSmallBuf ? 0 : dstLen + 1);
		char* bufPtr = useSmallBuf ? smallBuf : bigBuf.data();
		::vsprintf_s(bufPtr, dstLen + 1, format, argList);
		PrintMessage(color, bufPtr);
	}
}

void PrintMessageV(CONSOLE_COLOR color, const wchar_t* format, va_list argList)
{
	size_t dstLen = (size_t)::_vcwprintf(format, argList);
	if(dstLen)
	{
		bool useSmallBuf = dstLen < CONSOLE_SMALL_BUF_SIZE;
		wchar_t smallBuf[CONSOLE_SMALL_BUF_SIZE];
		std::vector<wchar_t> bigBuf(useSmallBuf ? 0 : dstLen + 1);
		wchar_t* bufPtr = useSmallBuf ? smallBuf : bigBuf.data();
		::vswprintf_s(bufPtr, dstLen + 1, format, argList);
		PrintMessage(color, bufPtr);
	}
}

void PrintMessageF(CONSOLE_COLOR color, const char* format, ...)
{
	va_list argList;
	va_start(argList, format);
	PrintMessageV(color, format, argList);
	va_end(argList);
}

void PrintMessageF(CONSOLE_COLOR color, const wchar_t* format, ...)
{
	va_list argList;
	va_start(argList, format);
	PrintMessageV(color, format, argList);
	va_end(argList);
}

void PrintWarningF(const char* format, ...)
{
	va_list argList;
	va_start(argList, format);
	PrintMessageV(CONSOLE_COLOR::WARNING, format, argList);
	va_end(argList);
}

void PrintWarningF(const wchar_t* format, ...)
{
	va_list argList;
	va_start(argList, format);
	PrintMessageV(CONSOLE_COLOR::WARNING, format, argList);
	va_end(argList);
}

void PrintErrorF(const char* format, ...)
{
	va_list argList;
	va_start(argList, format);
	PrintMessageV(CONSOLE_COLOR::WARNING, format, argList);
	va_end(argList);
}

void PrintErrorF(const wchar_t* format, ...)
{
	va_list argList;
	va_start(argList, format);
	PrintMessageV(CONSOLE_COLOR::WARNING, format, argList);
	va_end(argList);
}
*/

void SecondsToFriendlyStr(float seconds, std::string& out)
{
    if(seconds == 0.f)
    {
        out = "0";
        return;
    }

    if (seconds < 0.f)
	{
		out = "-";
        seconds = -seconds;
	}
	else
	{
		out.clear();
	}

	char s[32];

    // #.### ns
    if(seconds < 1e-6)
    {
        sprintf_s(s, "%.3f ns", seconds * 1e9);
        out += s;
    }
    // #.### us
    else if(seconds < 1e-3)
    {
        sprintf_s(s, "%.3f us", seconds * 1e6);
        out += s;
    }
    // #.### ms
    else if(seconds < 1.f)
    {
        sprintf_s(s, "%.3f ms", seconds * 1e3);
        out += s;
    }
    // #.### s
    else if(seconds < 60.f)
    {
        sprintf_s(s, "%.3f s", seconds);
        out += s;
    }
    else
    {
	    uint64_t seconds_u = (uint64_t)seconds;
	    // "#:## min"
	    if (seconds_u < 3600)
	    {
		    uint64_t minutes = seconds_u / 60;
		    seconds_u -= minutes * 60;
            sprintf_s(s, "%llu:%02llu min", minutes, seconds_u);
            out += s;
	    }
	    // "#:##:## h"
	    else
	    {
		    uint64_t minutes = seconds_u / 60;
            seconds_u -= minutes * 60;
		    uint64_t hours = minutes / 60;
		    minutes -= hours * 60;
            sprintf_s(s, "%llu:%02llu:%02llu h", hours, minutes, seconds_u);
            out += s;
	    }
    }
}
