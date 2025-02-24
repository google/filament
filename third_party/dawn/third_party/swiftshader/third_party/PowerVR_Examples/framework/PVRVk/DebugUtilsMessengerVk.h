/*!
\brief The PVRVk DebugUtilsMessenger class.
\file PVRVk/DebugUtilsMessengerVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/PVRVkObjectBaseVk.h"
#include "PVRVk/TypesVk.h"

namespace pvrvk {
/// <summary>DebugUtilsObjectNameInfo structure used to set the name associated with an object.</summary>
struct DebugUtilsObjectNameInfo
{
public:
	/// <summary>Constructor</summary>
	DebugUtilsObjectNameInfo() : _objectType(ObjectType::e_UNKNOWN), _objectHandle(static_cast<uint64_t>(-1)), _objectName("") {}

	/// <summary>Constructor</summary>
	/// <param name="objectType">The ObjectType of the object</param>
	/// <param name="objectHandle">The object handle</param>
	/// <param name="objectName">The name of the object</param>
	DebugUtilsObjectNameInfo(ObjectType objectType, uint64_t objectHandle, const std::string& objectName)
		: _objectType(objectType), _objectHandle(objectHandle), _objectName(objectName)
	{}

	/// <summary>Get the object type</summary>
	/// <returns>The object type of the structure</returns>
	ObjectType getObjectType() const { return _objectType; }

	/// <summary>Set the type of the object</summary>
	/// <param name="objectType">A new object type</param>
	void setObjectType(ObjectType objectType) { this->_objectType = objectType; }

	/// <summary>Get the object handle</summary>
	/// <returns>The object handle of the structure</returns>
	uint64_t getObjectHandle() const { return _objectHandle; }

	/// <summary>Set the handle of the object</summary>
	/// <param name="objectHandle">A new object handle</param>
	void setObjectHandle(uint64_t objectHandle) { this->_objectHandle = objectHandle; }

	/// <summary>Get the object name</summary>
	/// <returns>The object name of the structure</returns>
	const std::string& getObjectName() const { return _objectName; }

	/// <summary>Set the name of the object</summary>
	/// <param name="objectName">A new object name</param>
	void setObjectName(const std::string& objectName) { this->_objectName = objectName; }

private:
	ObjectType _objectType;
	uint64_t _objectHandle;
	std::string _objectName;
};

/// <summary>DebugUtilsObjectTagInfo structure used to set the tag associated with an object.</summary>
struct DebugUtilsObjectTagInfo
{
public:
	/// <summary>Constructor</summary>
	DebugUtilsObjectTagInfo() : _objectType(ObjectType::e_UNKNOWN), _objectHandle(static_cast<uint64_t>(-1)), _tagName(static_cast<uint64_t>(-1)), _tagSize(0), _tag(nullptr) {}

	/// <summary>Constructor</summary>
	/// <param name="objectType">The ObjectType of the object</param>
	/// <param name="objectHandle">The object handle</param>
	/// <param name="tagName">The name of the object</param>
	/// <param name="tagSize">The size of a tag</param>
	/// <param name="pTag">A tag to use</param>
	DebugUtilsObjectTagInfo(ObjectType objectType, uint64_t objectHandle, uint64_t tagName, size_t tagSize, const void* pTag)
		: _objectType(objectType), _objectHandle(objectHandle), _tagName(tagName), _tagSize(tagSize), _tag(pTag)
	{}

	/// <summary>Get the object type</summary>
	/// <returns>The object type of the structure</returns>
	ObjectType getObjectType() const { return _objectType; }

	/// <summary>Set the type of the object</summary>
	/// <param name="objectType">A new object type</param>
	void setObjectType(ObjectType objectType) { this->_objectType = objectType; }

	/// <summary>Get the object handle</summary>
	/// <returns>The object handle of the structure</returns>
	uint64_t getObjectHandle() const { return _objectHandle; }

	/// <summary>Set the handle of the object</summary>
	/// <param name="objectHandle">A new object handle</param>
	void setObjectHandle(uint64_t objectHandle) { this->_objectHandle = objectHandle; }

	/// <summary>Get the object tag name</summary>
	/// <returns>The object tag name of the structure</returns>
	uint64_t getTagName() const { return _tagName; }

	/// <summary>Set the tag name of the object</summary>
	/// <param name="tagName">A new object tag name</param>
	void setTagName(uint64_t tagName) { this->_tagName = tagName; }

	/// <summary>Get the object handle</summary>
	/// <returns>The object handle of the structure</returns>
	size_t getTagSize() const { return this->_tagSize; }

	/// <summary>Set the tag size for the object</summary>
	/// <param name="tagSize">A new tag size for the object</param>
	void setTagSize(size_t tagSize) { this->_tagSize = tagSize; }

	/// <summary>Get the object tag</summary>
	/// <returns>The object tag of the structure</returns>
	const void* getTag() const { return _tag; }

	/// <summary>Set the tag for the object</summary>
	/// <param name="tag">A new tag for the object handle</param>
	void setTag(const void* tag) { this->_tag = tag; }

private:
	ObjectType _objectType;
	uint64_t _objectHandle;
	uint64_t _tagName;
	size_t _tagSize;
	const void* _tag;
};

/// <summary>DebugUtilsLabel used to define a label.</summary>
struct DebugUtilsLabel
{
public:
	/// <summary>Constructor</summary>
	DebugUtilsLabel() : _labelName("")
	{
		_color[0] = { 0.0f };
		_color[1] = { 0.0f };
		_color[2] = { 0.0f };
		_color[3] = { 0.0f };
	}

	/// <summary>Constructor</summary>
	/// <param name="labelName">The label to use</param>
	/// <param name="colorR">The red color component</param>
	/// <param name="colorG">The green color component</param>
	/// <param name="colorB">The blue color component</param>
	/// <param name="colorA">The alpha color component</param>
	DebugUtilsLabel(const std::string& labelName, float colorR = 183.0f / 255.0f, float colorG = 26.0f / 255.0f, float colorB = 139.0f / 255.0f, float colorA = 1.0f)
		: _labelName(labelName)
	{
		_color[0] = colorR;
		_color[1] = colorG;
		_color[2] = colorB;
		_color[3] = colorA;
	}

	/// <summary>Get the label</summary>
	/// <returns>The label name</returns>
	const std::string& getLabelName() const { return _labelName; }

	/// <summary>Set the label</summary>
	/// <param name="labelName">A new label name</param>
	void setLabelName(const std::string& labelName) { this->_labelName = labelName; }

	/// <summary>Get red floating point component</summary>
	/// <returns>Red component</returns>
	float getR() const { return _color[0]; }

	/// <summary>Set red floating point component</summary>
	/// <param name="r">Red component</param>
	void setR(const float r) { this->_color[0] = r; }

	/// <summary>Get green floating point component</summary>
	/// <returns>Green component</returns>
	float getG() const { return _color[1]; }

	/// <summary>Set green floating point component</summary>
	/// <param name="g">green component</param>
	void setG(const float g) { this->_color[1] = g; }

	/// <summary>Get blue floating point component</summary>
	/// <returns>Blue component</returns>
	float getB() const { return _color[2]; }

	/// <summary>Set blue floating point component</summary>
	/// <param name="b">Blue component</param>
	void setB(const float b) { this->_color[2] = b; }

	/// <summary>Get alpha floating point component</summary>
	/// <returns>Alpha component</returns>
	float getA() const { return _color[3]; }

	/// <summary>Set alpha floating point component</summary>
	/// <param name="a">Alpha component</param>
	void setA(const float a) { this->_color[3] = a; }

private:
	std::string _labelName;
	float _color[4];
};

/// <summary>DebugUtilsMessengerCallbackData callback data structure.</summary>
struct DebugUtilsMessengerCallbackData
{
public:
	/// <summary>Constructor (zero initialization)</summary>
	DebugUtilsMessengerCallbackData()
		: _flags(DebugUtilsMessengerCallbackDataFlagsEXT::e_NONE), _messageIdName(""), _messageIdNumber(-1), _message(""), _queueLabels(std::vector<DebugUtilsLabel>()),
		  _cmdBufLabels(std::vector<DebugUtilsLabel>()), _objects(std::vector<DebugUtilsObjectNameInfo>())
	{}

	/// <summary>Constructor</summary>
	/// <param name="flags">A set of flags reserved for future use.</param>
	/// <param name="messageIdName">A bitmask specifying the type of events which will cause the callback to be called.</param>
	/// <param name="messageIdNumber">The application callback function to call.</param>
	/// <param name="message">The userdata which will be passed to the application callback function.</param>
	/// <param name="queueLabels">A list of relevant queue labels.</param>
	/// <param name="cmdBufLabels">A list of relevant command buffer labels.</param>
	/// <param name="objects">A list of relevant objects.</param>
	DebugUtilsMessengerCallbackData(DebugUtilsMessengerCallbackDataFlagsEXT flags, const std::string& messageIdName, int32_t messageIdNumber, const std::string& message,
		std::vector<DebugUtilsLabel>& queueLabels, std::vector<DebugUtilsLabel>& cmdBufLabels, std::vector<DebugUtilsObjectNameInfo>& objects)
		: _flags(flags), _messageIdName(messageIdName), _messageIdNumber(messageIdNumber), _message(message), _queueLabels(queueLabels), _cmdBufLabels(cmdBufLabels), _objects(objects)
	{}

	/// <summary>Get the message callback flags</summary>
	/// <returns>The message callback flags</returns>
	DebugUtilsMessengerCallbackDataFlagsEXT getFlags() const { return _flags; }

	/// <summary>Set the message callback flags</summary>
	/// <param name="flags">The message callback flags</param>
	void setFlags(DebugUtilsMessengerCallbackDataFlagsEXT flags) { this->_flags = flags; }

	/// <summary>Get the message id name</summary>
	/// <returns>The id name of the message</returns>
	const std::string& getMessageIdName() const { return _messageIdName; }

	/// <summary>Set the id name of the message</summary>
	/// <param name="messageIdName">A new id name for the message</param>
	void setMessageIdName(const std::string& messageIdName) { this->_messageIdName = messageIdName; }

	/// <summary>Get the message id number</summary>
	/// <returns>The id of the message</returns>
	int32_t getMessageIdNumber() const { return _messageIdNumber; }

	/// <summary>Set the id of the message</summary>
	/// <param name="messageIdNumber">A new id for the message</param>
	void setMessageIdName(int32_t messageIdNumber) { this->_messageIdNumber = messageIdNumber; }

	/// <summary>Get the message</summary>
	/// <returns>The message</returns>
	const std::string& getMessage() const { return _message; }

	/// <summary>Set the message</summary>
	/// <param name="message">A new message</param>
	void setMessage(const std::string& message) { this->_message = message; }

	/// <summary>Get the number of queue labels</summary>
	/// <returns>The number of queue labels</returns>
	inline uint32_t getNumQueueLabels() const { return static_cast<uint32_t>(_queueLabels.size()); }
	/// <summary>Get the list of queue labels</summary>
	/// <returns>The list of queue labels</returns>
	inline const std::vector<DebugUtilsLabel>& getQueueLabels() const { return _queueLabels; }
	/// <summary>Get the queue label at the index specified</summary>
	/// <param name="index">The index of the queue label to retrieve</param>
	/// <returns>The queue label retrieved</returns>
	inline const DebugUtilsLabel& getQueueLabel(uint32_t index) const { return _queueLabels[index]; }
	/// <summary>Sets the queue labels list</summary>
	/// <param name="queueLabels">A list of queue labels to add</param>
	inline void setQueueLabels(const std::vector<DebugUtilsLabel>& queueLabels)
	{
		this->_queueLabels.resize(queueLabels.size());
		std::copy(queueLabels.begin(), queueLabels.end(), this->_queueLabels.begin());
	}
	/// <summary>Adds a new queue label to the list of queue labels</summary>
	/// <param name="queueLabel">A new queue label to add to the existing list of queue labels</param>
	inline void addQueueLabel(const DebugUtilsLabel& queueLabel) { this->_queueLabels.emplace_back(queueLabel); }

	/// <summary>Get the number of cmdBuf labels</summary>
	/// <returns>The number of cmdBuf labels</returns>
	inline uint32_t getNumCmdBufLabels() const { return static_cast<uint32_t>(_cmdBufLabels.size()); }
	/// <summary>Get the list of cmdBuf labels</summary>
	/// <returns>The list of cmdBuf labels</returns>
	inline const std::vector<DebugUtilsLabel>& getCmdBufLabels() const { return _cmdBufLabels; }
	/// <summary>Get the cmdBuf label at the index specified</summary>
	/// <param name="index">The index of the cmdBuf label to retrieve</param>
	/// <returns>The cmdBuf label retrieved</returns>
	inline const DebugUtilsLabel& getCmdBufLabel(uint32_t index) const { return _cmdBufLabels[index]; }
	/// <summary>Sets the cmdBuf labels list</summary>
	/// <param name="cmdBufLabels">A list of cmdBuf labels to add</param>
	inline void setCmdBufLabels(const std::vector<DebugUtilsLabel>& cmdBufLabels)
	{
		this->_cmdBufLabels.resize(cmdBufLabels.size());
		std::copy(cmdBufLabels.begin(), cmdBufLabels.end(), this->_cmdBufLabels.begin());
	}
	/// <summary>Adds a new cmdBuf label to the list of cmdBuf labels</summary>
	/// <param name="cmdBufLabel">A new cmdBuf label to add to the existing list of cmdBuf labels</param>
	inline void addCmdBufLabel(const DebugUtilsLabel& cmdBufLabel) { this->_cmdBufLabels.emplace_back(cmdBufLabel); }

	/// <summary>Get the number of objects</summary>
	/// <returns>The number of objects</returns>
	inline uint32_t getNumObjects() const { return static_cast<uint32_t>(_objects.size()); }
	/// <summary>Get the list of objects</summary>
	/// <returns>The list of objects</returns>
	inline const std::vector<DebugUtilsObjectNameInfo>& getObjects() const { return _objects; }
	/// <summary>Get the cmdBuf label at the index specified</summary>
	/// <param name="index">The index of the cmdBuf label to retrieve</param>
	/// <returns>The cmdBuf label retrieved</returns>
	inline const DebugUtilsObjectNameInfo& getObject(uint32_t index) const { return _objects[index]; }
	/// <summary>Sets the objects list</summary>
	/// <param name="objects">A list of objects to add</param>
	inline void setObjects(const std::vector<DebugUtilsObjectNameInfo>& objects)
	{
		this->_objects.resize(objects.size());
		std::copy(objects.begin(), objects.end(), this->_objects.begin());
	}
	/// <summary>Adds a new cmdBuf label to the list of objects</summary>
	/// <param name="cmdBufLabel">A new cmdBuf label to add to the existing list of objects</param>
	inline void addCmdBufLabel(const DebugUtilsObjectNameInfo& cmdBufLabel) { this->_objects.emplace_back(cmdBufLabel); }

private:
	DebugUtilsMessengerCallbackDataFlagsEXT _flags;
	std::string _messageIdName;
	int32_t _messageIdNumber;
	std::string _message;
	std::vector<DebugUtilsLabel> _queueLabels;
	std::vector<DebugUtilsLabel> _cmdBufLabels;
	std::vector<DebugUtilsObjectNameInfo> _objects;
};

/// <summary>DebugUtilsMessengerCreateInfo creation descriptor.</summary>
struct DebugUtilsMessengerCreateInfo
{
public:
	/// <summary>Constructor (zero initialization)</summary>
	DebugUtilsMessengerCreateInfo()
		: _flags(DebugUtilsMessengerCreateFlagsEXT::e_NONE), _messageSeverity(DebugUtilsMessageSeverityFlagsEXT::e_NONE), _messageType(DebugUtilsMessageTypeFlagsEXT::e_NONE),
		  _callback(nullptr), _userData(nullptr)
	{}

	/// <summary>Constructor</summary>
	/// <param name="messageSeverity">A bitmask specifying which severity of events will cause the callback to be called.</param>
	/// <param name="messageType">A bitmask specifying the type of events which will cause the callback to be called.</param>
	/// <param name="callback">The application callback function to call.</param>
	/// <param name="pUserData">The userdata which will be passed to the application callback function.</param>
	/// <param name="flags">Reserved for future use.</param>
	DebugUtilsMessengerCreateInfo(DebugUtilsMessageSeverityFlagsEXT messageSeverity, DebugUtilsMessageTypeFlagsEXT messageType, PFN_vkDebugUtilsMessengerCallbackEXT callback,
		void* pUserData = nullptr, DebugUtilsMessengerCreateFlagsEXT flags = DebugUtilsMessengerCreateFlagsEXT::e_NONE)
		: _flags(flags), _messageSeverity(messageSeverity), _messageType(messageType), _callback(callback), _userData(pUserData)
	{}

	/// <summary>Get the flags for the creation info</summary>
	/// <returns>The DebugUtilsMessengerCreateFlagsEXT</returns>
	DebugUtilsMessengerCreateFlagsEXT getFlags() const { return _flags; }

	/// <summary>Set the DebugReportFlagsEXT which are reserved for future use.</summary>
	/// <param name="flags">Reserved for future use.</param>
	/// <returns>this (allow chaining)</returns>
	DebugUtilsMessengerCreateInfo& setFlags(const DebugUtilsMessengerCreateFlagsEXT& flags)
	{
		this->_flags = flags;
		return *this;
	}

	/// <summary>Get the severity flags for the creation info</summary>
	/// <returns>The DebugUtilsMessageSeverityFlagsEXT</returns>
	DebugUtilsMessageSeverityFlagsEXT getMessageSeverity() const { return _messageSeverity; }

	/// <summary>Set the DebugUtilsMessageSeverityFlagsEXT specifying which severity of events will cause the callback to be called.</summary>
	/// <param name="messageSeverity">A bitmask specifying which severity of events will cause the callback to be called.</param>
	/// <returns>this (allow chaining)</returns>
	DebugUtilsMessengerCreateInfo& setMessageSeverity(const DebugUtilsMessageSeverityFlagsEXT& messageSeverity)
	{
		this->_messageSeverity = messageSeverity;
		return *this;
	}

	/// <summary>Get the message type for the creation info</summary>
	/// <returns>The DebugUtilsMessageTypeFlagsEXT</returns>
	DebugUtilsMessageTypeFlagsEXT getMessageType() const { return _messageType; }

	/// <summary>Set the DebugUtilsMessageTypeFlagsEXT specifying the type of events which will cause the callback to be called.</summary>
	/// <param name="messageType">A bitmask specifying the type of events which will cause the callback to be called</param>
	/// <returns>this (allow chaining)</returns>
	DebugUtilsMessengerCreateInfo& setMessageType(const DebugUtilsMessageTypeFlagsEXT& messageType)
	{
		this->_messageType = messageType;
		return *this;
	}

	/// <summary>Get the application callback function</summary>
	/// <returns>The PFN_vkDebugUtilsMessengerCallbackEXT callback function</returns>
	PFN_vkDebugUtilsMessengerCallbackEXT getCallback() const { return _callback; }

	/// <summary>Set the PFN_vkDebugUtilsMessengerCallbackEXT specifying the callback function which will be called.</summary>
	/// <param name="callback">The application callback function to call.</param>
	/// <returns>this (allow chaining)</returns>
	DebugUtilsMessengerCreateInfo& setCallback(const PFN_vkDebugUtilsMessengerCallbackEXT& callback)
	{
		this->_callback = callback;
		return *this;
	}

	/// <summary>Get the user data passed to the callback</summary>
	/// <returns>The userdata which will be passed to the application callback function</returns>
	inline void* getPUserData() const { return _userData; }

	/// <summary>Set the user data passed to the callback.</summary>
	/// <param name="pUserData">The userdata which will be passed to the application callback function.</param>
	/// <returns>this (allow chaining)</returns>
	inline void setPUserData(void* pUserData) { this->_userData = pUserData; }

private:
	/// <summary>Reserved for future use.</summary>
	DebugUtilsMessengerCreateFlagsEXT _flags;
	/// <summary>A bitmask specifying which severity of events will cause the callback to be called.</summary>
	DebugUtilsMessageSeverityFlagsEXT _messageSeverity;
	/// <summary>A bitmask specifying the type of events which will cause the callback to be called.</summary>
	DebugUtilsMessageTypeFlagsEXT _messageType;
	/// <summary>The application callback function to call</summary>
	PFN_vkDebugUtilsMessengerCallbackEXT _callback;
	/// <summary>User data to be passed to the callback</summary>
	void* _userData;
};

namespace impl {
/// <summary>Vulkan DebugUtilsMessenger wrapper</summary>
class DebugUtilsMessenger_ : public PVRVkInstanceObjectBase<VkDebugUtilsMessengerEXT, ObjectType::e_DEBUG_UTILS_MESSENGER_EXT>
{
private:
	friend class Instance_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() = default;
		friend class DebugUtilsMessenger_;
	};

	static DebugUtilsMessenger constructShared(Instance& instance, const DebugUtilsMessengerCreateInfo& createInfo)
	{
		return std::make_shared<DebugUtilsMessenger_>(make_shared_enabler{}, instance, createInfo);
	}

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(DebugUtilsMessenger_)
	~DebugUtilsMessenger_();
	DebugUtilsMessenger_(make_shared_enabler, Instance& instance, const DebugUtilsMessengerCreateInfo& createInfo);
	//!\endcond
};
} // namespace impl
} // namespace pvrvk
