#include <app/counter.h>

#include <app/print.h> // for print

#include <cstdlib>       // for abort
#include <ostream>       // for ostream
#include <stdexcept>     // for runtime_error
#include <unordered_set> // for unordered_set
#include <utility>       // for swap, pair

static inline constexpr bool counter_enable_unordered_set = true;

auto singleton_constructed_objects() -> std::unordered_set<counter::obj const*>& {
    static std::unordered_set<counter::obj const*> static_data{};
    return static_data;
}

counter::obj::obj()
    : m_data(0)
    , m_counts(nullptr) {
    if constexpr (counter_enable_unordered_set) {
        if (!singleton_constructed_objects().emplace(this).second) {
            test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
            std::abort();
        }
    }
    ++static_default_ctor;
}

counter::obj::obj(const size_t& data, counter& counts)
    : m_data(data)
    , m_counts(&counts) {
    if constexpr (counter_enable_unordered_set) {
        if (!singleton_constructed_objects().emplace(this).second) {
            test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
            std::abort();
        }
    }
    ++m_counts->m_data.m_ctor;
}

counter::obj::obj(const counter::obj& o)
    : m_data(o.m_data)
    , m_counts(o.m_counts) {
    if constexpr (counter_enable_unordered_set) {
        if (1 != singleton_constructed_objects().count(&o)) {
            test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
            std::abort();
        }
        if (!singleton_constructed_objects().emplace(this).second) {
            test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
            std::abort();
        }
    }
    if (nullptr != m_counts) {
        ++m_counts->m_data.m_copy_ctor;
    }
}

counter::obj::obj(counter::obj&& o) noexcept
    : m_data(o.m_data)
    , m_counts(o.m_counts) {
    if constexpr (counter_enable_unordered_set) {
        if (1 != singleton_constructed_objects().count(&o)) {
            test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
            std::abort();
        }
        if (!singleton_constructed_objects().emplace(this).second) {
            test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
            std::abort();
        }
    }
    if (nullptr != m_counts) {
        ++m_counts->m_data.m_move_ctor;
    }
}

counter::obj::~obj() {
    if constexpr (counter_enable_unordered_set) {
        if (1 != singleton_constructed_objects().erase(this)) {
            test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
            std::abort();
        }
    }
    if (nullptr != m_counts) {
        ++m_counts->m_data.m_dtor;
    } else {
        ++static_dtor;
    }
}

auto counter::obj::operator==(obj const& o) const -> bool {
    if constexpr (counter_enable_unordered_set) {
        if (1 != singleton_constructed_objects().count(this) || 1 != singleton_constructed_objects().count(&o)) {
            test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
            std::abort();
        }
    }
    if (nullptr != m_counts) {
        ++m_counts->m_data.m_equals;
    }
    return m_data == o.m_data;
}

auto counter::obj::operator<(obj const& o) const -> bool {
    if constexpr (counter_enable_unordered_set) {
        if (1 != singleton_constructed_objects().count(this) || 1 != singleton_constructed_objects().count(&o)) {
            test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
            std::abort();
        }
    }
    if (nullptr != m_counts) {
        ++m_counts->m_data.m_less;
    }
    return m_data < o.m_data;
}

// NOLINTNEXTLINE(bugprone-unhandled-self-assignment,cert-oop54-cpp)
auto counter::obj::operator=(obj const& o) -> counter::obj& {
    if constexpr (counter_enable_unordered_set) {
        if (1 != singleton_constructed_objects().count(this) || 1 != singleton_constructed_objects().count(&o)) {
            test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
            std::abort();
        }
    }
    m_counts = o.m_counts;
    if (nullptr != m_counts) {
        ++m_counts->m_data.m_assign;
    }
    m_data = o.m_data;
    return *this;
}

