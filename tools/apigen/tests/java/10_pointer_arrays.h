#include <utils/compiler.h>
#include <utils/Entity.h>
#include <utils/Slice.h>

#include <stddef.h>
#include <stdint.h>

class PointerArraysTest {
public:
    void addEntities(utils::Slice<const utils::Entity> entities) noexcept;
    void removeEntities(utils::Slice<const utils::Entity> entities) noexcept;
    void updateEntities(utils::Slice<utils::Entity> entities) noexcept;
    void setIndices(utils::Slice<const uint32_t> indices) noexcept;
    void setFloats(utils::Slice<const float> values) noexcept;
};
