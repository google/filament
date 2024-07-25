#include <ankerl/unordered_dense.h>
#include <app/counter.h>
#include <app/counting_allocator.h>

#include <app/doctest.h>

#include <cstddef>
#include <third-party/nanobench.h>

#include <deque>

TEST_CASE("segmented_vector") {
    counter counts;
    INFO(counts);
    {
        auto vec = ankerl::unordered_dense::segmented_vector<counter::obj>();
        for (size_t i = 0; i < 1000; ++i) {
            vec.emplace_back(i, counts);
            REQUIRE(i + 1 == counts.ctor());
        }
        REQUIRE(0 == counts.move_ctor());
        REQUIRE(0 == counts.move_assign());
        counts("before dtor");
        REQUIRE(counts.data() == counter::data_t{1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
    }
    counts.check_all_done();
    REQUIRE(0 == counts.move_ctor());
    counts("done");
    REQUIRE(counts.data() == counter::data_t{1000, 0, 0, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0});
}

TEST_CASE("segmented_vector_capacity") {
    counter counts;
    INFO(counts);
    auto vec =
        ankerl::unordered_dense::segmented_vector<counter::obj, std::allocator<counter::obj>, sizeof(counter::obj) * 4>();
    REQUIRE(0 == vec.capacity());
    for (size_t i = 0; i < 50; ++i) {
        REQUIRE(i == vec.size());
        vec.emplace_back(i, counts);
        REQUIRE(i + 1 == vec.size());
        REQUIRE(vec.capacity() >= vec.size());
        REQUIRE(0 == vec.capacity() % 4);
    }
}

TEST_CASE("segmented_vector_idx") {
    counter counts;
    INFO(counts);
    auto vec =
        ankerl::unordered_dense::segmented_vector<counter::obj, std::allocator<counter::obj>, sizeof(counter::obj) * 4>();
    REQUIRE(0 == vec.capacity());
    for (size_t i = 0; i < 50; ++i) {
        vec.emplace_back(i, counts);
    }

    for (size_t i = 0; i < vec.size(); ++i) {
        REQUIRE(i == vec[i].get());
    }
}

TEST_CASE("segmented_vector_iterate") {
    counter counts;
    INFO(counts);
    auto vec =
        ankerl::unordered_dense::segmented_vector<counter::obj, std::allocator<counter::obj>, sizeof(counter::obj) * 4>();
    for (size_t i = 0; i < 50; ++i) {
        auto it = vec.begin();
        auto end = vec.end();

        REQUIRE(std::distance(it, end) == static_cast<std::ptrdiff_t>(vec.size()));
        size_t j = 0;
        while (it != end) {
            REQUIRE(it->get() == j);
            ++it;
            ++j;
        }
        vec.emplace_back(i, counts);
    }
}

TEST_CASE("segmented_vector_reserve") {
    auto counts = counts_for_allocator{};
    auto vec = ankerl::unordered_dense::segmented_vector<int, counting_allocator<int>, sizeof(int) * 16>(&counts);

    REQUIRE(0 == vec.capacity());
    REQUIRE(counts.size() < 2);
    vec.reserve(1100);
    REQUIRE(counts.size() > 63);
    counts.reset();
    REQUIRE(counts.size() == 0);
    REQUIRE(1104 == vec.capacity());

    for (size_t i = 0; i < vec.capacity(); ++i) {
        vec.emplace_back(0);
    }
    REQUIRE(counts.size() == 0);
    vec.emplace_back(123);

    // 3: 2 for std::vector<T*> reallocates, 1 for the new segment
    REQUIRE(counts.size() == 3);
}

using vec_t = ankerl::unordered_dense::segmented_vector<counter::obj>;
static_assert(sizeof(vec_t) == sizeof(std::vector<counter::obj*>) + sizeof(size_t));

TEST_CASE("bench_segmented_vector" * doctest::test_suite("bench") * doctest::skip()) {
    static constexpr auto num_elements = size_t{21233};

    using namespace std::literals;

    ankerl::nanobench::Rng rng(123);

    auto sv = ankerl::unordered_dense::segmented_vector<size_t>();
    for (size_t i = 0; i < num_elements; ++i) {
        sv.emplace_back(i);
    }

    ankerl::nanobench::Bench().minEpochTime(100ms).batch(sv.size()).run("shuffle stable_vector", [&] {
        rng.shuffle(sv);
    });

    auto c = std::deque<size_t>();
    for (size_t i = 0; i < num_elements; ++i) {
        c.push_back(i);
    }
    ankerl::nanobench::Bench().minEpochTime(100ms).batch(sv.size()).run("shuffle std::deque", [&] {
        rng.shuffle(c);
    });

    auto v = std::vector<size_t>();
    for (size_t i = 0; i < num_elements; ++i) {
        v.push_back(i);
    }
    ankerl::nanobench::Bench().minEpochTime(100ms).batch(sv.size()).run("shuffle std::vector", [&] {
        rng.shuffle(v);
    });
}