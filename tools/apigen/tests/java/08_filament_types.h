#include <filament/Skybox.h>

#include <utils/compiler.h>

namespace filament {
class Skybox;
}

class FilamentTypesTest {
public:
    void aMethodWithAFilamentTypeParameter(filament::Skybox* UTILS_NONNULL skybox) noexcept;
    void aMethodWithAConstFilamentTypeParameter(filament::Skybox const* UTILS_NULLABLE skybox) noexcept;
};
