/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2018, assimp team


All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the
following conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----------------------------------------------------------------------
*/

/** @file   ObjTools.h
 *  @brief  Some helpful templates for text parsing
 */
#ifndef OBJ_TOOLS_H_INC
#define OBJ_TOOLS_H_INC

#include <assimp/fast_atof.h>
#include <assimp/ParsingUtils.h>
#include <vector>

namespace Assimp {

/** @brief  Returns true, if the last entry of the buffer is reached.
 *  @param  it  Iterator of current position.
 *  @param  end Iterator with end of buffer.
 *  @return true, if the end of the buffer is reached.
 */
template<class char_t>
inline bool isEndOfBuffer(  char_t it, char_t end ) {
    if ( it == end )
    {
        return true;
    }
    else
    {
        --end;
    }
    return ( it == end );
}

/** @brief  Returns next word separated by a space
 *  @param  pBuffer Pointer to data buffer
 *  @param  pEnd    Pointer to end of buffer
 *  @return Pointer to next space
 */
template<class Char_T>
inline Char_T getNextWord( Char_T pBuffer, Char_T pEnd )
{
    while ( !isEndOfBuffer( pBuffer, pEnd ) )
    {
        if ( !IsSpaceOrNewLine( *pBuffer ) || IsLineEnd( *pBuffer ) ) {
            //if ( *pBuffer != '\\' )
                break;
        }
        pBuffer++;
    }
    return pBuffer;
}

/** @brief  Returns pointer a next token
 *  @param  pBuffer Pointer to data buffer
 *  @param  pEnd    Pointer to end of buffer
 *  @return Pointer to next token
 */
template<class Char_T>
inline Char_T getNextToken( Char_T pBuffer, Char_T pEnd )
{
    while ( !isEndOfBuffer( pBuffer, pEnd ) )
    {
        if( IsSpaceOrNewLine( *pBuffer ) )
            break;
        pBuffer++;
    }
    return getNextWord( pBuffer, pEnd );
}

/** @brief  Skips a line
 *  @param  it      Iterator set to current position
 *  @param  end     Iterator set to end of scratch buffer for readout
 *  @param  uiLine  Current line number in format
 *  @return Current-iterator with new position
 */
template<class char_t>
inline char_t skipLine( char_t it, char_t end, unsigned int &uiLine ) {
    while( !isEndOfBuffer( it, end ) && !IsLineEnd( *it ) ) {
        ++it;
    }
    
    if ( it != end ) {
        ++it;
        ++uiLine;
    }
    // fix .. from time to time there are spaces at the beginning of a material line
    while ( it != end && ( *it == '\t' || *it == ' ' ) ) {
        ++it;
    }

    return it;
}

/** @brief  Get a name from the current line. Preserve space in the middle,
 *    but trim it at the end.
 *  @param  it      set to current position
 *  @param  end     set to end of scratch buffer for readout
 *  @param  name    Separated name
 *  @return Current-iterator with new position
 */
template<class char_t>
inline char_t getName( char_t it, char_t end, std::string &name )
{
    name = "";
    if( isEndOfBuffer( it, end ) ) {
        return end;
    }

    char *pStart = &( *it );
    while( !isEndOfBuffer( it, end ) && !IsLineEnd( *it )) {
        ++it;
    }

    while(IsSpace( *it ) ) {
        --it;
    }
    // Get name
    // if there is no name, and the previous char is a separator, come back to start
    while (&(*it) < pStart) {
        ++it;
    }
    std::string strName( pStart, &(*it) );
    if ( strName.empty() )
        return it;
    else
        name = strName;

    return it;
}

/** @brief  Get a name from the current line. Do not preserve space
 *    in the middle, but trim it at the end.
 *  @param  it      set to current position
 *  @param  end     set to end of scratch buffer for readout
 *  @param  name    Separated name
 *  @return Current-iterator with new position
 */
template<class char_t>
inline char_t getNameNoSpace( char_t it, char_t end, std::string &name )
{
    name = "";
    if( isEndOfBuffer( it, end ) ) {
        return end;
    }

    char *pStart = &( *it );
    while( !isEndOfBuffer( it, end ) && !IsLineEnd( *it )
          && !IsSpaceOrNewLine( *it ) ) {
        ++it;
    }

    while( isEndOfBuffer( it, end ) || IsLineEnd( *it )
          || IsSpaceOrNewLine( *it ) ) {
        --it;
    }
    ++it;

    // Get name
    // if there is no name, and the previous char is a separator, come back to start
    while (&(*it) < pStart) {
        ++it;
    }
    std::string strName( pStart, &(*it) );
    if ( strName.empty() )
        return it;
    else
        name = strName;

    return it;
}

/** @brief  Get next word from given line
 *  @param  it      set to current position
 *  @param  end     set to end of scratch buffer for readout
 *  @param  pBuffer Buffer for next word
 *  @param  length  Buffer length
 *  @return Current-iterator with new position
 */
template<class char_t>
inline char_t CopyNextWord( char_t it, char_t end, char *pBuffer, size_t length )
{
    size_t index = 0;
    it = getNextWord<char_t>( it, end );
    while( !IsSpaceOrNewLine( *it ) && !isEndOfBuffer( it, end ) )
    {
        pBuffer[index] = *it ;
        index++;
        if (index == length-1)
            break;
        ++it;
    }
    pBuffer[ index ] = '\0';
    return it;
}

/** @brief  Get next float from given line
 *  @param  it      set to current position
 *  @param  end     set to end of scratch buffer for readout
 *  @param  value   Separated float value.
 *  @return Current-iterator with new position
 */
template<class char_t>
inline char_t getFloat( char_t it, char_t end, ai_real &value )
{
    static const size_t BUFFERSIZE = 1024;
    char buffer[ BUFFERSIZE ];
    it = CopyNextWord<char_t>( it, end, buffer, BUFFERSIZE );
    value = (ai_real) fast_atof( buffer );

    return it;
}

/** @brief  Will perform a simple tokenize.
 *  @param  str         String to tokenize.
 *  @param  tokens      Array with tokens, will be empty if no token was found.
 *  @param  delimiters  Delimiter for tokenize.
 *  @return Number of found token.
 */
template<class string_type>
unsigned int tokenize( const string_type& str, std::vector<string_type>& tokens,
                         const string_type& delimiters )
{
    // Skip delimiters at beginning.
    typename string_type::size_type lastPos = str.find_first_not_of( delimiters, 0 );

    // Find first "non-delimiter".
    typename string_type::size_type pos = str.find_first_of( delimiters, lastPos );
    while ( string_type::npos != pos || string_type::npos != lastPos )
    {
        // Found a token, add it to the vector.
        string_type tmp = str.substr(lastPos, pos - lastPos);
        if ( !tmp.empty() && ' ' != tmp[ 0 ] )
            tokens.push_back( tmp );

        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of( delimiters, pos );

        // Find next "non-delimiter"
        pos = str.find_first_of( delimiters, lastPos );
    }

    return static_cast<unsigned int>( tokens.size() );
}

template <class string_type>
string_type trim_whitespaces(string_type str)
{
    while (!str.empty() && IsSpace(str[0])) str.erase(0);
    while (!str.empty() && IsSpace(str[str.length() - 1])) str.erase(str.length() - 1);
    return str;
}

template<class T>
bool hasLineEnd( T it, T end ) {
    bool hasLineEnd( false );
    while ( !isEndOfBuffer( it, end ) ) {
        it++;
        if ( IsLineEnd( it ) ) {
            hasLineEnd = true;
            break;
        }
    }

    return hasLineEnd;
}

} // Namespace Assimp

#endif // OBJ_TOOLS_H_INC