auto counter::obj::operator=(obj&& o) noexcept -> counter::obj& {
    if constexpr (counter_enable_unordered_set) {
        if (1 != singleton_constructed_objects().count(this) || 1 != singleton_constructed_objects().count(&o)) {
            test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
            std::abort();
        }
    }
    if (nullptr != o.m_counts) {
        m_counts = o.m_counts;
    }
    m_data = o.m_data;
    if (nullptr != m_counts) {
        ++m_counts->m_data.m_move_assign;
    }
    return *this;
}

auto counter::obj::get() const -> size_t const& {
    if (nullptr != m_counts) {
        ++m_counts->m_data.m_const_get;
    }
    return m_data;
}

auto counter::obj::get() -> size_t& {
    if (nullptr != m_counts) {
        ++m_counts->m_data.m_get;
    }
    return m_data;
}

void counter::obj::swap(obj& other) {
    if constexpr (counter_enable_unordered_set) {
        if (1 != singleton_constructed_objects().count(this) || 1 != singleton_constructed_objects().count(&other)) {
            test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
            std::abort();
        }
    }
    using std::swap;
    swap(m_data, other.m_data);
    swap(m_counts, other.m_counts);
    if (nullptr != m_counts) {
        ++m_counts->m_data.m_swaps;
    }
}

auto counter::obj::get_for_hash() const -> size_t {
    if (nullptr != m_counts) {
        ++m_counts->m_data.m_hash;
    }
    return m_data;
}

counter::counter() {
    counter::static_default_ctor = 0;
    counter::static_dtor = 0;
}

void counter::check_all_done() const {
    if constexpr (counter_enable_unordered_set) {
        // check that all are destructed
        if (!singleton_constructed_objects().empty()) {
            test::print("ERROR at ~counter(): got {} objects still alive!", singleton_constructed_objects().size());
            std::abort();
        }

        if (m_data.m_dtor + static_dtor !=
            m_data.m_ctor + static_default_ctor + m_data.m_copy_ctor + m_data.m_default_ctor + m_data.m_move_ctor) {
            test::print("ERROR at ~counter(): number of counts does not match!\n");
            test::print(
                "{} dtor + {} staticDtor != {} ctor + {} staticDefaultCtor + {} copyCtor + {} defaultCtor + {} moveCtor\n",
                m_data.m_dtor,
                static_dtor,
                m_data.m_ctor,
                static_default_ctor,
                m_data.m_copy_ctor,
                m_data.m_default_ctor,
                m_data.m_move_ctor);
            std::abort();
        }
    }
}

counter::~counter() {
    check_all_done();
}

auto counter::total() const -> size_t {
    return m_data.m_ctor + static_default_ctor + m_data.m_copy_ctor + (m_data.m_dtor + static_dtor) + m_data.m_equals +
           m_data.m_less + m_data.m_assign + m_data.m_swaps + m_data.m_get + m_data.m_const_get + m_data.m_hash +
           m_data.m_move_ctor + m_data.m_move_assign;
}

void counter::operator()(std::string_view title) {
    m_records += fmt::format("{:9}{:9}{:9}{:9}{:9}{:9}{:9}{:9}{:9}{:9}{:9}{:9}{:9}|{:9}| {}\n",
                             m_data.m_ctor,
                             static_default_ctor,
                             m_data.m_copy_ctor,
                             m_data.m_dtor + static_dtor,
                             m_data.m_assign,
                             m_data.m_swaps,
                             m_data.m_get,
                             m_data.m_const_get,
                             m_data.m_hash,
                             m_data.m_equals,
                             m_data.m_less,
                             m_data.m_move_ctor,
                             m_data.m_move_assign,
                             total(),
                             title);
}

auto operator<<(std::ostream& os, counter const& c) -> std::ostream& {
    return os << c.m_records;
}

auto operator new(size_t /*unused*/, counter::obj* /*unused*/) -> void* {
    throw std::runtime_error("operator new overload is taken! Cast to void* to ensure the void pointer overload is taken.");
}
size_t counter::static_default_ctor = 0; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
size_t counter::static_dtor = 0;         // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
