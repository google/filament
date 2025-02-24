//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// string_utils:
//   String helper functions.
//

#include "common/string_utils.h"

#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

#include "common/platform.h"
#include "common/system_utils.h"

namespace
{

bool EndsWithSuffix(const char *str,
                    const size_t strLen,
                    const char *suffix,
                    const size_t suffixLen)
{
    return suffixLen <= strLen && strncmp(str + strLen - suffixLen, suffix, suffixLen) == 0;
}

}  // anonymous namespace

namespace angle
{

const char kWhitespaceASCII[] = " \f\n\r\t\v";

std::vector<std::string> SplitString(const std::string &input,
                                     const std::string &delimiters,
                                     WhitespaceHandling whitespace,
                                     SplitResult resultType)
{
    std::vector<std::string> result;
    if (input.empty())
    {
        return result;
    }

    std::string::size_type start = 0;
    while (start != std::string::npos)
    {
        auto end = input.find_first_of(delimiters, start);

        std::string piece;
        if (end == std::string::npos)
        {
            piece = input.substr(start);
            start = std::string::npos;
        }
        else
        {
            piece = input.substr(start, end - start);
            start = end + 1;
        }

        if (whitespace == TRIM_WHITESPACE)
        {
            piece = TrimString(piece, kWhitespaceASCII);
        }

        if (resultType == SPLIT_WANT_ALL || !piece.empty())
        {
            result.push_back(std::move(piece));
        }
    }

    return result;
}

void SplitStringAlongWhitespace(const std::string &input, std::vector<std::string> *tokensOut)
{

    std::istringstream stream(input);
    std::string line;

    while (std::getline(stream, line))
    {
        size_t prev = 0, pos;
        while ((pos = line.find_first_of(kWhitespaceASCII, prev)) != std::string::npos)
        {
            if (pos > prev)
                tokensOut->push_back(line.substr(prev, pos - prev));
            prev = pos + 1;
        }
        if (prev < line.length())
            tokensOut->push_back(line.substr(prev, std::string::npos));
    }
}

std::string TrimString(const std::string &input, const std::string &trimChars)
{
    auto begin = input.find_first_not_of(trimChars);
    if (begin == std::string::npos)
    {
        return "";
    }

    std::string::size_type end = input.find_last_not_of(trimChars);
    if (end == std::string::npos)
    {
        return input.substr(begin);
    }

    return input.substr(begin, end - begin + 1);
}

std::string GetPrefix(const std::string &input, size_t offset, const char *delimiter)
{
    size_t match = input.find(delimiter, offset);
    if (match == std::string::npos)
    {
        return input.substr(offset);
    }
    return input.substr(offset, match - offset);
}

std::string GetPrefix(const std::string &input, size_t offset, char delimiter)
{
    size_t match = input.find(delimiter, offset);
    if (match == std::string::npos)
    {
        return input.substr(offset);
    }
    return input.substr(offset, match - offset);
}

bool HexStringToUInt(const std::string &input, unsigned int *uintOut)
{
    unsigned int offset = 0;

    if (input.size() >= 2 && input[0] == '0' && input[1] == 'x')
    {
        offset = 2u;
    }

    // Simple validity check
    if (input.find_first_not_of("0123456789ABCDEFabcdef", offset) != std::string::npos)
    {
        return false;
    }

    std::stringstream inStream(input);
    inStream >> std::hex >> *uintOut;
    return !inStream.fail();
}

bool ReadFileToString(const std::string &path, std::string *stringOut)
{
    std::ifstream inFile(path.c_str(), std::ios::binary);
    if (inFile.fail())
    {
        return false;
    }

    inFile.seekg(0, std::ios::end);
    auto size = static_cast<std::string::size_type>(inFile.tellg());
    stringOut->resize(size);
    inFile.seekg(0, std::ios::beg);

    inFile.read(stringOut->data(), size);
    return !inFile.fail();
}

bool BeginsWith(const std::string &str, const std::string &prefix)
{
    return strncmp(str.c_str(), prefix.c_str(), prefix.length()) == 0;
}

bool BeginsWith(const std::string &str, const char *prefix)
{
    return strncmp(str.c_str(), prefix, strlen(prefix)) == 0;
}

bool BeginsWith(const char *str, const char *prefix)
{
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

bool BeginsWith(const std::string &str, const std::string &prefix, const size_t prefixLength)
{
    return strncmp(str.c_str(), prefix.c_str(), prefixLength) == 0;
}

bool EndsWith(const std::string &str, const std::string &suffix)
{
    return EndsWithSuffix(str.c_str(), str.length(), suffix.c_str(), suffix.length());
}

bool EndsWith(const std::string &str, const char *suffix)
{
    return EndsWithSuffix(str.c_str(), str.length(), suffix, strlen(suffix));
}

bool EndsWith(const char *str, const char *suffix)
{
    return EndsWithSuffix(str, strlen(str), suffix, strlen(suffix));
}

bool ContainsToken(const std::string &tokenStr, char delimiter, const std::string &token)
{
    if (token.empty())
    {
        return false;
    }
    // Compare token with all sub-strings terminated by delimiter or end of string
    std::string::size_type start = 0u;
    do
    {
        std::string::size_type end = tokenStr.find(delimiter, start);
        if (end == std::string::npos)
        {
            end = tokenStr.length();
        }
        const std::string::size_type length = end - start;
        if (length == token.length() && tokenStr.compare(start, length, token) == 0)
        {
            return true;
        }
        start = end + 1u;
    } while (start < tokenStr.size());
    return false;
}

void ToLower(std::string *str)
{
    for (char &ch : *str)
    {
        ch = static_cast<char>(::tolower(ch));
    }
}

void ToUpper(std::string *str)
{
    for (char &ch : *str)
    {
        ch = static_cast<char>(::toupper(ch));
    }
}

bool ReplaceSubstring(std::string *str,
                      const std::string &substring,
                      const std::string &replacement)
{
    size_t replacePos = str->find(substring);
    if (replacePos == std::string::npos)
    {
        return false;
    }
    str->replace(replacePos, substring.size(), replacement);
    return true;
}

int ReplaceAllSubstrings(std::string *str,
                         const std::string &substring,
                         const std::string &replacement)
{
    int count = 0;
    while (ReplaceSubstring(str, substring, replacement))
    {
        count++;
    }
    return count;
}

std::string ToCamelCase(const std::string &str)
{
    std::string result;

    bool lastWasUnderscore = false;
    for (char c : str)
    {
        if (c == '_')
        {
            lastWasUnderscore = true;
            continue;
        }

        if (lastWasUnderscore)
        {
            c                 = static_cast<char>(std::toupper(c));
            lastWasUnderscore = false;
        }
        result += c;
    }

    return result;
}

std::vector<std::string> GetStringsFromEnvironmentVarOrAndroidProperty(const char *varName,
                                                                       const char *propertyName,
                                                                       const char *separator)
{
    std::string environment = GetEnvironmentVarOrAndroidProperty(varName, propertyName);
    return SplitString(environment, separator, TRIM_WHITESPACE, SPLIT_WANT_NONEMPTY);
}

std::vector<std::string> GetCachedStringsFromEnvironmentVarOrAndroidProperty(
    const char *varName,
    const char *propertyName,
    const char *separator)
{
    std::string environment = GetEnvironmentVarOrAndroidProperty(varName, propertyName);
    return SplitString(environment, separator, TRIM_WHITESPACE, SPLIT_WANT_NONEMPTY);
}

// glob can have * as wildcard
bool NamesMatchWithWildcard(const char *glob, const char *name)
{
    // Find the first * in glob.
    const char *firstWildcard = strchr(glob, '*');

    // If there are no wildcards, match the strings precisely.
    if (firstWildcard == nullptr)
    {
        return strcmp(glob, name) == 0;
    }

    // Otherwise, match up to the wildcard first.
    size_t preWildcardLen = firstWildcard - glob;
    if (strncmp(glob, name, preWildcardLen) != 0)
    {
        return false;
    }

    const char *postWildcardRef = glob + preWildcardLen + 1;

    // As a small optimization, if the wildcard is the last character in glob, accept the match
    // already.
    if (postWildcardRef[0] == '\0')
    {
        return true;
    }

    // Try to match the wildcard with a number of characters.
    for (size_t matchSize = 0; name[matchSize] != '\0'; ++matchSize)
    {
        if (NamesMatchWithWildcard(postWildcardRef, name + matchSize))
        {
            return true;
        }
    }

    return false;
}

}  // namespace angle
