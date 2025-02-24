/*!
\brief Streams that are created from files.
\file PVRCore/stream/FileStream.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/stream/Stream.h"
#include <string>

namespace pvr {
/// <summary>A FileStream is a Stream that is used to access a File in the filesystem of the platform.</summary>
class FileStream : public Stream
{
protected:
	/// <summary>The underlying C FILE object</summary>
	mutable FILE* _file;
	/// <summary>The C flags used when opening the file</summary>
	std::string _flags;
	/// <summary>True to error when files are not found otherwise False to avoid an exception when the file is not found.</summary>
	bool _errorOnFileNotFound;

public:
	/// <summary>Create a new filestream of a specified file.</summary>
	/// <param name="filePath">The path of the file. Can be in any format the operating system understands (absolute,
	/// relative etc.)</param>
	/// <param name="flags">fopen-style flags.</param>
	/// <param name="errorOnFileNotFound">OPTIONAL. Set this to false to avoid an exception when the file is not found. If set to false,
	/// always check isReadable() or isWritable before using. Otherwise, a returned stream will always be opened with the correct flags.</param>
	/// <remarks>Possible flags: 'r':read,'w':truncate/write, 'a':append/write, r+: read/write, w+:truncate/read/write,
	/// a+:append/read/write</remarks>
	FileStream(const std::string& filePath, const std::string& flags, bool errorOnFileNotFound = true)
		: Stream(filePath, flags.find('r') != flags.npos || flags.find('+') != flags.npos,
			  flags.find('w') != flags.npos || flags.find('a') != flags.npos || flags.find('+') != flags.npos, true),
		  _file(NULL), _flags(flags), _errorOnFileNotFound(errorOnFileNotFound)
	{
		open();
	}

	~FileStream() { close(); }

	/// <summary>Create a new file stream from a filename</summary>
	/// <param name="filename">The filename to create a stream for</param>
	/// <param name="flags">The C++ open flags for the file ("r", "w", "a", "r+", "w+", "a+" and optionally "b"), see
	/// C++ standard.</param>
	/// <param name="errorOnFileNotFound">OPTIONAL. Set this to false to avoid an error when the file is not found.</param>
	/// <returns>Return a valid file stream, else Return null if it fails</returns>
	/// <remarks>Flags correspond the C++ standard for fopen r: Read (read from beginning, preserve contents, fail if
	/// not exist) w: Write (write from beginning, destroy contents, create if not exist) a: Append (write to end only
	/// (regardles of file pointer position), preserve contents, create if not exist) r+: Read+ (read/write from
	/// start, preserve contents, fail if not exist) w+: Write+ (read/write from start, destroy contents, create if
	/// not exist) a+: Append+ (read/write from end (write only happens to end), preserve contents, create if not
	/// exist) b: Additional flag: Do no special handling of line break / form feed characters</remarks>
	static std::unique_ptr<Stream> createFileStream(const char* filename, const char* flags, bool errorOnFileNotFound = true)
	{
		auto stream = std::make_unique<FileStream>(filename, flags, errorOnFileNotFound);
		stream->open();
		return std::move(stream);
	}

	/// <summary>Create a new file stream from a filename</summary>
	/// <param name="filename">The filename to create a stream for</param>
	/// <param name="flags">The C++ open flags for the file ("r", "w", "a", "r+", "w+", "a+" and optionally "b"), see
	/// C++ standard.</param>
	/// <param name="errorOnFileNotFound">OPTIONAL. Set this to false to avoid an error when the file is not found.</param>
	/// <returns>Return a valid file stream, else Return null if it fails</returns>
	/// <remarks>Flags correspond the C++ standard for fopen r: Read (read from beginning, preserve contents, fail if
	/// not exist) w: Write (write from beginning, destroy contents, create if not exist) a: Append (write to end only
	/// (regardles of file pointer position), preserve contents, create if not exist) r+: Read+ (read/write from
	/// start, preserve contents, fail if not exist) w+: Write+ (read/write from start, destroy contents, create if
	/// not exist) a+: Append+ (read/write from end (write only happens to end), preserve contents, create if not
	/// exist) b: Additional flag: Do no special handling of line break / form feed characters</remarks>
	static std::unique_ptr<Stream> createFileStream(const std::string& filename, const char* flags, bool errorOnFileNotFound = true)
	{
		return createFileStream(filename.c_str(), flags, errorOnFileNotFound);
	}

private:
	virtual uint64_t _getPosition() const override
	{
		if (_file) { return static_cast<size_t>(ftell(_file)); }
		else
		{
			return 0;
		}
	}

	virtual uint64_t _getSize() const override
	{
		if (_file)
		{
			fseek(_file, 0L, SEEK_END);
			long fileSize = ftell(_file);
			fseek(_file, 0L, SEEK_SET);
			return static_cast<size_t>(fileSize);
		}
		else
		{
			return 0;
		}
	}
	virtual void _read(size_t elementSize, size_t numElements, void* buffer, size_t& dataRead) const override
	{
		dataRead = 0;
		if (!_file) { throw FileIOError(getFileName(), "[Filestream::read] Attempted to read empty stream."); }
		if (!_isReadable) { throw FileIOError(getFileName(), "[Filestream::read] Attempted to read non-readable stream."); }

		dataRead = fread(buffer, elementSize, numElements, _file);
		if (dataRead != numElements)
		{
			if (feof(_file) != 0) { throw FileEOFError(getFileName(), "[Filestream::read] Was attempting to read past the end of stream."); }
			else
			{
				throw FileIOError(getFileName(), "[Filestream::read] Unknown Error.");
			}
		}
	}

	virtual void _write(size_t elementSize, size_t numElements, const void* buffer, size_t& dataWritten) override
	{
		dataWritten = 0;
		if (!_file) { throw FileIOError(getFileName(), "[Filestream::read] Attempted to write an empty string."); }
		if (!_isWritable) { throw FileIOError(getFileName(), "[Filestream::read] Attempted to write a non-writable stream."); }
		dataWritten = fwrite(buffer, elementSize, numElements, _file);
		if (dataWritten != numElements)
		{
			if (feof(_file) != 0) { throw FileIOError(getFileName(), "[Filestream::write] Was attempting to write past the end of stream."); }
			else
			{
				throw FileIOError(getFileName(), "[Filestream::write] Unknown error");
			}
		}
	}

	virtual void _seek(long offset, SeekOrigin origin) const override
	{
		if (!_file)
		{
			if (offset) { throw FileIOError(getFileName(), "[FileStream::seek] Attempt to seek in empty stream."); }
		}
		else
		{
			if (fseek(_file, offset, static_cast<int>(origin)) != 0) { throw FileIOError(getFileName(), "[FileStream::seek] Attempt to seek  past the end of stream."); }
		}
	}

	void open()
	{
		if (_file) // If file exists, just reset it.
		{ return seek(0, SeekOriginFromStart); } // open:
		if (_fileName.length() == 0 || _flags.length() == 0) { throw InvalidOperationError("[FileStream::open] Attempted to open a nonexistent file"); }

#ifdef _WIN32
#ifdef _UNICODE
		errno_t error = _wfopen_s(&_file, _fileName.c_str(), _flags.c_str());
#else
		errno_t error = fopen_s(&_file, _fileName.c_str(), _flags.c_str());
#endif
		if (error != 0)
		{
			if (_errorOnFileNotFound) { throw FileNotFoundError(_fileName, "[FileStream::open] Failed to open file."); }
			else
			{
				_isReadable = false;
				_isWritable = false;
				_isRandomAccess = false;
				_file = nullptr;
				return;
			}
		}
#else
		_file = fopen(_fileName.c_str(), _flags.c_str());
#endif

		if (!_file)
		{
			if (_errorOnFileNotFound) { throw FileNotFoundError(_fileName, "[FileStream::open] Failed to open file."); }
			else
			{
				_isReadable = false;
				_isWritable = false;
				_isRandomAccess = false;
				_file = nullptr;
				return;
			}
		}
	}

	/// <summary>Closes the stream.</summary>
	void close()
	{
		if (_file && fclose(_file) == EOF) { throw FileIOError(getFileName(), "[FileStream::close] Failure closing file."); }
		_file = 0;
	}
};

} // namespace pvr
