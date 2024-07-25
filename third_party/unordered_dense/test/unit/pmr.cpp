#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

#include <fmt/core.h>

#include <cstddef>     // for size_t
#include <cstdint>     // for uint64_t
#include <stdexcept>   // thrown in no_null_memory_resource
#include <string_view> // for string_view
#include <utility>     // for move
#include <vector>      // for vector

#if defined(ANKERL_UNORDERED_DENSE_PMR)

// windows' vector has different allocation behavior, macos has linker errors
#    if __linux__

class logging_memory_resource : public ANKERL_UNORDERED_DENSE_PMR::memory_resource {
    auto do_allocate(std::size_t bytes, std::size_t alignment) -> void* override {
        fmt::print("+ {} bytes, {} alignment\n", bytes, alignment);
        return ANKERL_UNORDERED_DENSE_PMR::new_delete_resource()->allocate(bytes, alignment);
    }

    void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override {
        fmt::print("- {} bytes, {} alignment, {} ptr\n", bytes, alignment, p);
        return ANKERL_UNORDERED_DENSE_PMR::new_delete_resource()->deallocate(p, bytes, alignment);
    }

    [[nodiscard]] auto do_is_equal(const ANKERL_UNORDERED_DENSE_PMR::memory_resource& other) const noexcept -> bool override {
        return this == &other;
    }
};

class no_null_memory_resource : public ANKERL_UNORDERED_DENSE_PMR::memory_resource {
    auto do_allocate(std::size_t bytes, std::size_t alignment) -> void* override {
        if (bytes == 0) {
            throw std::runtime_error("no_null_memory_resource::do_allocate should not do_allocate 0");
        }
        return ANKERL_UNORDERED_DENSE_PMR::new_delete_resource()->allocate(bytes, alignment);
    }

    void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override {
        if (nullptr == p || 0U == bytes || 0U == alignment) {
            throw std::runtime_error("no_null_memory_resource::do_deallocate should not deallocate with any 0 value");
        }
        return ANKERL_UNORDERED_DENSE_PMR::new_delete_resource()->deallocate(p, bytes, alignment);
    }

    [[nodiscard]] auto do_is_equal(const ANKERL_UNORDERED_DENSE_PMR::memory_resource& other) const noexcept -> bool override {
        return this == &other;
    }
};

class track_peak_memory_resource : public ANKERL_UNORDERED_DENSE_PMR::memory_resource {
    uint64_t m_peak = 0;
    uint64_t m_current = 0;
    uint64_t m_num_allocs = 0;
    uint64_t m_num_deallocs = 0;
    mutable uint64_t m_num_is_equals = 0;

    auto do_allocate(std::size_t bytes, std::size_t alignment) -> void* override {
        ++m_num_allocs;
        m_current += bytes;
        if (m_current > m_peak) {
            m_peak = m_current;
        }
        return ANKERL_UNORDERED_DENSE_PMR::new_delete_resource()->allocate(bytes, alignment);
    }

    void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override {
        if (p == nullptr) {
            return;
        }
        ++m_num_deallocs;
        m_current -= bytes;
        return ANKERL_UNORDERED_DENSE_PMR::new_delete_resource()->deallocate(p, bytes, alignment);
    }

    [[nodiscard]] auto do_is_equal(const ANKERL_UNORDERED_DENSE_PMR::memory_resource& other) const noexcept -> bool override {
        ++m_num_is_equals;
        return this == &other;
    }

public:
    [[nodiscard]] auto current() const -> uint64_t {
        return m_current;
    }

    [[nodiscard]] auto peak() const -> uint64_t {
        return m_peak;
    }

    [[nodiscard]] auto num_allocs() const -> uint64_t {
        return m_num_allocs;
    }

    [[nodiscard]] auto num_deallocs() const -> uint64_t {
        return m_num_deallocs;
    }

    [[nodiscard]] auto num_is_equals() const -> uint64_t {
        return m_num_is_equals;
    }
};

TYPE_TO_STRING_PMR_MAP(uint64_t, uint64_t);

