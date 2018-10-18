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
/// \file   X3DImporter_Networking.cpp
/// \brief  Parsing data from nodes of "Networking" set of X3D.
/// \date   2015-2016
/// \author smal.root@gmail.com

#ifndef ASSIMP_BUILD_NO_X3D_IMPORTER

#include "X3DImporter.hpp"
#include "X3DImporter_Macro.hpp"

// Header files, Assimp.
#include <assimp/DefaultIOSystem.h>

//#include <regex>

namespace Assimp
{

//static std::regex pattern_parentDir(R"((^|/)[^/]+/../)");
static std::string parentDir("/../");

// <Inline
// DEF=""              ID
// USE=""              IDREF
// bboxCenter="0 0 0"  SFVec3f  [initializeOnly]
// bboxSize="-1 -1 -1" SFVec3f  [initializeOnly]
// load="true"         SFBool   [inputOutput]
// url=""              MFString [inputOutput]
// />
void X3DImporter::ParseNode_Networking_Inline()
{
    std::string def, use;
    bool load = true;
    std::list<std::string> url;

	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECKUSEDEF_RET(def, use);
		MACRO_ATTRREAD_CHECK_RET("load", load, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_REF("url", url, XML_ReadNode_GetAttrVal_AsListS);
	MACRO_ATTRREAD_LOOPEND;

	// if "USE" defined then find already defined element.
	if(!use.empty())
	{
		CX3DImporter_NodeElement* ne;

		MACRO_USE_CHECKANDAPPLY(def, use, ENET_Group, ne);
	}
	else
	{
		ParseHelper_Group_Begin(true);// create new grouping element and go deeper if node has children.
		// at this place new group mode created and made current, so we can name it.
		if(!def.empty()) NodeElement_Cur->ID = def;

		if(load && (url.size() > 0))
		{
			std::string full_path = mpIOHandler->CurrentDirectory() + url.front();

			//full_path = std::regex_replace(full_path, pattern_parentDir, "$1");
			for (std::string::size_type pos = full_path.find(parentDir); pos != std::string::npos; pos = full_path.find(parentDir, pos)) {
				if (pos > 0) {
					std::string::size_type pos2 = full_path.rfind('/', pos - 1);
					if (pos2 != std::string::npos) {
						full_path.erase(pos2, pos - pos2 + 3);
						pos = pos2;
					}
					else {
						full_path.erase(0, pos + 4);
						pos = 0;
					}
				}
				else {
					pos += 3;
				}
			}
			// Attribute "url" can contain list of strings. But we need only one - first.
			std::string::size_type slashPos = full_path.find_last_of("\\/");
			mpIOHandler->PushDirectory(slashPos == std::string::npos ? std::string() : full_path.substr(0, slashPos + 1));
			ParseFile(full_path, mpIOHandler);
			mpIOHandler->PopDirectory();
		}

		// check for X3DMetadataObject childs.
		if(!mReader->isEmptyElement()) ParseNode_Metadata(NodeElement_Cur, "Inline");

		// exit from node in that place
		ParseHelper_Node_Exit();
	}// if(!use.empty()) else
}

}// namespace Assimp

#endif // !ASSIMP_BUILD_NO_X3D_IMPORTER
