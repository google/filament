// This file is part of gltfpack; see gltfpack.h for version/license details
#include "gltfpack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

TempFile::TempFile()
	: fd(-1)
{
}

TempFile::TempFile(const char* suffix)
    : fd(-1)
{
	create(suffix);
}

TempFile::~TempFile()
{
	if (!path.empty())
		remove(path.c_str());

#ifndef _WIN32
	if (fd >= 0)
		close(fd);
#endif
}

void TempFile::create(const char* suffix)
{
	assert(fd < 0 && path.empty());

#if defined(_WIN32)
	const char* temp_dir = getenv("TEMP");
	path = temp_dir ? temp_dir : ".";
	path += "\\gltfpack-XXXXXX";
	(void)_mktemp(&path[0]);
	path += suffix;
#elif defined(__wasi__)
	static int id = 0;
	char ids[16];
	snprintf(ids, sizeof(ids), "%d", id++);

	path = "gltfpack-temp-";
	path += ids;
	path += suffix;
#else
	path = "/tmp/gltfpack-XXXXXX";
	path += suffix;
	fd = mkstemps(&path[0], strlen(suffix));
#endif
}

std::string getFullPath(const char* path, const char* base_path)
{
	std::string result = base_path;

	std::string::size_type slash = result.find_last_of("/\\");
	result.erase(slash == std::string::npos ? 0 : slash + 1);

	result += path;

	return result;
}

std::string getFileName(const char* path)
{
	std::string result = path;

	std::string::size_type slash = result.find_last_of("/\\");
	if (slash != std::string::npos)
		result.erase(0, slash + 1);

	std::string::size_type dot = result.find_last_of('.');
	if (dot != std::string::npos)
		result.erase(dot);

	return result;
}

std::string getExtension(const char* path)
{
	std::string result = path;

	std::string::size_type slash = result.find_last_of("/\\");
	std::string::size_type dot = result.find_last_of('.');

	if (slash != std::string::npos && dot != std::string::npos && dot < slash)
		dot = std::string::npos;

	result.erase(0, dot);

	for (size_t i = 0; i < result.length(); ++i)
		if (unsigned(result[i] - 'A') < 26)
			result[i] = (result[i] - 'A') + 'a';

	return result;
}

bool readFile(const char* path, std::string& data)
{
	FILE* file = fopen(path, "rb");
	if (!file)
		return false;

	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (length <= 0)
	{
		fclose(file);
		return false;
	}

	data.resize(length);
	size_t result = fread(&data[0], 1, data.size(), file);
	int rc = fclose(file);

	return rc == 0 && result == data.size();
}

bool writeFile(const char* path, const std::string& data)
{
	FILE* file = fopen(path, "wb");
	if (!file)
		return false;

	size_t result = fwrite(&data[0], 1, data.size(), file);
	int rc = fclose(file);

	return rc == 0 && result == data.size();
}
