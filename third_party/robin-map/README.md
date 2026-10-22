[![CI](https://github.com/Tessil/robin-map/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/Tessil/robin-map/actions/workflows/ci.yml)

## A C++ implementation of a fast hash map and hash set using robin hood hashing

The robin-map library is a C++ implementation of a fast hash map and hash set using open-addressing and linear robin hood hashing with backward shift deletion to resolve collisions.

Four classes are provided: `tsl::robin_map`, `tsl::robin_set`, `tsl::robin_pg_map` and `tsl::robin_pg_set`. The first two are faster and use a power of two growth policy, the last two use a prime growth policy instead and are able to cope better with a poor hash function. Use the prime version if there is a chance of repeating patterns in the lower bits of your hash (e.g. you are storing pointers with an identity hash function). See [GrowthPolicy](#growth-policy) for details.

A **benchmark** of `tsl::robin_map` against other hash maps may be found [here](https://tessil.github.io/2016/08/29/benchmark-hopscotch-map.html). This page also gives some advices on which hash table structure you should try for your use case (useful if you are a bit lost with the multiple hash tables implementations in the `tsl` namespace).

### Key features

- Header-only library, just add the [include](include/) directory to your include path and you are ready to go. If you use CMake, you can also use the `tsl::robin_map` exported target from the [CMakeLists.txt](CMakeLists.txt).
- Fast hash table, check the [benchmark](https://tessil.github.io/2016/08/29/benchmark-hopscotch-map.html) for some numbers.
- Support for move-only and non-default constructible key/value.
- Support for heterogeneous lookups allowing the usage of `find` with a type different than `Key` (e.g. if you have a map that uses `std::unique_ptr<foo>` as key, you can use a `foo*` or a `std::uintptr_t` as key parameter to `find` without constructing a `std::unique_ptr<foo>`, see [example](#heterogeneous-lookups)).
- No need to reserve any sentinel value from the keys.
- Possibility to store the hash value alongside the stored key-value for faster rehash and lookup if the hash or the key equal functions are expensive to compute. Note that hash may be stored even if not asked explicitly when the library can detect that it will have no impact on the size of the structure in memory due to alignment. See the [StoreHash](https://tessil.github.io/robin-map/classtsl_1_1robin__map.html#details) template parameter for details.
- If the hash is known before a lookup, it is possible to pass it as parameter to speed-up the lookup (see `precalculated_hash` parameter in [API](https://tessil.github.io/robin-map/classtsl_1_1robin__map.html#a35021b11aabb61820236692a54b3a0f8)).
- Support for efficient serialization and deserialization (see [example](#serialization) and the `serialize/deserialize` methods in the [API](https://tessil.github.io/robin-map/classtsl_1_1robin__map.html) for details).
- The library can be used with exceptions disabled (through `-fno-exceptions` option on Clang and GCC, without an `/EH` option on MSVC or simply by defining `TSL_NO_EXCEPTIONS`). `std::terminate` is used in replacement of the `throw` instruction when exceptions are disabled.
- API closely similar to `std::unordered_map` and `std::unordered_set`.

### Differences compared to `std::unordered_map`

`tsl::robin_map` tries to have an interface similar to `std::unordered_map`, but some differences exist.
- The **strong exception guarantee only holds** if the following statement is true `std::is_nothrow_swappable<value_type>::value && std::is_nothrow_move_constructible<value_type>::value` (where `value_type` is `Key` for `tsl::robin_set` and `std::pair<Key, T>` for `tsl::robin_map`). Otherwise if an exception is thrown during the swap or the move, the structure may end up in a undefined state. Note that per the standard, a `value_type` with a noexcept copy constructor and no move constructor also satisfies this condition and will thus guarantee the strong exception guarantee for the structure (see [API](https://tessil.github.io/robin-map/classtsl_1_1robin__map.html#details) for details).
- The type `Key`, and also `T` in case of map, must be swappable. They must also be copy and/or move constructible.
- Iterator invalidation doesn't behave in the same way, any operation modifying the hash table invalidate them (see [API](https://tessil.github.io/robin-map/classtsl_1_1robin__map.html#details) for details).
- References and pointers to keys or values in the map are invalidated in the same way as iterators to these keys-values.
- For iterators of `tsl::robin_map`, `operator*()` and `operator->()` return a reference and a pointer to `const std::pair<Key, T>` instead of `std::pair<const Key, T>` making the value `T` not modifiable. To modify the value you have to call the `value()` method of the iterator to get a mutable reference. Example:
```c++
tsl::robin_map<int, int> map = {{1, 1}, {2, 1}, {3, 1}};
for(auto it = map.begin(); it != map.end(); ++it) {
    //it->second = 2; // Illegal
    it.value() = 2; // Ok
}
```
- No support for some buckets related methods (like `bucket_size`, `bucket`, ...).

These differences also apply between `std::unordered_set` and `tsl::robin_set`.

Thread-safety guarantees are the same as `std::unordered_map/set` (i.e. possible to have multiple readers with no writer).

### Growth policy

The library supports multiple growth policies through the `GrowthPolicy` template parameter. Three policies are provided by the library but you can easily implement your own if needed.

* **[tsl::rh::power_of_two_growth_policy.](https://tessil.github.io/robin-map/classtsl_1_1rh_1_1power__of__two__growth__policy.html)** Default policy used by `tsl::robin_map/set`. This policy keeps the size of the bucket array of the hash table to a power of two. This constraint allows the policy to avoid the usage of the slow modulo operation to map a hash to a bucket, instead of <code>hash % 2<sup>n</sup></code>, it uses <code>hash & (2<sup>n</sup> - 1)</code> (see [fast modulo](https://en.wikipedia.org/wiki/Modulo_operation#Performance_issues)). Fast but this may cause a lot of collisions with a poor hash function as the modulo with a power of two only masks the most significant bits in the end.
* **[tsl::rh::prime_growth_policy.](https://tessil.github.io/robin-map/classtsl_1_1rh_1_1prime__growth__policy.html)** Default policy used by `tsl::robin_pg_map/set`. The policy keeps the size of the bucket array of the hash table to a prime number. When mapping a hash to a bucket, using a prime number as modulo will result in a better distribution of the hash across the buckets even with a poor hash function. To allow the compiler to optimize the modulo operation, the policy use a lookup table with constant primes modulos (see [API](https://tessil.github.io/robin-map/classtsl_1_1rh_1_1prime__growth__policy.html#details) for details). Slower than `tsl::rh::power_of_two_growth_policy` but more secure.
* **[tsl::rh::mod_growth_policy.](https://tessil.github.io/robin-map/classtsl_1_1rh_1_1mod__growth__policy.html)** The policy grows the map by a customizable growth factor passed in parameter. It then just use the modulo operator to map a hash to a bucket. Slower but more flexible.


To implement your own policy, you have to implement the following interface.

```c++
struct custom_policy {
    // Called on hash table construction and rehash, min_bucket_count_in_out is the minimum buckets
    // that the hash table needs. The policy can change it to a higher number of buckets if needed 
    // and the hash table will use this value as bucket count. If 0 bucket is asked, then the value
    // must stay at 0.
    explicit custom_policy(std::size_t& min_bucket_count_in_out);
    
    // Return the bucket [0, bucket_count()) to which the hash belongs. 
    // If bucket_count() is 0, it must always return 0.
    std::size_t bucket_for_hash(std::size_t hash) const noexcept;
    
    // Return the number of buckets that should be used on next growth
    std::size_t next_bucket_count() const;
    
    // Maximum number of buckets supported by the policy
    std::size_t max_bucket_count() const;
    
    // Reset the growth policy as if the policy was created with a bucket count of 0.
    // After a clear, the policy must always return 0 when bucket_for_hash() is called.
    void clear() noexcept;
}
```

### Installation

To use robin-map, just add the [include](include/) directory to your include path. It is a **header-only** library.

If you use CMake, you can also use the `tsl::robin_map` exported target from the [CMakeLists.txt](CMakeLists.txt) with `target_link_libraries`. 
```cmake
# Example where the robin-map project is stored in a third-party directory
add_subdirectory(third-party/robin-map)
target_link_libraries(your_target PRIVATE tsl::robin_map)  
```

If the project has been installed through `make install`, you can also use `find_package(tsl-robin-map REQUIRED)` instead of `add_subdirectory`.

The library is available in [vcpkg](https://github.com/Microsoft/vcpkg/tree/master/ports/robin-map) and [conan](https://conan.io/center/tsl-robin-map). It's also present in [Debian](https://packages.debian.org/buster/robin-map-dev), [Ubuntu](https://packages.ubuntu.com/disco/robin-map-dev) and [Fedora](https://apps.fedoraproject.org/packages/robin-map-devel) package repositories.

The code should work with any C++17 standard-compliant compiler.

To run the tests you will need the Boost Test library and CMake.

```bash
git clone https://github.com/Tessil/robin-map.git
cd robin-map/tests
mkdir build
cd build
cmake ..
cmake --build .
./tsl_robin_map_tests
```

### Usage

The API can be found [here](https://tessil.github.io/robin-map/). 

All methods are not documented yet, but they replicate the behavior of the ones in `std::unordered_map` and `std::unordered_set`, except if specified otherwise.


### Example

```c++
#include <cstdint>
#include <iostream>
#include <string>
#include <tsl/robin_map.h>
#include <tsl/robin_set.h>

int main() {
    tsl::robin_map<std::string, int> map = {{"a", 1}, {"b", 2}};
    map["c"] = 3;
    map["d"] = 4;
    
    map.insert({"e", 5});
    map.erase("b");
    
    for(auto it = map.begin(); it != map.end(); ++it) {
        //it->second += 2; // Not valid.
        it.value() += 2;
    }
    
    // {d, 6} {a, 3} {e, 7} {c, 5}
    for(const auto& key_value : map) {
        std::cout << "{" << key_value.first << ", " << key_value.second << "}" << std::endl;
    }
    
        
    if(map.find("a") != map.end()) {
        std::cout << "Found \"a\"." << std::endl;
    }
    
    const std::size_t precalculated_hash = std::hash<std::string>()("a");
    // If we already know the hash beforehand, we can pass it in parameter to speed-up lookups.
    if(map.find("a", precalculated_hash) != map.end()) {
        std::cout << "Found \"a\" with hash " << precalculated_hash << "." << std::endl;
    }
    
    
    /*
     * Calculating the hash and comparing two std::string may be slow. 
     * We can store the hash of each std::string in the hash map to make 
     * the inserts and lookups faster by setting StoreHash to true.
     */ 
    tsl::robin_map<std::string, int, std::hash<std::string>, 
                   std::equal_to<std::string>,
                   std::allocator<std::pair<std::string, int>>,
                   true> map2;
                       
    map2["a"] = 1;
    map2["b"] = 2;
    
    // {a, 1} {b, 2}
    for(const auto& key_value : map2) {
        std::cout << "{" << key_value.first << ", " << key_value.second << "}" << std::endl;
    }
    
    
    
    
    tsl::robin_set<int> set;
    set.insert({1, 9, 0});
    set.insert({2, -1, 9});
    
    // {0} {1} {2} {9} {-1}
    for(const auto& key : set) {
        std::cout << "{" << key << "}" << std::endl;
    }
}  
```

#### Heterogeneous lookups

Heterogeneous overloads allow the usage of other types than `Key` for lookup and erase operations as long as the used types are hashable and comparable to `Key`.

To activate the heterogeneous overloads in `tsl::robin_map/set`, the qualified-id `KeyEqual::is_transparent` must be valid. It works the same way as for [`std::map::find`](http://en.cppreference.com/w/cpp/container/map/find). You can either use [`std::equal_to<>`](http://en.cppreference.com/w/cpp/utility/functional/equal_to_void) or define your own function object.

Both `KeyEqual` and `Hash` will need to be able to deal with the different types.

```c++
#include <functional>
#include <iostream>
#include <string>
#include <tsl/robin_map.h>


struct employee {
    employee(int id, std::string name) : m_id(id), m_name(std::move(name)) {
    }
    
    // Either we include the comparators in the class and we use `std::equal_to<>`...
    friend bool operator==(const employee& empl, int empl_id) {
        return empl.m_id == empl_id;
    }
    
    friend bool operator==(int empl_id, const employee& empl) {
        return empl_id == empl.m_id;
    }
    
    friend bool operator==(const employee& empl1, const employee& empl2) {
        return empl1.m_id == empl2.m_id;
    }
    
    
    int m_id;
    std::string m_name;
};

// ... or we implement a separate class to compare employees.
struct equal_employee {
    using is_transparent = void;
    
    bool operator()(const employee& empl, int empl_id) const {
        return empl.m_id == empl_id;
    }
    
    bool operator()(int empl_id, const employee& empl) const {
        return empl_id == empl.m_id;
    }
    
    bool operator()(const employee& empl1, const employee& empl2) const {
        return empl1.m_id == empl2.m_id;
    }
};

struct hash_employee {
    std::size_t operator()(const employee& empl) const {
        return std::hash<int>()(empl.m_id);
    }
    
    std::size_t operator()(int id) const {
        return std::hash<int>()(id);
    }
};


int main() {
    // Use std::equal_to<> which will automatically deduce and forward the parameters
    tsl::robin_map<employee, int, hash_employee, std::equal_to<>> map; 
    map.insert({employee(1, "John Doe"), 2001});
    map.insert({employee(2, "Jane Doe"), 2002});
    map.insert({employee(3, "John Smith"), 2003});

    // John Smith 2003
    auto it = map.find(3);
    if(it != map.end()) {
        std::cout << it->first.m_name << " " << it->second << std::endl;
    }

    map.erase(1);



    // Use a custom KeyEqual which has an is_transparent member type
    tsl::robin_map<employee, int, hash_employee, equal_employee> map2;
    map2.insert({employee(4, "Johnny Doe"), 2004});

    // 2004
    std::cout << map2.at(4) << std::endl;
}  
```

#### Serialization

The library provides an efficient way to serialize and deserialize a map or a set so that it can be saved to a file or send through the network.
To do so, it requires the user to provide a function object for both serialization and deserialization.

```c++
struct serializer {
    // Must support the following types for U: std::int16_t, std::uint32_t, 
    // std::uint64_t, float and std::pair<Key, T> if a map is used or Key for 
    // a set.
    template<typename U>
    void operator()(const U& value);
};
```

```c++
struct deserializer {
    // Must support the following types for U: std::int16_t, std::uint32_t, 
    // std::uint64_t, float and std::pair<Key, T> if a map is used or Key for 
    // a set.
    template<typename U>
    U operator()();
};
```

Note that the implementation leaves binary compatibility (endianness, float binary representation, size of int, ...) of the types it serializes/deserializes in the hands of the provided function objects if compatibility is required.

More details regarding the `serialize` and `deserialize` methods can be found in the [API](https://tessil.github.io/robin-map/classtsl_1_1robin__map.html).

```c++
#include <cassert>
#include <cstdint>
#include <fstream>
#include <type_traits>
#include <tsl/robin_map.h>


class serializer {
public:
    serializer(const char* file_name) {
        m_ostream.exceptions(m_ostream.badbit | m_ostream.failbit);
        m_ostream.open(file_name, std::ios::binary);
    }
    
    template<class T,
             typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
    void operator()(const T& value) {
        m_ostream.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }
    
    void operator()(const std::pair<std::int64_t, std::int64_t>& value) {
        (*this)(value.first);
        (*this)(value.second);
    }

private:
    std::ofstream m_ostream;
};

class deserializer {
public:
    deserializer(const char* file_name) {
        m_istream.exceptions(m_istream.badbit | m_istream.failbit | m_istream.eofbit);
        m_istream.open(file_name, std::ios::binary);
    }
    
    template<class T>
    T operator()() {
        T value;
        deserialize(value);
        
        return value;
    }
    
private:
    template<class T,
             typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
    void deserialize(T& value) {
        m_istream.read(reinterpret_cast<char*>(&value), sizeof(T));
    }
    
    void deserialize(std::pair<std::int64_t, std::int64_t>& value) {
        deserialize(value.first);
        deserialize(value.second);
    }

private:
    std::ifstream m_istream;
};


int main() {
    const tsl::robin_map<std::int64_t, std::int64_t> map = {{1, -1}, {2, -2}, {3, -3}, {4, -4}};
    
    
    const char* file_name = "robin_map.data";
    {
        serializer serial(file_name);
        map.serialize(serial);
    }
    
    {
        deserializer dserial(file_name);
        auto map_deserialized = tsl::robin_map<std::int64_t, std::int64_t>::deserialize(dserial);
        
        assert(map == map_deserialized);
    }
    
    {
        deserializer dserial(file_name);
        
        /**
         * If the serialized and deserialized map are hash compatibles (see conditions in API), 
         * setting the argument to true speed-up the deserialization process as we don't have 
         * to recalculate the hash of each key. We also know how much space each bucket needs.
         */
        const bool hash_compatible = true;
        auto map_deserialized = 
            tsl::robin_map<std::int64_t, std::int64_t>::deserialize(dserial, hash_compatible);
        
        assert(map == map_deserialized);
    }
} 
```

##### Serialization with Boost Serialization and compression with zlib

It is possible to use a serialization library to avoid the boilerplate. 

The following example uses Boost Serialization with the Boost zlib compression stream to reduce the size of the resulting serialized file. The example requires C++20 due to the usage of the template parameter list syntax in lambdas, but it can be adapted to less recent versions.

```c++
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/utility.hpp>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <tsl/robin_map.h>


namespace boost { namespace serialization {
    template<class Archive, class Key, class T>
    void serialize(Archive & ar, tsl::robin_map<Key, T>& map, const unsigned int version) {
        split_free(ar, map, version); 
    }

    template<class Archive, class Key, class T>
    void save(Archive & ar, const tsl::robin_map<Key, T>& map, const unsigned int /*version*/) {
        auto serializer = [&ar](const auto& v) { ar & v; };
        map.serialize(serializer);
    }

    template<class Archive, class Key, class T>
    void load(Archive & ar, tsl::robin_map<Key, T>& map, const unsigned int /*version*/) {
        auto deserializer = [&ar]<typename U>() { U u; ar & u; return u; };
        map = tsl::robin_map<Key, T>::deserialize(deserializer);
    }
}}


int main() {
    tsl::robin_map<std::int64_t, std::int64_t> map = {{1, -1}, {2, -2}, {3, -3}, {4, -4}};
    
    
    const char* file_name = "robin_map.data";
    {
        std::ofstream ofs;
        ofs.exceptions(ofs.badbit | ofs.failbit);
        ofs.open(file_name, std::ios::binary);
        
        boost::iostreams::filtering_ostream fo;
        fo.push(boost::iostreams::zlib_compressor());
        fo.push(ofs);
        
        boost::archive::binary_oarchive oa(fo);
        
        oa << map;
    }
    
    {
        std::ifstream ifs;
        ifs.exceptions(ifs.badbit | ifs.failbit | ifs.eofbit);
        ifs.open(file_name, std::ios::binary);
        
        boost::iostreams::filtering_istream fi;
        fi.push(boost::iostreams::zlib_decompressor());
        fi.push(ifs);
        
        boost::archive::binary_iarchive ia(fi);
     
        tsl::robin_map<std::int64_t, std::int64_t> map_deserialized;   
        ia >> map_deserialized;
        
        assert(map == map_deserialized);
    }
}
```

#### Performance pitfalls

Two potential performance pitfalls involving `tsl::robin_map` and
`tsl::robin_set` are noteworthy:

1. *Bad hashes*. Hash functions that produce many collisions can lead to the
   following surprising behavior: when the number of collisions exceeds a
   certain threshold, the hash table will automatically expand to fix the
   problem. However, in degenerate cases, this expansion might have _no effect_
   on the collision count, causing a failure mode where a linear sequence of
   insertion leads to exponential storage growth.

   This case has mainly been observed when using the default power-of-two
   growth strategy with the default STL `std::hash<T>` for arithmetic types
   `T`, which is often an identity! See issue
   [#39](https://github.com/Tessil/robin-map/issues/39) for an example. The
   solution is simple: use a better hash function and/or `tsl::robin_pg_set` /
   `tsl::robin_pg_map`.

2. *Element erasure and low load factors*. `tsl::robin_map` and
   `tsl::robin_set` mirror the STL map/set API, which exposes an `iterator
   erase(iterator)` method that removes an element at a certain position,
   returning a valid iterator that points to the next element.

   Constructing this new iterator object requires walking to the next nonempty
   bucket in the table, which can be a expensive operation when the hash table
   has a low *load factor* (i.e., when `capacity()` is much larger then
   `size()`).

   The `erase()` method furthermore never shrinks & re-hashes the table as
   this is not permitted by the specification of this function. A linear
   sequence of random removals without intermediate insertions can then lead to
   a degenerate case with quadratic runtime cost.

   In such cases, an iterator return value is often not even needed, so the
   cost is entirely unnecessary. Both `tsl::robin_set` and `tsl::robin_map`
   therefore provide an alternative erasure method `void erase_fast(iterator)`
   that does not return an iterator to avoid having to find the next element.

### License

The code is licensed under the MIT license, see the [LICENSE file](LICENSE) for details.
