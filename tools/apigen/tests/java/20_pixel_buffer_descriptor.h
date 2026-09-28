#include <utils/compiler.h>

#include <stddef.h>
#include <stdint.h>

namespace filament {

class Engine;
class RenderTarget;

namespace backend {
class PixelBufferDescriptor;
} // namespace backend

class PixelBufferDescriptorTest {
public:
    void readPixels(uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
            backend::PixelBufferDescriptor&& buffer);

    void readPixels(RenderTarget* UTILS_NONNULL renderTarget,
            uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
            backend::PixelBufferDescriptor&& buffer);

    void setImage(Engine& engine, uint32_t level,
            uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
            backend::PixelBufferDescriptor&& buffer);
};

} // namespace filament
