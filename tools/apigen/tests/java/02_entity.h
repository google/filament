#include <utils/Entity.h>
#include <utils/EntityInstance.h>

class Foo {
public:
    using Instance = utils::EntityInstance<Foo>;

    void setEntity(utils::Entity e) noexcept;
    utils::Entity getEntity() const noexcept;

    void setEntityInstanceA(utils::EntityInstance<Foo, false> i) noexcept;
    utils::EntityInstance<Foo, false> getEntityInstanceA() const noexcept;

    void setEntityInstanceB(utils::EntityInstance<Foo, true> i) noexcept;
    utils::EntityInstance<Foo, true> getEntityInstanceB() const noexcept;

    void setEntityInstanceAlias(Instance i) noexcept;
    Instance getEntityInstanceAlias() const noexcept;
};
