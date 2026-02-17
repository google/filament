/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <imagediff/ImageDiff.h>
#include <imageio/ImageDecoder.h>
#include <imageio/ImageEncoder.h>
#include <imageio-lite/ImageDecoder.h>
#include <imageio-lite/ImageEncoder.h>
#include <image/LinearImage.h>
#include <utils/Path.h>
#include <utils/CString.h>

#include <fstream>
#include <iostream>
#include <string>
#include <getopt/getopt.h>

using namespace utils;
using namespace image;

static void printUsage(const char* name) {
    std::cerr << "Usage: " << name << " [options] <reference> <candidate>\n"
              << "Options:\n"
              << "  --config <path>   Path to JSON configuration file.\n"
              << "  --mask <path>     Path to mask image.\n"
              << "  --diff <path>     Path to output difference image.\n"
              << "  --help, -h        Print this help message.\n";
}

static LinearImage loadImage(const Path& path) {
    if (!path.exists()) {
        std::cerr << "Error: File not found: " << path << std::endl;
        return LinearImage();
    }

    auto decodeWithImageIO = [](std::istream& stream, const std::string& name) {
        return image::ImageDecoder::decode(stream, name, image::ImageDecoder::ColorSpace::LINEAR);
    };

    auto decodeWithImageIOLite = [](std::istream& stream, const std::string& name) {
        return imageio_lite::ImageDecoder::decode(stream, CString(name.c_str()),
                imageio_lite::ImageDecoder::ColorSpace::LINEAR);
    };

    std::string ext = path.getExtension();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    bool preferLite = (ext == "tif" || ext == "tiff");

    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        std::cerr << "Error: Could not open file: " << path << std::endl;
        return LinearImage();
    }

    LinearImage img;
    if (preferLite) {
        img = decodeWithImageIOLite(stream, path.getName());
        if (!img.isValid()) {
            stream.clear();
            stream.seekg(0, std::ios::beg);
            img = decodeWithImageIO(stream, path.getName());
        }
    } else {
        img = decodeWithImageIO(stream, path.getName());
        if (!img.isValid()) {
            stream.clear();
            stream.seekg(0, std::ios::beg);
            img = decodeWithImageIOLite(stream, path.getName());
        }
    }

    if (!img.isValid()) {
         std::cerr << "Error: Could not decode image: " << path << std::endl;
    }
    return img;
}

static std::string readFile(const Path& path) {
    std::ifstream t(path);
    if (!t) return "";
    t.seekg(0, std::ios::end);
    size_t size = t.tellg();
    std::string buffer(size, ' ');
    t.seekg(0);
    t.read(&buffer[0], size);
    return buffer;
}

static void saveDiffImage(const Path& path, const LinearImage& image) {
    std::ofstream stream(path, std::ios::binary);
    if (!stream) {
        std::cerr << "Error: Could not open output file: " << path << std::endl;
        return;
    }

    std::string ext = path.getExtension();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    bool success = false;
    if (ext == "tif" || ext == "tiff") {
        success = imageio_lite::ImageEncoder::encode(stream,
            imageio_lite::ImageEncoder::Format::TIFF,
            image, "", CString(path.getName().c_str()));
    } else {
        image::ImageEncoder::Format format = image::ImageEncoder::chooseFormat(path.getName());
        success = image::ImageEncoder::encode(stream, format, image, "", path.getName());
    }

    if (!success) {
        std::cerr << "Error: Failed to write difference image to " << path << std::endl;
    }
}

int main(int argc, char** argv) {
    static struct option longOptions[] = {
        {"config", required_argument, 0, 'c'},
        {"mask", required_argument, 0, 'm'},
        {"diff", required_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    Path configPath;
    Path maskPath;
    Path diffPath;

    int opt;
    int optionIndex = 0;
    while ((opt = getopt_long(argc, argv, "c:m:d:h", longOptions, &optionIndex)) != -1) {
        switch (opt) {
            case 'c':
                configPath = Path(optarg);
                break;
            case 'm':
                maskPath = Path(optarg);
                break;
            case 'd':
                diffPath = Path(optarg);
                break;
            case 'h':
                printUsage(argv[0]);
                return 0;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }

    if (optind + 2 > argc) {
        std::cerr << "Error: Missing arguments." << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    Path refPath(argv[optind]);
    Path candPath(argv[optind + 1]);

    LinearImage refImg = loadImage(refPath);
    if (!refImg.isValid()) return 1;

    LinearImage candImg = loadImage(candPath);
    if (!candImg.isValid()) return 1;

    LinearImage maskImg;
    if (!maskPath.isEmpty()) {
        maskImg = loadImage(maskPath);
        if (!maskImg.isValid()) return 1;
    }

    imagediff::ImageDiffConfig config;
    if (!configPath.isEmpty()) {
        std::string jsonContent = readFile(configPath);
        if (jsonContent.empty()) {
            std::cerr << "Error: Could not read config file: " << configPath << std::endl;
            return 1;
        }
        if (!imagediff::parseConfig(jsonContent.c_str(), jsonContent.size(), &config)) {
            std::cerr << "Error: Failed to parse config file." << std::endl;
            return 1;
        }
    } else {
        config.mode = imagediff::ImageDiffConfig::Mode::LEAF;
        config.maxAbsDiff = 0.0f;
    }

    bool generateDiff = !diffPath.isEmpty();
    imagediff::ImageDiffResult result = imagediff::compare(refImg, candImg, config,
            maskImg.isValid() ? &maskImg : nullptr, generateDiff);

    if (generateDiff && result.diffImage.isValid()) {
        saveDiffImage(diffPath, result.diffImage);
    }

    utils::CString jsonOutput = imagediff::serializeResult(result);
    std::cout << jsonOutput.c_str() << std::endl;

    return 0;
}
