#include "VzTexture.h"
#include "../VzEngineApp.h"
#include "../FIncludes.h"

#include "../../libs/imageio/include/imageio/ImageDecoder.h"

#include <fstream>
#include <iostream>

extern Engine* gEngine;
extern vzm::VzEngineApp gEngineApp;

using namespace image;
namespace vzm
{
    bool VzTexture::LoadImage(const std::string& fileName, const bool generateMIPs)
    {
        VzTextureRes* tex_res = gEngineApp.GetTextureRes(GetVID());

        if (tex_res->texture) {
            gEngine->destroy(tex_res->texture);
            tex_res->texture = nullptr;
        }

        Path file_name(fileName);
        if (!file_name.exists()) {
            backlog::post("The input image does not exist: " + fileName, backlog::LogLevel::Error);
            return false;
        }

        std::ifstream inputStream(file_name, std::ios::binary);
        image::LinearImage* image = new LinearImage(ImageDecoder::decode(
            inputStream, file_name, ImageDecoder::ColorSpace::SRGB));

        if (!image->isValid()) {
            backlog::post("The input image is invalid:: " + fileName, backlog::LogLevel::Error);
            return false;
        }

        inputStream.close();

        uint32_t channels = image->getChannels();
        uint32_t w = image->getWidth();
        uint32_t h = image->getHeight();
        Texture* texture = Texture::Builder()
            .width(w)
            .height(h)
            .levels(0xff)
            .format(channels == 3 ?
                Texture::InternalFormat::RGB16F : Texture::InternalFormat::RGBA16F)
            .sampler(Texture::Sampler::SAMPLER_2D)
            .build(*gEngine);

        Texture::PixelBufferDescriptor::Callback freeCallback = [](void* buf, size_t, void* data) {
            delete reinterpret_cast<LinearImage*>(data);
            };

        Texture::PixelBufferDescriptor buffer(
            image->getPixelRef(),
            size_t(w * h * channels * sizeof(float)),
            channels == 3 ? Texture::Format::RGB : Texture::Format::RGBA,
            Texture::Type::FLOAT,
            freeCallback,
            image
        );

        texture->setImage(*gEngine, 0, std::move(buffer));
        if (generateMIPs)
        {
            texture->generateMipmaps(*gEngine);
        }

        tex_res->texture = texture;

        tex_res->sampler.setMagFilter(TextureSampler::MagFilter::LINEAR);
        tex_res->sampler.setMinFilter(TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR);
        tex_res->sampler.setWrapModeS(TextureSampler::WrapMode::REPEAT);
        tex_res->sampler.setWrapModeT(TextureSampler::WrapMode::REPEAT);

        UpdateTimeStamp();
        return true;
    }

    void VzTexture::SetMinFilter(const SamplerMinFilter filter)
    {
        VzTextureRes* tex_res = gEngineApp.GetTextureRes(GetVID());
        tex_res->sampler.setMinFilter((TextureSampler::MinFilter)filter);
        UpdateTimeStamp();
    }
    void VzTexture::SetMagFilter(const SamplerMagFilter filter)
    {
        VzTextureRes* tex_res = gEngineApp.GetTextureRes(GetVID());
        tex_res->sampler.setMagFilter((TextureSampler::MagFilter)filter);
        UpdateTimeStamp();
    }
    void VzTexture::SetWrapModeS(const SamplerWrapMode mode)
    {
        VzTextureRes* tex_res = gEngineApp.GetTextureRes(GetVID());
        tex_res->sampler.setWrapModeS((TextureSampler::WrapMode)mode);
        UpdateTimeStamp();
    }
    void VzTexture::SetWrapModeT(const SamplerWrapMode mode)
    {
        VzTextureRes* tex_res = gEngineApp.GetTextureRes(GetVID());
        tex_res->sampler.setWrapModeT((TextureSampler::WrapMode)mode);
        UpdateTimeStamp();
    }

    bool VzTexture::GenerateMIPs()
    {
        VzTextureRes* tex_res = gEngineApp.GetTextureRes(GetVID());

        if (tex_res->texture == nullptr) {
            return false;
        }
        tex_res->texture->generateMipmaps(*gEngine);

        UpdateTimeStamp();
        return true;
    }
}
