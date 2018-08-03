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

#include <vector>
#include <string>

BEGIN_ODDLPARSER_NS

// Forward declarations
class IOStreamBase;
class Value;
class OpenDDLParser;

struct Identifier;
struct Reference;
struct Property;
struct DataArrayList;

///
/// @ingroup    OpenDDLParser
///	@brief      This class represents one single instance in the object tree of the parsed OpenDDL-file.
///
/// A DDLNode represents one leaf in the OpenDDL-node tree. It can have one parent node and multiple children.
/// You can assign special properties to a single DDLNode instance.
///	A node instance can store values via a linked list. You can get the first value from the DDLNode.
/// A node can store data-array-lists and references as well.
///
class DLL_ODDLPARSER_EXPORT DDLNode {
public:
    friend class OpenDDLParser;

    /// @brief  The child-node-list type.
    typedef std::vector<DDLNode*> DllNodeList;

    /// @brief  The child-node-list iterator.
    typedef std::vector<DDLNode*>::iterator DDLNodeIt;

public:
    ///	@brief  The class destructor.
    ~DDLNode();

    ///	@brief  Will attach a parent node instance, an older one will be released.
    /// @param  parent      [in] The parent node instance.
    void attachParent( DDLNode *parent );

    /// @brief  Will try to detach a parent node instance, if there is any.
    void detachParent();

    ///	@brief  Returns the assigned parent node instance, will return ddl_nullptr id no parent is assigned.
    /// @return The parent node instance.
    DDLNode *getParent() const;

    ///	@brief  Returns the child node list.
    /// @return The list of child nodes.
    const DllNodeList &getChildNodeList() const;

    /// Set the type of the DDLNode instance.
    /// @param  type    [in] The type.
    void setType( const std::string &type );

    /// @brief  Returns the type of the DDLNode instance.
    /// @return The type of the DDLNode instance.
    const std::string &getType() const;

    /// Set the name of the DDLNode instance.
    /// @param  name        [in] The name.
    void setName( const std::string &name );

    /// @brief  Returns the name of the DDLNode instance.
    /// @return The name of the DDLNode instance.
    const std::string &getName() const;

    /// @brief  Set a new property set.
    ///	@param  prop        [in] The first element of the property set.
    void setProperties( Property *prop );

    ///	@brief  Returns the first element of the assigned property set.
    ///	@return The first property of the assigned property set.
    Property *getProperties() const;

    ///	@brief  Looks for a given property.
    /// @param  name        [in] The name for the property to look for.
    /// @return true, if a corresponding property is assigned to the node, false if not.
    bool hasProperty( const std::string &name );

    ///	@brief  Will return true, if any properties are assigned to the node instance.
    ///	@return True, if properties are assigned.
    bool hasProperties() const;

    ///	@brief  Search for a given property and returns it. Will return ddl_nullptr if no property was found.
    /// @param  name        [in] The name for the property to look for.
    /// @return The property or ddl_nullptr if no property was found.
    Property *findPropertyByName( const std::string &name );
    
    /// @brief  Set a new value set.
    /// @param  val         [in] The first value instance of the value set.
    void setValue( Value *val );

    ///	@brief  Returns the first element of the assigned value set.
    ///	@return The first property of the assigned value set.
    Value *getValue() const;

    /// @brief  Set a new DataArrayList.
    /// @param  dtArrayList [in] The DataArrayList instance.
    void setDataArrayList( DataArrayList *dtArrayList );

    ///	@brief  Returns the DataArrayList.
    ///	@return The DataArrayList.
    DataArrayList *getDataArrayList() const;

    /// @brief  Set a new Reference set.
    /// @param  refs        [in] The first value instance of the Reference set.
    void setReferences( Reference  *refs );

    ///	@brief  Returns the first element of the assigned Reference set.
    ///	@return The first property of the assigned Reference set.
    Reference *getReferences() const;

    /// @brief  Will dump the node into the stream.
    /// @param  stream      [in] The stream to write to.
    void dump(IOStreamBase &stream);

    ///	@brief  The creation method.
    /// @param  type        [in] The DDLNode type.
    ///	@param  name        [in] The name for the new DDLNode instance.
    /// @param  parent      [in] The parent node instance or ddl_nullptr if no parent node is there.
    /// @return The new created node instance.
    static DDLNode *create( const std::string &type, const std::string &name, DDLNode *parent = ddl_nullptr );

private:
    DDLNode( const std::string &type, const std::string &name, size_t idx, DDLNode *parent = ddl_nullptr );
    DDLNode();
    DDLNode( const DDLNode & ) ddl_no_copy;
    DDLNode &operator = ( const DDLNode & ) ddl_no_copy;
    static void releaseNodes();

private:
    std::string m_type;
    std::string m_name;
    DDLNode *m_parent;
    std::vector<DDLNode*> m_children;
    Property *m_properties;
    Value *m_value;
    DataArrayList *m_dtArrayList;
    Reference *m_references;
    size_t m_idx;
    static DllNodeList s_allocatedNodes;
};

END_ODDLPARSER_NS
