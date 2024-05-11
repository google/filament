/*
 * Copyright (C) 2021 The Android Open Source Project
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

#ifndef IMAGE_HDRDECODER_H_
#define IMAGE_HDRDECODER_H_

#include <imageio/ImageDecoder.h>

namespace image {

class HDRDecoder : public ImageDecoder::Decoder {
public:
    static HDRDecoder* create(std::istream& stream);
    static bool checkSignature(char const* buf);

    HDRDecoder(const HDRDecoder&) = delete;
    HDRDecoder& operator=(const HDRDecoder&) = delete;

private:
    explicit HDRDecoder(std::istream& stream);
    ~HDRDecoder() override;

    // ImageDecoder::Decoder interface
    LinearImage decode() override;

    static const char sigRadiance[];
    static const char sigRGBE[];
    std::istream& mStream;
    std::streampos mStreamStartPos;
};

} // namespace image

#endif /* IMAGE_IMAGEDECODER_H_ */
