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
/// \file X3DImporter_Macro.hpp
/// \brief Useful macrodefines.
/// \date 2015-2016
/// \author smal.root@gmail.com

#ifndef X3DIMPORTER_MACRO_HPP_INCLUDED
#define X3DIMPORTER_MACRO_HPP_INCLUDED

/// \def MACRO_USE_CHECKANDAPPLY(pDEF, pUSE, pNE)
/// Used for regular checking while attribute "USE" is defined.
/// \param [in] pDEF - string holding "DEF" value.
/// \param [in] pUSE - string holding "USE" value.
/// \param [in] pType - type of element to find.
/// \param [out] pNE - pointer to found node element.
#define MACRO_USE_CHECKANDAPPLY(pDEF, pUSE, pType, pNE) \
	do { \
	XML_CheckNode_MustBeEmpty(); \
	if(!pDEF.empty()) Throw_DEF_And_USE(); \
	if(!FindNodeElement(pUSE, CX3DImporter_NodeElement::pType, &pNE)) Throw_USE_NotFound(pUSE); \
	 \
	NodeElement_Cur->Child.push_back(pNE);/* add found object as child to current element */ \
	} while(false)

/// \def MACRO_ATTRREAD_LOOPBEG
/// Begin of loop that read attributes values.
#define MACRO_ATTRREAD_LOOPBEG \
	for(int idx = 0, idx_end = mReader->getAttributeCount(); idx < idx_end; idx++) \
	{ \
		std::string an(mReader->getAttributeName(idx));

/// \def MACRO_ATTRREAD_LOOPEND
/// End of loop that read attributes values.
#define MACRO_ATTRREAD_LOOPEND \
		Throw_IncorrectAttr(an); \
	}

/// \def MACRO_ATTRREAD_CHECK_REF
/// Check current attribute name and if it equal to requested then read value. Result write to output variable by reference. If result was read then
/// "continue" will called.
/// \param [in] pAttrName - attribute name.
/// \param [out] pVarName - output variable name.
/// \param [in] pFunction - function which read attribute value and write it to pVarName.
#define MACRO_ATTRREAD_CHECK_REF(pAttrName, pVarName, pFunction) \
	if(an == pAttrName) \
	{ \
		pFunction(idx, pVarName); \
		continue; \
	}

/// \def MACRO_ATTRREAD_CHECK_RET
/// Check current attribute name and if it equal to requested then read value. Result write to output variable using return value of \ref pFunction.
/// If result was read then  "continue" will called.
/// \param [in] pAttrName - attribute name.
/// \param [out] pVarName - output variable name.
/// \param [in] pFunction - function which read attribute value and write it to pVarName.
#define MACRO_ATTRREAD_CHECK_RET(pAttrName, pVarName, pFunction) \
	if(an == pAttrName) \
	{ \
		pVarName = pFunction(idx); \
		continue; \
	}

/// \def MACRO_ATTRREAD_CHECKUSEDEF_RET
/// Compact variant for checking "USE" and "DEF". Also skip bbox attributes: "bboxCenter", "bboxSize".
/// If result was read then  "continue" will called.
/// \param [out] pDEF_Var - output variable name for "DEF" value.
/// \param [out] pUSE_Var - output variable name for "USE" value.
#define MACRO_ATTRREAD_CHECKUSEDEF_RET(pDEF_Var, pUSE_Var) \
	MACRO_ATTRREAD_CHECK_RET("DEF", pDEF_Var, mReader->getAttributeValue); \
	MACRO_ATTRREAD_CHECK_RET("USE", pUSE_Var, mReader->getAttributeValue); \
	if(an == "bboxCenter") continue; \
	if(an == "bboxSize") continue; \
	if(an == "containerField") continue; \
	do {} while(false)

/// \def MACRO_NODECHECK_LOOPBEGIN(pNodeName)
/// Begin of loop of parsing child nodes. Do not add ';' at end.
/// \param [in] pNodeName - current node name.
#define MACRO_NODECHECK_LOOPBEGIN(pNodeName) \
	do { \
	bool close_found = false; \
	 \
	while(mReader->read()) \
	{ \
		if(mReader->getNodeType() == irr::io::EXN_ELEMENT) \
		{

/// \def MACRO_NODECHECK_LOOPEND(pNodeName)
/// End of loop of parsing child nodes.
/// \param [in] pNodeName - current node name.
#define MACRO_NODECHECK_LOOPEND(pNodeName) \
		}/* if(mReader->getNodeType() == irr::io::EXN_ELEMENT) */ \
		else if(mReader->getNodeType() == irr::io::EXN_ELEMENT_END) \
		{ \
			if(XML_CheckNode_NameEqual(pNodeName)) \
			{ \
				close_found = true; \
	 \
				break; \
			} \
		}/* else if(mReader->getNodeType() == irr::io::EXN_ELEMENT_END) */ \
	}/* while(mReader->read()) */ \
	 \
	if(!close_found) Throw_CloseNotFound(pNodeName); \
	 \
	} while(false)

#define MACRO_NODECHECK_METADATA(pNodeName) \
	MACRO_NODECHECK_LOOPBEGIN(pNodeName) \
			/* and childs must be metadata nodes */ \
			if(!ParseHelper_CheckRead_X3DMetadataObject()) XML_CheckNode_SkipUnsupported(pNodeName); \
	 MACRO_NODECHECK_LOOPEND(pNodeName)

/// \def MACRO_FACE_ADD_QUAD_FA(pCCW, pOut, pIn, pP1, pP2, pP3, pP4)
/// Add points as quad. Means that pP1..pP4 set in CCW order.
#define MACRO_FACE_ADD_QUAD_FA(pCCW, pOut, pIn, pP1, pP2, pP3, pP4) \
	do { \
	if(pCCW) \
	{ \
		pOut.push_back(pIn[pP1]); \
		pOut.push_back(pIn[pP2]); \
		pOut.push_back(pIn[pP3]); \
		pOut.push_back(pIn[pP4]); \
	} \
	else \
	{ \
		pOut.push_back(pIn[pP4]); \
		pOut.push_back(pIn[pP3]); \
		pOut.push_back(pIn[pP2]); \
		pOut.push_back(pIn[pP1]); \
	} \
	} while(false)

/// \def MACRO_FACE_ADD_QUAD(pCCW, pOut, pP1, pP2, pP3, pP4)
/// Add points as quad. Means that pP1..pP4 set in CCW order.
#define MACRO_FACE_ADD_QUAD(pCCW, pOut, pP1, pP2, pP3, pP4) \
	do { \
	if(pCCW) \
	{ \
		pOut.push_back(pP1); \
		pOut.push_back(pP2); \
		pOut.push_back(pP3); \
		pOut.push_back(pP4); \
	} \
	else \
	{ \
		pOut.push_back(pP4); \
		pOut.push_back(pP3); \
		pOut.push_back(pP2); \
		pOut.push_back(pP1); \
	} \
	} while(false)

#endif // X3DIMPORTER_MACRO_HPP_INCLUDED
