/**
 * MIT License
 * 
 * Copyright (c) 2017 Tessil
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef TSL_ROBIN_GROWTH_POLICY_H
#define TSL_ROBIN_GROWTH_POLICY_H 


#include <algorithm>
#include <array>
#include <climits>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <limits>
#include <ratio>
#include <stdexcept>

#ifdef __EXCEPTIONS
#   define THROW(_e, _m) throw _e(_m)
#else
#   include <stdio.h>
#   ifndef NDEBUG
#       define THROW(_e, _m) do { fprintf(stderr, _m); std::terminate(); } while(0)
#   else
#       define THROW(_e, _m) std::terminate()
#   endif
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if __has_builtin(__builtin_expect)
#   define TSL_LIKELY( exp )    (__builtin_expect( !!(exp), true ))
#else
#   define TSL_LIKELY( exp )    (exp)
#endif

namespace tsl {
namespace rh {
    
/**
 * Grow the hash table by a factor of GrowthFactor keeping the bucket count to a power of two. It allows
 * the table to use a mask operation instead of a modulo operation to map a hash to a bucket.
 * 
 * GrowthFactor must be a power of two >= 2.
 */
template<std::size_t GrowthFactor>
class power_of_two_growth_policy {
public:
    /**
     * Called on the hash table creation and on rehash. The number of buckets for the table is passed in parameter.
     * This number is a minimum, the policy may update this value with a higher value if needed (but not lower).
     */
    power_of_two_growth_policy(std::size_t& min_bucket_count_in_out) {
        if(min_bucket_count_in_out > max_bucket_count()) {
            THROW(std::length_error, "The hash table exceeds its maxmimum size.");
        }
        
        static_assert(MIN_BUCKETS_SIZE > 0, "MIN_BUCKETS_SIZE must be > 0.");
        const std::size_t min_bucket_count = MIN_BUCKETS_SIZE;
        
        min_bucket_count_in_out = std::max(min_bucket_count, min_bucket_count_in_out);
        min_bucket_count_in_out = round_up_to_power_of_two(min_bucket_count_in_out);
        m_mask = min_bucket_count_in_out - 1;
    }
    
    /**
     * Return the bucket [0, bucket_count()) to which the hash belongs.
     */
    std::size_t bucket_for_hash(std::size_t hash) const noexcept {
        return hash & m_mask;
    }
    
    /**
     * Return the bucket count to use when the bucket array grows on rehash.
     */
    std::size_t next_bucket_count() const {
        if((m_mask + 1) > max_bucket_count() / GrowthFactor) {
            THROW(std::length_error, "The hash table exceeds its maxmimum size.");
        }
        
        return (m_mask + 1) * GrowthFactor;
    }
    
    /**
     * Return the maximum number of buckets supported by the policy.
     */
    std::size_t max_bucket_count() const {
        // Largest power of two.
        return (std::numeric_limits<std::size_t>::max() / 2) + 1;
    }
    
private:
    static std::size_t round_up_to_power_of_two(std::size_t value) {
        if(is_power_of_two(value)) {
            return value;
        }
        
        if(value == 0) {
            return 1;
        }
            
        --value;
        for(std::size_t i = 1; i < sizeof(std::size_t) * CHAR_BIT; i *= 2) {
            value |= value >> i;
        }
        
        return value + 1;
    }
    
    static constexpr bool is_power_of_two(std::size_t value) {
        return value != 0 && (value & (value - 1)) == 0;
    }
    
protected:
    static const std::size_t MIN_BUCKETS_SIZE = 2;
    static_assert(is_power_of_two(GrowthFactor) && GrowthFactor >= 2, "GrowthFactor must be a power of two >= 2.");
    
    std::size_t m_mask;
};


/**
 * Grow the hash table by GrowthFactor::num / GrowthFactor::den and use a modulo to map a hash
 * to a bucket. Slower but it can be usefull if you want a slower growth.
 */
template<class GrowthFactor = std::ratio<3, 2>>
class mod_growth_policy {
public:
    mod_growth_policy(std::size_t& min_bucket_count_in_out) {
        if(min_bucket_count_in_out > max_bucket_count()) {
            THROW(std::length_error, "The hash table exceeds its maxmimum size.");
        }
        
        static_assert(MIN_BUCKETS_SIZE > 0, "MIN_BUCKETS_SIZE must be > 0.");
        const std::size_t min_bucket_count = MIN_BUCKETS_SIZE;
        
        min_bucket_count_in_out = std::max(min_bucket_count, min_bucket_count_in_out);
        m_bucket_count = min_bucket_count_in_out;
    }
    
    std::size_t bucket_for_hash(std::size_t hash) const noexcept {
        return hash % m_bucket_count;
    }
    
    std::size_t next_bucket_count() const {
        if(m_bucket_count == max_bucket_count()) {
            THROW(std::length_error, "The hash table exceeds its maxmimum size.");
        }
        
        const double next_bucket_count = std::ceil(double(m_bucket_count) * REHASH_SIZE_MULTIPLICATION_FACTOR);
        if(!std::isnormal(next_bucket_count)) {
            THROW(std::length_error, "The hash table exceeds its maxmimum size.");
        }
        
        if(next_bucket_count > double(max_bucket_count())) {
            return max_bucket_count();
        }
        else {
            return std::size_t(next_bucket_count);
        }
    }
    
