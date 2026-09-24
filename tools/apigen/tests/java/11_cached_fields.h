#include <filament/IndirectLight.h>
#include <filament/Skybox.h>

#include <utils/compiler.h>

namespace filament {
class Skybox;
class IndirectLight;
}

class CachedFieldsTest {
public:
    void setSkybox(filament::Skybox* UTILS_NULLABLE skybox) noexcept;
    filament::Skybox* UTILS_NULLABLE getSkybox() const noexcept;

    void setIndirectLight(filament::IndirectLight* UTILS_NULLABLE ibl) noexcept;
    filament::IndirectLight* UTILS_NULLABLE getIndirectLight() const noexcept;
};
