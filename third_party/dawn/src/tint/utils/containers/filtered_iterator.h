// Copyright 2024 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_UTILS_CONTAINERS_FILTERED_ITERATOR_H_
#define SRC_TINT_UTILS_CONTAINERS_FILTERED_ITERATOR_H_

#include <utility>

namespace tint {

/// FilteredIterator is an iterator that skip over elements where the predicate function returns
/// false.
/// @tparam PREDICATE the filter predicate function
/// @tparam ITERATOR the inner iterator type
template <typename PREDICATE, typename ITERATOR>
class FilteredIterator {
  public:
    /// @param current the current iterator.
    /// @param end the end iterator.
    FilteredIterator(ITERATOR current, ITERATOR end) : current_(current), end_(end) {
        // Skip to first element that passes the predicate
        while (current_ != end && !PREDICATE{}(*current_)) {
            ++current_;
        }
    }

    /// Copy constructor
    FilteredIterator(const FilteredIterator&) = default;

    /// Copy assignment operator
    FilteredIterator& operator=(const FilteredIterator&) = default;

    /// Move constructor
    FilteredIterator(FilteredIterator&&) = default;

    /// Move assignment operator
    FilteredIterator& operator=(FilteredIterator&&) = default;

    /// Increments the iterator until PREDICATE returns true, or the end is reached.
    /// @return this FilteredIterator
    FilteredIterator& operator++() {
        ++current_;
        while (current_ != end_ && !PREDICATE{}(*current_)) {
            ++current_;
        }
        return *this;
    }

    /// @returns the element at the current iterator
    auto operator*() const { return *current_; }

    /// @returns the element at the current iterator
    auto operator->() const { return current_.operator->(); }

    /// Equality operator
    /// @returns true if this iterator is equal to @p other
    bool operator==(const FilteredIterator& other) const { return current_ == other.current_; }

    /// Inequality operator
    /// @returns true if this iterator is not equal to @p other
    bool operator!=(const FilteredIterator& other) const { return current_ != other.current_; }

  private:
    /// The current iterator.
    ITERATOR current_;
    /// The end iterator.
    ITERATOR end_;
};

/// FilteredIterable is a wrapper around an object with begin() / end() methods, that returns
/// iterators that skip over elements where the predicate function returns false.
/// @tparam PREDICATE the filter predicate function
/// @tparam ITERABLE the inner iterable object type
template <typename PREDICATE, typename ITERABLE>
struct FilteredIterable {
    /// The iterator type
    using iterator = typename std::decay_t<ITERABLE>::iterator;
    /// The const iterator type
    using const_iterator = typename std::decay_t<ITERABLE>::const_iterator;

    /// The wrapped iterable object.
    ITERABLE iterable;

    /// @return the filtered start iterator
    FilteredIterator<PREDICATE, iterator> begin() { return {iterable.begin(), iterable.end()}; }

    /// @return the filtered end iterator
    FilteredIterator<PREDICATE, iterator> end() { return {iterable.end(), iterable.end()}; }

    /// @return the filtered const start iterator
    FilteredIterator<PREDICATE, const_iterator> begin() const {
        return {iterable.begin(), iterable.end()};
    }

    /// @return the filtered const end iterator
    FilteredIterator<PREDICATE, const_iterator> end() const {
        return {iterable.end(), iterable.end()};
    }
};

/// @returns a FilteredIterable from the filter function `PREDICATE`, wrapping @p iterable
/// @tparam PREDICATE the filter predicate function
/// @tparam ITERABLE the inner iterable object type
template <typename PREDICATE, typename ITERABLE>
FilteredIterable<PREDICATE, ITERABLE&> Filter(ITERABLE& iterable) {
    return {iterable};
}

}  // namespace tint

#endif  // SRC_TINT_UTILS_CONTAINERS_FILTERED_ITERATOR_H_