    std::size_t max_bucket_count() const {
        return MAX_BUCKET_COUNT;
    }
    
private:
    static const std::size_t MIN_BUCKETS_SIZE = 2;
    static constexpr double REHASH_SIZE_MULTIPLICATION_FACTOR = 1.0 * GrowthFactor::num / GrowthFactor::den;
    static const std::size_t MAX_BUCKET_COUNT = 
            std::size_t(double(
                    std::numeric_limits<std::size_t>::max() / REHASH_SIZE_MULTIPLICATION_FACTOR
            ));
            
    static_assert(REHASH_SIZE_MULTIPLICATION_FACTOR >= 1.1, "Growth factor should be >= 1.1.");
    
    std::size_t m_bucket_count;
};



namespace detail {

static constexpr const std::array<std::size_t, 39> PRIMES = {{
    5ul, 17ul, 29ul, 37ul, 53ul, 67ul, 79ul, 97ul, 131ul, 193ul, 257ul, 389ul, 521ul, 769ul, 1031ul, 1543ul, 2053ul, 
    3079ul, 6151ul, 12289ul, 24593ul, 49157ul, 98317ul, 196613ul, 393241ul, 786433ul, 1572869ul, 3145739ul, 
    6291469ul, 12582917ul, 25165843ul, 50331653ul, 100663319ul, 201326611ul, 402653189ul, 805306457ul, 
    1610612741ul, 3221225473ul, 4294967291ul
}};

template<unsigned int IPrime>
static constexpr std::size_t mod(std::size_t hash) { return hash % PRIMES[IPrime]; }

// MOD_PRIME[iprime](hash) returns hash % PRIMES[iprime]. This table allows for faster modulo as the
// compiler can optimize the modulo code better with a constant known at the compilation.
static constexpr const std::array<std::size_t(*)(std::size_t), 39> MOD_PRIME = {{ 
    &mod<0>, &mod<1>, &mod<2>, &mod<3>, &mod<4>, &mod<5>, &mod<6>, &mod<7>, &mod<8>, &mod<9>, &mod<10>, 
    &mod<11>, &mod<12>, &mod<13>, &mod<14>, &mod<15>, &mod<16>, &mod<17>, &mod<18>, &mod<19>, &mod<20>, 
    &mod<21>, &mod<22>, &mod<23>, &mod<24>, &mod<25>, &mod<26>, &mod<27>, &mod<28>, &mod<29>, &mod<30>, 
    &mod<31>, &mod<32>, &mod<33>, &mod<34>, &mod<35>, &mod<36>, &mod<37> , &mod<38>
}};

}

/**
 * Grow the hash table by using prime numbers as bucket count. Slower than tsl::rh::power_of_two_growth_policy in  
 * general but will probably distribute the values around better in the buckets with a poor hash function.
 * 
 * To allow the compiler to optimize the modulo operation, a lookup table is used with constant primes numbers.
 * 
 * With a switch the code would look like:
 * \code
 * switch(iprime) { // iprime is the current prime of the hash table
 *     case 0: hash % 5ul;
 *             break;
 *     case 1: hash % 17ul;
 *             break;
 *     case 2: hash % 29ul;
 *             break;
 *     ...
 * }    
 * \endcode
 * 
 * Due to the constant variable in the modulo the compiler is able to optimize the operation
 * by a series of multiplications, substractions and shifts. 
 * 
 * The 'hash % 5' could become something like 'hash - (hash * 0xCCCCCCCD) >> 34) * 5' in a 64 bits environement.
 */
class prime_growth_policy {
public:
    prime_growth_policy(std::size_t& min_bucket_count_in_out) {
        auto it_prime = std::lower_bound(detail::PRIMES.begin(), 
                                         detail::PRIMES.end(), min_bucket_count_in_out);
        if(it_prime == detail::PRIMES.end()) {
            THROW(std::length_error, "The hash table exceeds its maxmimum size.");
        }
        
        m_iprime = static_cast<unsigned int>(std::distance(detail::PRIMES.begin(), it_prime));
        min_bucket_count_in_out = *it_prime;
    }
    
    std::size_t bucket_for_hash(std::size_t hash) const noexcept {
        return detail::MOD_PRIME[m_iprime](hash);
    }
    
    std::size_t next_bucket_count() const {
        if(m_iprime + 1 >= detail::PRIMES.size()) {
            THROW(std::length_error, "The hash table exceeds its maxmimum size.");
        }
        
        return detail::PRIMES[m_iprime + 1];
    }   
    
    std::size_t max_bucket_count() const {
        return detail::PRIMES.back();
    }
    
private:
    unsigned int m_iprime;
    
    static_assert(std::numeric_limits<decltype(m_iprime)>::max() >= detail::PRIMES.size(), 
                  "The type of m_iprime is not big enough.");
}; 

}
}

#endif
