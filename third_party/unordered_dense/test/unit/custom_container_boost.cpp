#if ANKERL_UNORDERED_DENSE_HAS_BOOST

#    if __clang__
#        pragma clang diagnostic push
#        pragma clang diagnostic ignored "-Wold-style-cast"
#    endif
#    include <ankerl/unordered_dense.h>

#    include <app/doctest.h>

#    include <boost/container/vector.hpp>
#    include <boost/interprocess/allocators/allocator.hpp>
#    include <boost/interprocess/allocators/node_allocator.hpp>
#    include <boost/interprocess/containers/vector.hpp>
#    include <boost/interprocess/managed_shared_memory.hpp>

#    include <deque>

// Alias an STL-like allocator of ints that allocates ints from the segment
using shmem_allocator =
    boost::interprocess::allocator<std::pair<int, std::string>, boost::interprocess::managed_shared_memory::segment_manager>;
using shmem_vector = boost::interprocess::vector<std::pair<int, std::string>, shmem_allocator>;

// Remove shared memory on construction and destruction
struct shm_remove {
    shm_remove() {
        boost::interprocess::shared_memory_object::remove("MySharedMemory");
    }
    ~shm_remove() {
        boost::interprocess::shared_memory_object::remove("MySharedMemory");
    }

    shm_remove(shm_remove const&) = delete;
    shm_remove(shm_remove&&) = delete;
    auto operator=(shm_remove const&) -> shm_remove = delete;
    auto operator=(shm_remove&&) -> shm_remove = delete;
};

TYPE_TO_STRING_MAP(int, std::string, ankerl::unordered_dense::hash<int>, std::equal_to<int>, shmem_vector);

// See https://www.boost.org/doc/libs/1_80_0/doc/html/interprocess/allocators_containers.html
TEST_CASE_TEMPLATE(
    "boost_container_vector",
    map_t,
    ankerl::unordered_dense::map<int, std::string, ankerl::unordered_dense::hash<int>, std::equal_to<int>, shmem_vector>,
    ankerl::unordered_dense::
        segmented_map<int, std::string, ankerl::unordered_dense::hash<int>, std::equal_to<int>, shmem_allocator>) {

    auto remover = shm_remove();

    // Create shared memory
    auto segment = boost::interprocess::managed_shared_memory(boost::interprocess::create_only, "MySharedMemory", 1024 * 1024);
    auto map = map_t{shmem_allocator{segment.get_segment_manager()}};

    int total = 10000;

    for (int i = 0; i < total; ++i) {
        map.try_emplace(i, std::to_string(i));
    }

    REQUIRE(map.size() == static_cast<size_t>(total));
    for (int i = 0; i < total; ++i) {
        auto it = map.find(i);
        REQUIRE(it != map.end());
        REQUIRE(it->first == i);
        REQUIRE(it->second == std::to_string(i));
    }

    map.erase(total + 123);
    REQUIRE(map.size() == static_cast<size_t>(total));
    map.erase(29);
    REQUIRE(map.size() == static_cast<size_t>(total - 1));

    map.emplace(std::pair<int, std::string>(9999999, "hello"));
    REQUIRE(map.size() == static_cast<size_t>(total));
}

#endif // ANKERL_UNORDERED_DENSE_HAS_BOOST
