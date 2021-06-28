//
// Copyright (c) 2017-2021 Advanced Micro Devices, Inc. All rights reserved.
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

#pragma once

#include "VmaUsage.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>
#include <array>
#include <type_traits>
#include <utility>
#include <chrono>
#include <string>
#include <limits>

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>

typedef std::chrono::high_resolution_clock::time_point time_point;
typedef std::chrono::high_resolution_clock::duration duration;

inline float ToFloatSeconds(duration d)
{
    return std::chrono::duration_cast<std::chrono::duration<float>>(d).count();
}

void SecondsToFriendlyStr(float seconds, std::string& out);

template <typename T>
T ceil_div(T x, T y)
{
    return (x+y-1) / y;
}
template <typename T>
inline T round_div(T x, T y)
{
    return (x+y/(T)2) / y;
}

template <typename T>
inline T align_up(T val, T align)
{
    return (val + align - 1) / align * align;
}

struct StrRange
{
    const char* beg;
    const char* end;

    StrRange() { }
    StrRange(const char* beg, const char* end) : beg(beg), end(end) { }
    explicit StrRange(const char* sz) : beg(sz), end(sz + strlen(sz)) { }
    explicit StrRange(const std::string& s) : beg(s.data()), end(s.data() + s.length()) { }

    size_t length() const { return end - beg; }
    void to_str(std::string& out) const { out.assign(beg, end); }
};

inline bool StrRangeEq(const StrRange& lhs, const char* rhsSz)
{
    const size_t rhsLen = strlen(rhsSz);
    return rhsLen == lhs.length() &&
        memcmp(lhs.beg, rhsSz, rhsLen) == 0;
}

