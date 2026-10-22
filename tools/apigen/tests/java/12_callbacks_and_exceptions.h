#include <utils/compiler.h>
#include <utils/Entity.h>
#include <utils/Invocable.h>

#include <stdint.h>

class CallbacksAndExceptionsTest {
public:
    // 1. void return, noexcept
    void forEach(utils::Invocable<void(utils::Entity entity)>&& functor) const noexcept;

    // 2. void return, non-noexcept (throwing)
    void forEachThrowing(utils::Invocable<void(utils::Entity entity)>&& functor);

    // 3. bool return, noexcept
    bool findFirst(utils::Invocable<bool(utils::Entity entity)>&& predicate) const noexcept;

    // 4. bool return, non-noexcept (throwing)
    bool findFirstThrowing(utils::Invocable<bool(utils::Entity entity)>&& predicate);

    // 5. Multi-param callback, noexcept
    void processPair(utils::Invocable<void(int32_t a, float b)>&& callback) noexcept;

    // 6. Multi-param callback, non-noexcept (throwing)
    void processPairThrowing(utils::Invocable<void(int32_t a, float b)>&& callback);
};