TEST_CASE_PMR_MAP("pmr", uint64_t, uint64_t) {
    auto mr = track_peak_memory_resource();
    {
        REQUIRE(mr.current() == 0);
        auto map = map_t(&mr);

        for (size_t i = 0; i < 1; ++i) {
            map[i] = i;
        }
        REQUIRE(mr.current() != 0);

        // gets a copy, but it has the same memory resource
        auto alloc = map.get_allocator();
        REQUIRE(alloc.resource() == &mr);
    }
    REQUIRE(mr.current() == 0);
}

TEST_CASE_PMR_MAP("pmr_no_null", uint64_t, uint64_t) {
    auto mr = no_null_memory_resource();
    { auto map = map_t(&mr); }
    {
        auto map = map_t(&mr);
        for (size_t i = 0; i < 1; ++i) {
            map[i] = i;
        }
    }

    {
        auto map = map_t(&mr);
        for (size_t i = 0; i < 1; ++i) {
            map[i] = i;
        }
        auto map2 = std::move(map);
    }
}

void show([[maybe_unused]] track_peak_memory_resource const& mr, [[maybe_unused]] std::string_view name) {
    // fmt::print("{}: {} allocs, {} deallocs, {} is_equals\n", name, mr.num_allocs(), mr.num_deallocs(), mr.num_is_equals());
}

// the following tests are vector specific, so don't use segmented_vector

TEST_CASE("pmr_copy") {
    using map_t = ankerl::unordered_dense::pmr::map<uint64_t, uint64_t>;
    auto mr1 = track_peak_memory_resource();
    auto map1 = map_t(&mr1);
    map1[1] = 2;

    auto mr2 = track_peak_memory_resource();
    auto map2 = map_t(&mr2);
    map2[3] = 4;
    show(mr1, "mr1");
    show(mr2, "mr2");

    map1 = map2;
    REQUIRE(map1.size() == 1);
    REQUIRE(map1.find(3) != map1.end());
    show(mr1, "mr1");
    show(mr2, "mr2");

    REQUIRE(mr1.num_allocs() == 3);
    REQUIRE(mr1.num_deallocs() == 1);
    REQUIRE(mr1.num_is_equals() == 0);

    REQUIRE(mr2.num_allocs() == 2);
    REQUIRE(mr2.num_deallocs() == 0);
    REQUIRE(mr2.num_is_equals() == 0);
}

TEST_CASE("pmr_move_different_mr") {
    using map_t = ankerl::unordered_dense::pmr::map<uint64_t, uint64_t>;

    auto mr1 = track_peak_memory_resource();
    auto map1 = map_t(&mr1);
    map1[1] = 2;

    auto mr2 = track_peak_memory_resource();
    auto map2 = map_t(&mr2);
    map2[3] = 4;
    show(mr1, "mr1");
    show(mr2, "mr2");

    map1 = std::move(map2);
    REQUIRE(map1.size() == 1);
    REQUIRE(map1.find(3) != map1.end());
    show(mr1, "mr1");
    show(mr2, "mr2");

    REQUIRE(mr1.num_allocs() == 3);
    REQUIRE(mr1.num_deallocs() == 1);
    REQUIRE(mr1.num_is_equals() == 1);

    REQUIRE(mr2.num_allocs() == 2);
    REQUIRE(mr2.num_deallocs() == 0);
    REQUIRE(mr2.num_is_equals() == 1);
}

TEST_CASE("pmr_move_same_mr") {
    using map_t = ankerl::unordered_dense::pmr::map<uint64_t, uint64_t>;

    auto mr1 = track_peak_memory_resource();
    auto map1 = map_t(&mr1);
    map1[1] = 2;
    REQUIRE(mr1.num_allocs() == 2);
    REQUIRE(mr1.num_deallocs() == 0);
    REQUIRE(mr1.num_is_equals() == 0);

    auto map2 = ankerl::unordered_dense::pmr::map<uint64_t, uint64_t>(&mr1);
    map2[3] = 4;
    show(mr1, "mr1");

    map1 = std::move(map2);
    REQUIRE(map1.size() == 1);
    REQUIRE(map1.find(3) != map1.end());
    show(mr1, "mr1");

    REQUIRE(mr1.num_allocs() == 5); // 5 because of the initial allocation
    REQUIRE(mr1.num_deallocs() == 2);
    REQUIRE(mr1.num_is_equals() == 0);
}

#    endif
#endif