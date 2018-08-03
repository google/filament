/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2017, assimp team


All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the following
conditions are met:

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
---------------------------------------------------------------------------
*/

/// \file AMFImporter_Geometry.cpp
/// \brief Parsing data from geometry nodes.
/// \date 2016
/// \author smal.root@gmail.com

#ifndef ASSIMP_BUILD_NO_AMF_IMPORTER

#include "AMFImporter.hpp"
#include "AMFImporter_Macro.hpp"

namespace Assimp
{

// <mesh>
// </mesh>
// A 3D mesh hull.
// Multi elements - Yes.
// Parent element - <object>.
void AMFImporter::ParseNode_Mesh()
{
CAMFImporter_NodeElement* ne;

	// create new mesh object.
	ne = new CAMFImporter_NodeElement_Mesh(mNodeElement_Cur);
	// Check for child nodes
	if(!mReader->isEmptyElement())
	{
		bool vert_read = false;

		ParseHelper_Node_Enter(ne);
		MACRO_NODECHECK_LOOPBEGIN("mesh");
			if(XML_CheckNode_NameEqual("vertices"))
			{
				// Check if data already defined.
				if(vert_read) Throw_MoreThanOnceDefined("vertices", "Only one vertices set can be defined for <mesh>.");
				// read data and set flag about it
				ParseNode_Vertices();
				vert_read = true;

				continue;
			}

			if(XML_CheckNode_NameEqual("volume")) { ParseNode_Volume(); continue; }
		MACRO_NODECHECK_LOOPEND("mesh");
		ParseHelper_Node_Exit();
	}// if(!mReader->isEmptyElement())
	else
	{
		mNodeElement_Cur->Child.push_back(ne);// Add element to child list of current element
	}// if(!mReader->isEmptyElement()) else

	mNodeElement_List.push_back(ne);// and to node element list because its a new object in graph.
}

// <vertices>
// </vertices>
// The list of vertices to be used in defining triangles.
// Multi elements - No.
// Parent element - <mesh>.
void AMFImporter::ParseNode_Vertices()
{
CAMFImporter_NodeElement* ne;

	// create new mesh object.
	ne = new CAMFImporter_NodeElement_Vertices(mNodeElement_Cur);
	// Check for child nodes
	if(!mReader->isEmptyElement())
	{
		ParseHelper_Node_Enter(ne);
		MACRO_NODECHECK_LOOPBEGIN("vertices");
			if(XML_CheckNode_NameEqual("vertex")) { ParseNode_Vertex(); continue; }
		MACRO_NODECHECK_LOOPEND("vertices");
		ParseHelper_Node_Exit();
	}// if(!mReader->isEmptyElement())
	else
	{
		mNodeElement_Cur->Child.push_back(ne);// Add element to child list of current element
	}// if(!mReader->isEmptyElement()) else

	mNodeElement_List.push_back(ne);// and to node element list because its a new object in graph.
}

// <vertex>
// </vertex>
// A vertex to be referenced in triangles.
// Multi elements - Yes.
// Parent element - <vertices>.
void AMFImporter::ParseNode_Vertex()
{
CAMFImporter_NodeElement* ne;

	// create new mesh object.
	ne = new CAMFImporter_NodeElement_Vertex(mNodeElement_Cur);
	// Check for child nodes
	if(!mReader->isEmptyElement())
	{
		bool col_read = false;
		bool coord_read = false;

		ParseHelper_Node_Enter(ne);
		MACRO_NODECHECK_LOOPBEGIN("vertex");
			if(XML_CheckNode_NameEqual("color"))
			{
				// Check if data already defined.
				if(col_read) Throw_MoreThanOnceDefined("color", "Only one color can be defined for <vertex>.");
				// read data and set flag about it
				ParseNode_Color();
				col_read = true;

				continue;
			}

			if(XML_CheckNode_NameEqual("coordinates"))
			{
				// Check if data already defined.
				if(coord_read) Throw_MoreThanOnceDefined("coordinates", "Only one coordinates set can be defined for <vertex>.");
				// read data and set flag about it
				ParseNode_Coordinates();
				coord_read = true;

				continue;
			}

			if(XML_CheckNode_NameEqual("metadata")) { ParseNode_Metadata(); continue; }
		MACRO_NODECHECK_LOOPEND("vertex");
		ParseHelper_Node_Exit();
	}// if(!mReader->isEmptyElement())
	else
	{
		mNodeElement_Cur->Child.push_back(ne);// Add element to child list of current element
	}// if(!mReader->isEmptyElement()) else

	mNodeElement_List.push_back(ne);// and to node element list because its a new object in graph.
}

// <coordinates>
// </coordinates>
// Specifies the 3D location of this vertex.
// Multi elements - No.
// Parent element - <vertex>.
//
// Children elements:
//   <x>, <y>, <z>
//   Multi elements - No.
//   X, Y, or Z coordinate, respectively, of a vertex position in space.
void AMFImporter::ParseNode_Coordinates()
{
CAMFImporter_NodeElement* ne;

	// create new color object.
	ne = new CAMFImporter_NodeElement_Coordinates(mNodeElement_Cur);

	CAMFImporter_NodeElement_Coordinates& als = *((CAMFImporter_NodeElement_Coordinates*)ne);// alias for convenience

	// Check for child nodes
	if(!mReader->isEmptyElement())
	{
		bool read_flag[3] = { false, false, false };

		ParseHelper_Node_Enter(ne);
		MACRO_NODECHECK_LOOPBEGIN("coordinates");
			MACRO_NODECHECK_READCOMP_F("x", read_flag[0], als.Coordinate.x);
			MACRO_NODECHECK_READCOMP_F("y", read_flag[1], als.Coordinate.y);
			MACRO_NODECHECK_READCOMP_F("z", read_flag[2], als.Coordinate.z);
		MACRO_NODECHECK_LOOPEND("coordinates");
		ParseHelper_Node_Exit();
		// check that all components was defined
		if((read_flag[0] && read_flag[1] && read_flag[2]) == 0) throw DeadlyImportError("Not all coordinate's components are defined.");

	}// if(!mReader->isEmptyElement())
	else
	{
		mNodeElement_Cur->Child.push_back(ne);// Add element to child list of current element
	}// if(!mReader->isEmptyElement()) else

	mNodeElement_List.push_back(ne);// and to node element list because its a new object in graph.
}

// <volume
// materialid="" - Which material to use.
// type=""       - What this volume describes can be “region” or “support”. If none specified, “object” is assumed. If support, then the geometric
//                 requirements 1-8 listed in section 5 do not need to be maintained.
// >
// </volume>
// Defines a volume from the established vertex list.
// Multi elements - Yes.
// Parent element - <mesh>.
void AMFImporter::ParseNode_Volume()
{
std::string materialid;
std::string type;
CAMFImporter_NodeElement* ne;

	// Read attributes for node <color>.
	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECK_RET("materialid", materialid, mReader->getAttributeValue);
		MACRO_ATTRREAD_CHECK_RET("type", type, mReader->getAttributeValue);
	MACRO_ATTRREAD_LOOPEND;

	// create new object.
	ne = new CAMFImporter_NodeElement_Volume(mNodeElement_Cur);
	// and assign read data
	((CAMFImporter_NodeElement_Volume*)ne)->MaterialID = materialid;
	((CAMFImporter_NodeElement_Volume*)ne)->Type = type;
	// Check for child nodes
	if(!mReader->isEmptyElement())
	{
		bool col_read = false;

		ParseHelper_Node_Enter(ne);
		MACRO_NODECHECK_LOOPBEGIN("volume");
			if(XML_CheckNode_NameEqual("color"))
			{
				// Check if data already defined.
				if(col_read) Throw_MoreThanOnceDefined("color", "Only one color can be defined for <volume>.");
				// read data and set flag about it
				ParseNode_Color();
				col_read = true;

				continue;
			}

			if(XML_CheckNode_NameEqual("triangle")) { ParseNode_Triangle(); continue; }
			if(XML_CheckNode_NameEqual("metadata")) { ParseNode_Metadata(); continue; }
		MACRO_NODECHECK_LOOPEND("volume");
		ParseHelper_Node_Exit();
	}// if(!mReader->isEmptyElement())
	else
	{
		mNodeElement_Cur->Child.push_back(ne);// Add element to child list of current element
	}// if(!mReader->isEmptyElement()) else

	mNodeElement_List.push_back(ne);// and to node element list because its a new object in graph.
}

// <triangle>
// </triangle>
// Defines a 3D triangle from three vertices, according to the right-hand rule (counter-clockwise when looking from the outside).
// Multi elements - Yes.
// Parent element - <volume>.
//
// Children elements:
//   <v1>, <v2>, <v3>
//   Multi elements - No.
//   Index of the desired vertices in a triangle or edge.
void AMFImporter::ParseNode_Triangle()
{
CAMFImporter_NodeElement* ne;

	// create new color object.
	ne = new CAMFImporter_NodeElement_Triangle(mNodeElement_Cur);

	CAMFImporter_NodeElement_Triangle& als = *((CAMFImporter_NodeElement_Triangle*)ne);// alias for convenience

	// Check for child nodes
	if(!mReader->isEmptyElement())
	{
		bool col_read = false, tex_read = false;
		bool read_flag[3] = { false, false, false };

		ParseHelper_Node_Enter(ne);
		MACRO_NODECHECK_LOOPBEGIN("triangle");
			if(XML_CheckNode_NameEqual("color"))
			{
				// Check if data already defined.
				if(col_read) Throw_MoreThanOnceDefined("color", "Only one color can be defined for <triangle>.");
				// read data and set flag about it
				ParseNode_Color();
				col_read = true;

				continue;
			}

			if(XML_CheckNode_NameEqual("texmap"))// new name of node: "texmap".
			{
				// Check if data already defined.
				if(tex_read) Throw_MoreThanOnceDefined("texmap", "Only one texture coordinate can be defined for <triangle>.");
				// read data and set flag about it
				ParseNode_TexMap();
				tex_read = true;

				continue;
			}
			else if(XML_CheckNode_NameEqual("map"))// old name of node: "map".
			{
				// Check if data already defined.
				if(tex_read) Throw_MoreThanOnceDefined("map", "Only one texture coordinate can be defined for <triangle>.");
				// read data and set flag about it
				ParseNode_TexMap(true);
				tex_read = true;

				continue;
			}

			MACRO_NODECHECK_READCOMP_U32("v1", read_flag[0], als.V[0]);
			MACRO_NODECHECK_READCOMP_U32("v2", read_flag[1], als.V[1]);
			MACRO_NODECHECK_READCOMP_U32("v3", read_flag[2], als.V[2]);
		MACRO_NODECHECK_LOOPEND("triangle");
		ParseHelper_Node_Exit();
		// check that all components was defined
		if((read_flag[0] && read_flag[1] && read_flag[2]) == 0) throw DeadlyImportError("Not all vertices of the triangle are defined.");

	}// if(!mReader->isEmptyElement())
	else
	{
		mNodeElement_Cur->Child.push_back(ne);// Add element to child list of current element
	}// if(!mReader->isEmptyElement()) else

	mNodeElement_List.push_back(ne);// and to node element list because its a new object in graph.
}

}// namespace Assimp

#endif // !ASSIMP_BUILD_NO_AMF_IMPORTER