inline bool StrRangeToUint(const StrRange& s, uint32_t& out)
{
    char* end = (char*)s.end;
    out = (uint32_t)strtoul(s.beg, &end, 10);
    return end == s.end;
}
inline bool StrRangeToUint(const StrRange& s, uint64_t& out)
{
    char* end = (char*)s.end;
    out = (uint64_t)strtoull(s.beg, &end, 10);
    return end == s.end;
}
inline bool StrRangeToPtr(const StrRange& s, uint64_t& out)
{
    char* end = (char*)s.end;
    out = (uint64_t)strtoull(s.beg, &end, 16);
    return end == s.end;
}
inline bool StrRangeToFloat(const StrRange& s, float& out)
{
    char* end = (char*)s.end;
    out = strtof(s.beg, &end);
    return end == s.end;
}
inline bool StrRangeToBool(const StrRange& s, bool& out)
{
    if(s.end - s.beg == 1)
    {
        if(*s.beg == '1')
        {
            out = true;
        }
        else if(*s.beg == '0')
        {
            out = false;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}
bool StrRangeToPtrList(const StrRange& s, std::vector<uint64_t>& out);

class LineSplit
{
public:
    LineSplit(const char* data, size_t numBytes) :
        m_Data(data),
        m_NumBytes(numBytes),
        m_NextLineBeg(0),
        m_NextLineIndex(0)
    {
    }

    bool GetNextLine(StrRange& out);
    size_t GetNextLineIndex() const { return m_NextLineIndex; }

private:
    const char* const m_Data;
    const size_t m_NumBytes;
    size_t m_NextLineBeg;
    size_t m_NextLineIndex;
};

class CsvSplit
{
public:
    static const size_t RANGE_COUNT_MAX = 32;

    void Set(const StrRange& line, size_t maxCount = RANGE_COUNT_MAX);

    const StrRange& GetLine() const { return m_Line; }

    size_t GetCount() const { return m_Count; }
    StrRange GetRange(size_t index) const 
    {
        if(index < m_Count)
        {
            return StrRange {
                m_Line.beg + m_Ranges[index * 2],
                m_Line.beg + m_Ranges[index * 2 + 1] };
        }
        else
        {
            return StrRange{0, 0};
        }
    }

private:
    StrRange m_Line = { nullptr, nullptr };
    size_t m_Count = 0;
    size_t m_Ranges[RANGE_COUNT_MAX * 2]; // Pairs of begin-end.
};

class CmdLineParser
{
public:
	enum RESULT
	{
		RESULT_OPT,
		RESULT_PARAMETER,
		RESULT_END,
		RESULT_ERROR,
	};

	CmdLineParser(int argc, char **argv);
	CmdLineParser(const char *CmdLine);
	
    void RegisterOpt(uint32_t Id, char Opt, bool Parameter);
	void RegisterOpt(uint32_t Id, const std::string &Opt, bool Parameter);
	
    RESULT ReadNext();
	uint32_t GetOptId();
	const std::string & GetParameter();

private:
	struct SHORT_OPT
	{
		uint32_t Id;
		char Opt;
		bool Parameter;

		SHORT_OPT(uint32_t Id, char Opt, bool Parameter) : Id(Id), Opt(Opt), Parameter(Parameter) { }
	};

	struct LONG_OPT
	{
		uint32_t Id;
		std::string Opt;
		bool Parameter;

		LONG_OPT(uint32_t Id, std::string Opt, bool Parameter) : Id(Id), Opt(Opt), Parameter(Parameter) { }
	};

	char **m_argv;
	const char *m_CmdLine;
	int m_argc;
	size_t m_CmdLineLength;
	size_t m_ArgIndex;

	bool ReadNextArg(std::string *OutArg);

	std::vector<SHORT_OPT> m_ShortOpts;
	std::vector<LONG_OPT> m_LongOpts;

	SHORT_OPT * FindShortOpt(char Opt);
	LONG_OPT * FindLongOpt(const std::string &Opt);

	bool m_InsideMultioption;
	std::string m_LastArg;
	size_t m_LastArgIndex;
	uint32_t m_LastOptId;
	std::string m_LastParameter;
};

/*
Parses and stores a sequence of ranges.

Upper range is inclusive.

Examples:

    "1" -> [ {1, 1} ]
    "1,10" -> [ {1, 1}, {10, 10} ]
    "2-6" -> [ {2, 6} ]
    "-8" -> [ {MIN, 8} ]
    "12-" -> [ {12, MAX} ]
    "1-10,12,15-" -> [ {1, 10}, {12, 12}, {15, MAX} ]

TODO: Optimize it: Do sorting and merging while parsing. Do binary search while
reading.
*/
template<typename T>
class RangeSequence
{
public:
    typedef std::pair<T, T> RangeType;

    void Clear() { m_Ranges.clear(); }
    bool Parse(const StrRange& str);

    bool IsEmpty() const { return m_Ranges.empty(); }
    size_t GetCount() const { return m_Ranges.size(); }
    const RangeType* GetRanges() const { return m_Ranges.data(); }

    bool Includes(T number) const;
    
private:
    std::vector<RangeType> m_Ranges;
};

template<typename T>
bool RangeSequence<T>::Parse(const StrRange& str)
{
    m_Ranges.clear();

    StrRange currRange = { str.beg, str.beg };
    while(currRange.beg < str.end)
    {
        currRange.end = currRange.beg + 1;
        // Find next ',' or the end.
        while(currRange.end < str.end && *currRange.end != ',')
        {
            ++currRange.end;
        }

        // Find '-' within this range.
        const char* hyphenPos = currRange.beg;
        while(hyphenPos < currRange.end && *hyphenPos != '-')
        {
            ++hyphenPos;
        }

        // No hyphen - single number like '10'.
        if(hyphenPos == currRange.end)
        {
            RangeType range;
            if(!StrRangeToUint(currRange, range.first))
            {
                return false;
            }
            range.second = range.first;
            m_Ranges.push_back(range);
        }
        // Hyphen at the end, like '10-'.
        else if(hyphenPos + 1 == currRange.end)
        {
            const StrRange numberRange = { currRange.beg, hyphenPos };
            RangeType range;
            if(!StrRangeToUint(numberRange, range.first))
            {
                return false;
            }
            range.second = std::numeric_limits<T>::max();
            m_Ranges.push_back(range);
        }
        // Hyphen at the beginning, like "-10".
        else if(hyphenPos == currRange.beg)
        {
            const StrRange numberRange = { currRange.beg + 1, currRange.end };
            RangeType range;
            range.first = std::numeric_limits<T>::min();
            if(!StrRangeToUint(numberRange, range.second))
            {
                return false;
            }
            m_Ranges.push_back(range);
        }
        // Hyphen in the middle, like "1-10".
        else
        {
            const StrRange numberRange1 = { currRange.beg, hyphenPos };
            const StrRange numberRange2 = { hyphenPos + 1, currRange.end };
            RangeType range;
            if(!StrRangeToUint(numberRange1, range.first) ||
                !StrRangeToUint(numberRange2, range.second) ||
                range.second < range.first)
            {
                return false;
            }
            m_Ranges.push_back(range);
        }

        // Skip ','
        currRange.beg = currRange.end + 1;
    }

    return true;
}

template<typename T>
bool RangeSequence<T>::Includes(T number) const
{
    for(const auto& it : m_Ranges)
    {
        if(number >= it.first && number <= it.second)
        {
            return true;
        }
    }
    return false;
}

/*
class RandomNumberGenerator
{
public:
    RandomNumberGenerator() : m_Value{GetTickCount()} {}
    RandomNumberGenerator(uint32_t seed) : m_Value{seed} { }
    void Seed(uint32_t seed) { m_Value = seed; }
    uint32_t Generate() { return GenerateFast() ^ (GenerateFast() >> 7); }

private:
    uint32_t m_Value;
    uint32_t GenerateFast() { return m_Value = (m_Value * 196314165 + 907633515); }
};

enum class CONSOLE_COLOR
{
    INFO,
    NORMAL,
    WARNING,
    ERROR_,
    COUNT
};

void SetConsoleColor(CONSOLE_COLOR color);

void PrintMessage(CONSOLE_COLOR color, const char* msg);
void PrintMessage(CONSOLE_COLOR color, const wchar_t* msg);

inline void Print(const char* msg) { PrintMessage(CONSOLE_COLOR::NORMAL, msg); }
inline void Print(const wchar_t* msg) { PrintMessage(CONSOLE_COLOR::NORMAL, msg); }
inline void PrintWarning(const char* msg) { PrintMessage(CONSOLE_COLOR::WARNING, msg); }
inline void PrintWarning(const wchar_t* msg) { PrintMessage(CONSOLE_COLOR::WARNING, msg); }
inline void PrintError(const char* msg) { PrintMessage(CONSOLE_COLOR::ERROR_, msg); }
inline void PrintError(const wchar_t* msg) { PrintMessage(CONSOLE_COLOR::ERROR_, msg); }

void PrintMessageV(CONSOLE_COLOR color, const char* format, va_list argList);
void PrintMessageV(CONSOLE_COLOR color, const wchar_t* format, va_list argList);
void PrintMessageF(CONSOLE_COLOR color, const char* format, ...);
void PrintMessageF(CONSOLE_COLOR color, const wchar_t* format, ...);
void PrintWarningF(const char* format, ...);
void PrintWarningF(const wchar_t* format, ...);
void PrintErrorF(const char* format, ...);
void PrintErrorF(const wchar_t* format, ...);
*/
