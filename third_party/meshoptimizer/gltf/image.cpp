// This file is part of gltfpack; see gltfpack.h for version/license details
#include "gltfpack.h"

#include <string.h>

static const char* kMimeTypes[][2] = {
    {"image/jpeg", ".jpg"},
    {"image/jpeg", ".jpeg"},
    {"image/png", ".png"},
};

static const char* inferMimeType(const char* path)
{
	std::string ext = getExtension(path);

	for (size_t i = 0; i < sizeof(kMimeTypes) / sizeof(kMimeTypes[0]); ++i)
		if (ext == kMimeTypes[i][1])
			return kMimeTypes[i][0];

	return "";
}

static bool parseDataUri(const char* uri, std::string& mime_type, std::string& result)
{
	if (strncmp(uri, "data:", 5) == 0)
	{
		const char* comma = strchr(uri, ',');

		if (comma && comma - uri >= 7 && strncmp(comma - 7, ";base64", 7) == 0)
		{
			const char* base64 = comma + 1;
			size_t base64_size = strlen(base64);
			size_t size = base64_size - base64_size / 4;

			if (base64_size >= 2)
			{
				size -= base64[base64_size - 2] == '=';
				size -= base64[base64_size - 1] == '=';
			}

			void* data = 0;

			cgltf_options options = {};
			cgltf_result res = cgltf_load_buffer_base64(&options, size, base64, &data);

			if (res != cgltf_result_success)
				return false;

			mime_type = std::string(uri + 5, comma - 7);
			result = std::string(static_cast<const char*>(data), size);

			free(data);

			return true;
		}
	}

	return false;
}

bool readImage(const cgltf_image& image, const char* input_path, std::string& data, std::string& mime_type)
{
	if (image.uri && parseDataUri(image.uri, mime_type, data))
	{
		return true;
	}
	else if (image.buffer_view && image.buffer_view->buffer->data && image.mime_type)
	{
		const cgltf_buffer_view* view = image.buffer_view;

		data.assign(static_cast<const char*>(view->buffer->data) + view->offset, view->size);
		mime_type = image.mime_type;
		return true;
	}
	else if (image.uri && *image.uri)
	{
		std::string path = image.uri;

		cgltf_decode_uri(&path[0]);
		path.resize(strlen(&path[0]));

		mime_type = image.mime_type ? image.mime_type : inferMimeType(path.c_str());

		return readFile(getFullPath(path.c_str(), input_path).c_str(), data);
	}
	else
	{
		return false;
	}
}

static int readInt16(const std::string& data, size_t offset)
{
	return (unsigned char)data[offset] * 256 + (unsigned char)data[offset + 1];
}

static int readInt32(const std::string& data, size_t offset)
{
	return (unsigned((unsigned char)data[offset]) << 24) |
	       (unsigned((unsigned char)data[offset + 1]) << 16) |
	       (unsigned((unsigned char)data[offset + 2]) << 8) |
	       unsigned((unsigned char)data[offset + 3]);
}

static bool getDimensionsPng(const std::string& data, int& width, int& height)
{
	if (data.size() < 8 + 8 + 13 + 4)
		return false;

	const char* signature = "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a";
	if (data.compare(0, 8, signature) != 0)
		return false;

	if (data.compare(12, 4, "IHDR") != 0)
		return false;

	width = readInt32(data, 16);
	height = readInt32(data, 20);

	return true;
}

static bool getDimensionsJpeg(const std::string& data, int& width, int& height)
{
	size_t offset = 0;

	// note, this can stop parsing before reaching the end but we stop at SOF anyway
	while (offset + 4 <= data.size())
	{
		if (data[offset] != '\xff')
			return false;

		char marker = data[offset + 1];

		if (marker == '\xff')
		{
			offset++;
			continue; // padding
		}

		// d0..d9 correspond to SOI, RSTn, EOI
		if (marker == 0 || unsigned(marker - '\xd0') <= 9)
		{
			offset += 2;
			continue; // no payload
		}

		// c0..c1 correspond to SOF0, SOF1
		if (marker == '\xc0' || marker == '\xc2')
		{
			if (offset + 10 > data.size())
				return false;

			width = readInt16(data, offset + 7);
			height = readInt16(data, offset + 5);

			return true;
		}

		offset += 2 + readInt16(data, offset + 2);
	}

	return false;
}

static bool hasTransparencyPng(const std::string& data)
{
	if (data.size() < 8 + 8 + 13 + 4)
		return false;

	const char* signature = "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a";
	if (data.compare(0, 8, signature) != 0)
		return false;

	if (data.compare(12, 4, "IHDR") != 0)
		return false;

	int ctype = data[25];

	if (ctype != 3)
		return ctype == 4 || ctype == 6;

	size_t offset = 8; // reparse IHDR chunk for simplicity

	while (offset + 12 <= data.size())
	{
		int length = readInt32(data, offset);

		if (length < 0)
			return false;

		if (data.compare(offset + 4, 4, "tRNS") == 0)
			return true;

		offset += 12 + length;
	}

	return false;
}

bool hasAlpha(const std::string& data, const char* mime_type)
{
	if (strcmp(mime_type, "image/png") == 0)
		return hasTransparencyPng(data);
	else
		return false;
}

bool getDimensions(const std::string& data, const char* mime_type, int& width, int& height)
{
	if (strcmp(mime_type, "image/png") == 0)
		return getDimensionsPng(data, width, height);
	if (strcmp(mime_type, "image/jpeg") == 0)
		return getDimensionsJpeg(data, width, height);

	return false;
}

static int roundPow2(int value)
{
	int result = 1;

	while (result < value)
		result <<= 1;

	// to prevent odd texture sizes from increasing the size too much, we round to nearest power of 2 above a certain size
	if (value > 128 && result * 3 / 4 > value)
		result >>= 1;

	return result;
}

static int roundBlock(int value, bool pow2)
{
	if (value == 0)
		return 4;

	if (pow2 && value > 4)
		return roundPow2(value);

	return (value + 3) & ~3;
}

void adjustDimensions(int& width, int& height, const Settings& settings)
{
	width = int(width * settings.texture_scale);
	height = int(height * settings.texture_scale);

	if (settings.texture_limit && (width > settings.texture_limit || height > settings.texture_limit))
	{
		float limit_scale = float(settings.texture_limit) / float(width > height ? width : height);

		width = int(width * limit_scale);
		height = int(height * limit_scale);
	}

	width = roundBlock(width, settings.texture_pow2);
	height = roundBlock(height, settings.texture_pow2);
}

const char* mimeExtension(const char* mime_type)
{
	for (size_t i = 0; i < sizeof(kMimeTypes) / sizeof(kMimeTypes[0]); ++i)
		if (strcmp(kMimeTypes[i][0], mime_type) == 0)
			return kMimeTypes[i][1];

	return ".raw";
}
