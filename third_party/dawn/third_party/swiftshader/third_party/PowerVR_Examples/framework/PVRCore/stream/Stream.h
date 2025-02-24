/*!
\brief Contains a class used to abstract streams of data (files, blocks of memory, resources etc.).
\file PVRCore/stream/Stream.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Errors.h"
#include "PVRCore/strings/StringFunctions.h"
#include <memory>
#include <vector>
#include <string>

namespace pvr {

class Stream;
/// <summary>A simple std::runtime_error wrapper for throwing exceptions when undertaking File IO operations</summary>
class FileIOError : public PvrError
{
public:
	/// <summary>Constructor.</summary>
	explicit FileIOError(const Stream& stream);
	/// <summary>Constructor.</summary>
	/// <param name="stream">The stream being used.</param>
	/// <param name="message">A message to log alongside the exception message.</param>
	FileIOError(const Stream& stream, std::string message);
	/// <summary>Constructor.</summary>
	/// <param name="filenameOrMessage">A message to log alongside the exception message.</param>
	explicit FileIOError(std::string filenameOrMessage) : PvrError("[" + filenameOrMessage + "]: File IO operation failed") {}
	/// <summary>Constructor.</summary>
	/// <param name="filename">The filenae of the file being used.</param>
	/// <param name="message">A message to log alongside the exception message.</param>
	FileIOError(std::string filename, std::string message) : PvrError("[" + filename + "]: File IO operation failed - " + message) {}
};

/// <summary>A simple std::runtime_error wrapper for throwing exceptions when attempting to read past the End Of File</summary>
class FileEOFError : public PvrError
{
public:
	/// <summary>Constructor.</summary>
	explicit FileEOFError(const Stream& stream);
	/// <summary>Constructor.</summary>
	/// <param name="stream">The stream being used.</param>
	/// <param name="message">A message to log alongside the exception message.</param>
	FileEOFError(const Stream& stream, std::string message);
	/// <summary>Constructor.</summary>
	/// <param name="filenameOrMessage">A message to log alongside the exception message.</param>
	explicit FileEOFError(std::string filenameOrMessage) : PvrError("[" + filenameOrMessage + "]: Attempted to read past the end of file.") {}
	/// <summary>Constructor.</summary>
	/// <param name="filename">The filenae of the file being used.</param>
	/// <param name="message">A message to log alongside the exception message.</param>
	FileEOFError(std::string filename, std::string message) : PvrError("[" + filename + "]: Attempted to read past the end of file - " + message) {}
};

/// <summary>A simple std::runtime_error wrapper for throwing exceptions when file not found errors occur</summary>
class FileNotFoundError : public std::runtime_error
{
public:
	/// <summary>Constructor.</summary>
	explicit FileNotFoundError(const Stream& stream);
	/// <summary>Constructor.</summary>
	/// <param name="stream">The stream being used.</param>
	/// <param name="message">A message to log alongside the exception message.</param>
	FileNotFoundError(const Stream& stream, std::string message);
	/// <summary>Constructor.</summary>
	/// <param name="filenameOrMessage">A message to log alongside the exception message.</param>
	explicit FileNotFoundError(std::string filenameOrMessage) : std::runtime_error("[" + filenameOrMessage + "]: File not found") {}
	/// <summary>Constructor.</summary>
	/// <param name="filename">The filenae of the file being used.</param>
	/// <param name="message">A message to log alongside the exception message.</param>
	FileNotFoundError(std::string filename, std::string message) : std::runtime_error("[" + filename + "]: File not found - " + message) {}
};

/// <summary>This class is used to abstract streams of data (files, blocks of memory, resources etc.). In general a
/// stream is considered something that can be read or written from. Specializations for many different types of
/// streams are provided by the PowerVR Framework, the most commonly used ones being Files and Memory. The common
/// interface and pointer types allow the Stream to abstract data in a very useful manner.</summary>
class Stream
{
public:
	/// <summary>When seeking, select if your offset should be considered to be from the Start of the stream, the
	/// Current point in the stream or the End of the stream.</summary>
	enum SeekOrigin
	{
		SeekOriginFromStart,
		SeekOriginFromCurrent,
		SeekOriginFromEnd
	};

	/// <summary>Destructor (virtual, this class can and should be used polymorphically).</summary>
	virtual ~Stream() = default;

	/// <summary>Return true if this stream can be read from.</summary>
	/// <returns>True if this stream can be read from.</returns>
	bool isReadable() const { return _isReadable; }

	/// <summary>Return true if this stream can be written from.</summary>
	/// <returns>True if this stream can be written to.</returns>
	bool isWritable() const { return _isWritable; }

	/// <summary>Get the filename of the file that this std::string represents, if such exists. Otherwise, empty std::string.</summary>
	/// <returns>The filename of the file that this std::string represents, if such exists. Otherwise, empty std::string.</returns>
	const std::string& getFileName() const { return _fileName; }

public:
	/// <summary>Main read function. Read up to a specified amount of items into the provided buffer.</summary>
	/// <param name="elementSize">The size of each element that will be read.</param>
	/// <param name="numElements">The maximum number of elements to read.</param>
	/// <param name="buffer">The buffer into which to write the data.</param>
	/// <param name="dataRead">After returning, will contain the number of items that were actually read</param>
	void read(size_t elementSize, size_t numElements, void* buffer, size_t& dataRead) const
	{
		dataRead = 0;
		if (!_isReadable) { throw InvalidOperationError("[Stream::read]: Attempted to read non readable stream"); }
		_read(elementSize, numElements, buffer, dataRead);
	}

	/// <summary>Main read function. Read exactly a specified amount of items into the provided buffer, otherwise error.</summary>
	/// <param name="elementSize">The size of each element that will be read.</param>
	/// <param name="numElements">The maximum number of elements to read.</param>
	/// <param name="buffer">The buffer into which to write the data.</param>
	void readExact(size_t elementSize, size_t numElements, void* buffer) const
	{
		size_t dataRead;
		read(elementSize, numElements, buffer, dataRead);
		if (dataRead != numElements)
		{
			throw FileEOFError(*this,
				std::string("[Stream::readExact]: Failed to read specified number of elements. Size of element: [" + std::to_string(elementSize) + "]. Attempted to read [" +
					std::to_string(numElements) + "] but got [" + std::to_string(dataRead) + "]"));
		}
	}

	/// <summary>Main write function. Write into the stream the specified amount of items from a provided buffer.</summary>
	/// <param name="elementSize">The size of each element that will be written.</param>
	/// <param name="numElements">The number of elements to write.</param>
	/// <param name="buffer">The buffer from which to read the data. If the buffer is smaller than elementSize *
	/// numElements bytes, result is undefined.</param>
	/// <param name="dataWritten">After returning, will contain the number of items that were actually written. Will
	/// contain numElements unless an error has occured.</param>
	void write(size_t elementSize, size_t numElements, const void* buffer, size_t& dataWritten)
	{
		dataWritten = 0;
		if (!_isWritable) { throw InvalidOperationError("[Stream::write]: Attempt to write to non-writable stream"); }
		_write(elementSize, numElements, buffer, dataWritten);
	}

	/// <summary>Main write function. Write into the stream exactly the specified amount of items from a provided buffer,
	/// otherwise throw error</summary>
	/// <param name="elementSize">The size of each element that will be written.</param>
	/// <param name="numElements">The number of elements to write.</param>
	/// <param name="buffer">The buffer from which to read the data. If the buffer is smaller than elementSize *
	/// numElements bytes, result is undefined.</param>
	void writeExact(size_t elementSize, size_t numElements, const void* buffer)
	{
		size_t dataWritten;
		write(elementSize, numElements, buffer, dataWritten);
		if (dataWritten != numElements) { throw FileIOError(*this, std::string("Stream::writeExact: Failed to write specified number of elements.")); }
	}

	/// <summary>If supported, seek a specific point for random access streams. After successful call, subsequent operation will
	/// happen in the specified point.</summary>
	/// <param name="offset">The offset to seec from "origin"</param>
	/// <param name="origin">Beginning of stream, End of stream or Current position</param>
	void seek(long offset, SeekOrigin origin) const
	{
		if (!isRandomAccess()) { throw InvalidOperationError(pvr::strings::createFormatted("[pvr::Stream] Attempted to seek on non-seekable stream '%s'", getFileName().c_str())); }
		_seek(offset, origin);
	}

	/// <summary>Returns true if a stream supports seek, otherwise false.</summary>
	/// <returns>True if a stream supports seek, otherwise false</summary>
	bool isSeekable() const { return isRandomAccess(); }

	/// <summary>Returns true if a stream supports seek, otherwise false.</summary>
	/// <returns>True if a stream supports seek, otherwise false</summary>
	bool isRandomAccess() const { return _isRandomAccess; }

	/// <summary>If supported, returns the current position in the stream.</summary>
	/// <returns>If suppored, returns the current position in the stream. Otherwise, returns 0.</returns>
	size_t getPosition() const { return size_t(_getPosition()); }

	/// <summary>If supported, returns the current position in the stream.</summary>
	/// <returns>If suppored, returns the current position in the stream. Otherwise, returns 0.</returns>
	uint64_t getPosition64() const { return _getPosition(); }

	/// <summary>If supported, returns the total size of the stream.</summary>
	/// <returns>If suppored, returns the total amount of data in the stream. Otherwise, returns 0.</returns>
	size_t getSize() const { return size_t(_getSize()); }

	/// <summary>If supported, returns the total size of the stream, always in 64 bit precision.</summary>
	/// <returns>If suppored, returns the total amount of data in the stream. Otherwise, returns 0.</returns>
	uint64_t getSize64() const { return _getSize(); }

	/// <summary>Convenience functions that reads all data in the stream into a contiguous block of memory of a specified
	/// element type. Requires random-access stream.</summary>
	/// <typeparam name="Type_">The type of item that will be read into.</typeparam>
	/// <returns>A std::vector of Type_ containing all data from the current point to the end of the stream.</returns>
	template<typename Type_>
	std::vector<Type_> readToEnd() const
	{
		std::vector<Type_> ret;
		uint64_t mySize = getSize() - getPosition();
		uint64_t numElements = mySize / sizeof(Type_);
		ret.resize(size_t(numElements));
		size_t actuallyRead;
		read(sizeof(Type_), size_t(numElements), ret.data(), actuallyRead);
		return ret;
	}

	/// <summary>Convenience function that reads all data in the stream into a raw, contiguous block of memory. Requires
	/// random-access stream.</summary>
	/// <param name="outString">A std::vector<char> that will contain all data in the stream.</param>
	/// <returns>true if successful, false otherwise.</returns>
	void readIntoCharBuffer(std::vector<char>& outString) const
	{
		outString.resize((size_t)getSize() + 1);

		size_t dataRead;
		read(1, (size_t)getSize(), outString.data(), dataRead);
	}

	/// <summary>Convenience function that reads all data in the stream into a raw, contiguous block of memory. Requires
	/// random-access stream.</summary>
	/// <param name="outString">A std::vector<char> that will contain all data in the stream.</param>
	/// <returns>true if successful, false otherwise.</returns>
	template<typename T_>
	void readIntoBuffer(std::vector<T_>& outString) const
	{
		size_t initial_size = outString.size();
		outString.resize(size_t(initial_size + getSize()));

		size_t dataRead;
		return read(sizeof(T_), size_t(getSize()), outString.data() + initial_size, dataRead);
	}

	/// <summary>Convenience function that reads all data in the stream into a raw, contiguous block of memory. Requires
	/// random-access stream.</summary>
	/// <returns>A std::vector<char> that will contain all data in the stream.</returns>
	std::vector<char> readChars() const
	{
		std::vector<char> pData;
		readIntoCharBuffer(pData);
		return pData;
	}

	/// <summary>Convenience function that reads all data in the stream into a std::string</summary>
	/// <param name="outString">The string where the stream's data will all be saved</param>
	void readIntoString(std::string& outString) const
	{
		uint64_t sz = getSize();
		outString.resize((size_t)sz);

		if (sz > 0) { readExact(1, (size_t)getSize(), &outString[0]); }
	}
	/// <summary>Convenience function that reads all data in the stream into a std::string</summary>
	/// <returns>A string containing the stream's data</returns>
	std::string readString() const
	{
		uint64_t sz = getSize();
		std::string outString;
		outString.resize((size_t)sz);

		if (sz > 0) { readExact(1, (size_t)getSize(), &outString[0]); }
		return outString;
	}

protected:
	/// <summary>Constructor. Open a stream to a new filename. Must be overriden as it only sets the filename.</summary>
	/// <param name="fileName">Commmonly the filename, but may be any type of resource identifier (such as Windows
	/// Embedded Resource id)</param>
	explicit Stream(const std::string& fileName, bool readable, bool writable, bool seekable)
		: _isReadable(readable), _isWritable(writable), _isRandomAccess(seekable), _fileName(fileName)
	{}
	/// <summary>True if the stream can be read</summary>
	bool _isReadable;
	/// <summary>True if the stream can be written</summary>
	bool _isWritable;
	/// <summary>True if the stream is random read write</summary>
	bool _isRandomAccess;
	/// <summary>The filename (conceptually, a resource identifier as there may be other sources for the stream)</summary>
	std::string _fileName;

private:
	/// <summary>Override this function to implement a random access stream. After successful call, subsequent operation will
	/// happen in the specified point.</summary>
	/// <param name="offset">The offset to seec from "origin"</param>
	/// <param name="origin">Beginning of stream, End of stream or Current position</param>
	virtual void _seek(long offset, SeekOrigin origin) const = 0;

	virtual void _write(size_t elementSize, size_t numElements, const void* buffer, size_t& dataWritten) = 0;

	virtual void _read(size_t elementSize, size_t numElements, void* buffer, size_t& dataRead) const = 0;

	virtual uint64_t _getPosition() const = 0;
	virtual uint64_t _getSize() const = 0;

	// Disable copying and assign.
	Stream& operator=(const Stream&) = delete;
	Stream(const Stream&) = delete;
};

/// <summary>Constructor</summary>
/// <param name="stream">The stream being used.</param>
inline FileIOError::FileIOError(const Stream& stream) : PvrError("[" + stream.getFileName() + "]: File IO operation failed") {}
/// <summary>Constructor</summary>
/// <param name="stream">The stream being used.</param>
/// <param name="message">A message to log alongside the exception message.</param>
inline FileIOError::FileIOError(const Stream& stream, std::string message) : PvrError("[" + stream.getFileName() + "] : File IO operation failed - " + message) {}

/// <summary>Constructor</summary>
/// <param name="stream">The stream being used.</param>
inline FileEOFError::FileEOFError(const Stream& stream) : PvrError("[" + stream.getFileName() + "]: File IO operation failed") {}
/// <summary>Constructor</summary>
/// <param name="stream">The stream being used.</param>
/// <param name="message">A message to log alongside the exception message.</param>
inline FileEOFError::FileEOFError(const Stream& stream, std::string message) : PvrError("[" + stream.getFileName() + "] : File IO operation failed - " + message) {}

/// <summary>Constructor</summary>
/// <param name="stream">The stream being used.</param>
inline FileNotFoundError::FileNotFoundError(const Stream& stream) : std::runtime_error("[" + stream.getFileName() + "]: File not found.") {}
/// <summary>Constructor</summary>
/// <param name="stream">The stream being used.</param>
/// <param name="message">A message to log alongside the exception message.</param>
inline FileNotFoundError::FileNotFoundError(const Stream& stream, std::string message) : std::runtime_error("[" + stream.getFileName() + "] : File not found - " + message) {}
} // namespace pvr
