/* Copyright (c) 2019-2024 The Khronos Group Inc.
 * Copyright (c) 2019-2024 Valve Corporation
 * Copyright (c) 2019-2024 LunarG, Inc.
 * Copyright (C) 2019-2024 Google Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <map>
#include <optional>
#include <string>
#include <sstream>
#include <utility>
#include <cstdint>
#include <vulkan/utility/vk_small_containers.hpp>

namespace vku {
namespace sparse {
// range_map
//
// Implements an ordered map of non-overlapping, non-empty ranges
//
template <typename Index>
struct range {
    using index_type = Index;
    index_type begin;  // Inclusive lower bound of range
    index_type end;    // Exlcusive upper bound of range

    inline bool empty() const { return begin == end; }
    inline bool valid() const { return begin <= end; }
    inline bool invalid() const { return !valid(); }
    inline bool non_empty() const { return begin < end; }  //  valid and !empty

    inline bool is_prior_to(const range &other) const { return end == other.begin; }
    inline bool is_subsequent_to(const range &other) const { return begin == other.end; }
    inline bool includes(const index_type &index) const { return (begin <= index) && (index < end); }
    inline bool includes(const range &other) const { return (begin <= other.begin) && (other.end <= end); }
    inline bool excludes(const index_type &index) const { return (index < begin) || (end <= index); }
    inline bool excludes(const range &other) const { return (other.end <= begin) || (end <= other.begin); }
    inline bool intersects(const range &other) const { return includes(other.begin) || other.includes(begin); }
    inline index_type distance() const { return end - begin; }

    inline bool operator==(const range &rhs) const { return (begin == rhs.begin) && (end == rhs.end); }
    inline bool operator!=(const range &rhs) const { return (begin != rhs.begin) || (end != rhs.end); }

    inline range &operator-=(const index_type &offset) {
        begin = begin - offset;
        end = end - offset;
        return *this;
    }

    inline range &operator+=(const index_type &offset) {
        begin = begin + offset;
        end = end + offset;
        return *this;
    }

    inline range operator+(const index_type &offset) const { return range(begin + offset, end + offset); }

    // for a reversible/transitive < operator compare first on begin and then end
    // only less or begin is less or if end is less when begin is equal
    bool operator<(const range &rhs) const {
        bool result = false;
        if (invalid()) {
            // all invalid < valid, allows map/set validity check by looking at begin()->first
            // all invalid are equal, thus only equal if this is invalid and rhs is valid
            result = rhs.valid();
        } else if (begin < rhs.begin) {
            result = true;
        } else if ((begin == rhs.begin) && (end < rhs.end)) {
            result = true;  // Simple common case -- boundary case require equality check for correctness.
        }
        return result;
    }

    // use as "strictly less/greater than" to check for non-overlapping ranges
    bool strictly_less(const range &rhs) const { return end <= rhs.begin; }
    bool strictly_less(const index_type &index) const { return end <= index; }
    bool strictly_greater(const range &rhs) const { return rhs.end <= begin; }
    bool strictly_greater(const index_type &index) const { return index < begin; }

    range &operator=(const range &rhs) {
        begin = rhs.begin;
        end = rhs.end;
        return *this;
    }

    // Compute ranges intersection. Returns empty range on non-intersection
    range operator&(const range &rhs) const {
        if (includes(rhs.begin)) {
            return range(rhs.begin, std::min(end, rhs.end));
        } else if (rhs.includes(begin)) {
            return range(begin, std::min(end, rhs.end));
        }
        return range();
    }

    index_type size() const { return end - begin; }
    range() : begin(), end() {}
    range(const index_type &begin_, const index_type &end_) : begin(begin_), end(end_) {}
    range(const range &other) : begin(other.begin), end(other.end) {}
};

template <typename Range>
class range_view {
  public:
    using index_type = typename Range::index_type;
    class iterator {
      public:
        iterator &operator++() {
            ++current;
            return *this;
        }
        const index_type &operator*() const { return current; }
        bool operator!=(const iterator &rhs) const { return current != rhs.current; }
        iterator(index_type value) : current(value) {}

      private:
        index_type current;
    };
    range_view(const Range &range) : range_(range) {}
    const iterator begin() const { return iterator(range_.begin); }
    const iterator end() const { return iterator(range_.end); }

  private:
    const Range &range_;
};

template <typename Range>
std::string string_range(const Range &range) {
    std::stringstream ss;
    ss << "[" << range.begin << ", " << range.end << ')';
    return ss.str();
}

template <typename Range>
std::string string_range_hex(const Range &range) {
    std::stringstream ss;
    ss << std::hex << "[0x" << range.begin << ", 0x" << range.end << ')';
    return ss.str();
}

// Type parameters for the range_map(s)
struct insert_range_no_split_bounds {
    const static bool split_boundaries = false;
};

struct insert_range_split_bounds {
    const static bool split_boundaries = true;
};

struct split_op_keep_both {
    static constexpr bool keep_lower() { return true; }
    static constexpr bool keep_upper() { return true; }
};

struct split_op_keep_lower {
    static constexpr bool keep_lower() { return true; }
    static constexpr bool keep_upper() { return false; }
};

struct split_op_keep_upper {
    static constexpr bool keep_lower() { return false; }
    static constexpr bool keep_upper() { return true; }
};

enum class value_precedence { prefer_source, prefer_dest };

template <typename Iterator, typename Map, typename Range>
Iterator split(Iterator in, Map &map, const Range &range);

// The range based sparse map implemented on the ImplMap
template <typename Key, typename T, typename RangeKey = range<Key>, typename ImplMap = std::map<RangeKey, T>>
class range_map {
  public:
  protected:
    using MapKey = RangeKey;
    ImplMap impl_map_;
    using ImplIterator = typename ImplMap::iterator;
    using ImplConstIterator = typename ImplMap::const_iterator;

  public:
    using mapped_type = typename ImplMap::mapped_type;
    using value_type = typename ImplMap::value_type;
    using key_type = typename ImplMap::key_type;
    using index_type = typename key_type::index_type;
    using size_type = typename ImplMap::size_type;

  protected:
    template <typename ThisType>
    using ConstCorrectImplIterator = decltype(std::declval<ThisType>().impl_begin());

    template <typename ThisType, typename WrappedIterator = ConstCorrectImplIterator<ThisType>>
    static WrappedIterator lower_bound_impl(ThisType &that, const key_type &key) {
        if (key.valid()) {
            // ImplMap doesn't give us what want with a direct query, it will give us the first entry contained (if any) in key,
            // not the first entry intersecting key, so, first look for the the first entry that starts at or after key.begin
            // with the operator > in range, we can safely use an empty range for comparison
            auto lower = that.impl_map_.lower_bound(key_type(key.begin, key.begin));

            // If there is a preceding entry it's possible that begin is included, as all we know is that lower.begin >= key.begin
            // or lower is at end
            if (!that.at_impl_begin(lower)) {
                auto prev = lower;
                --prev;
                // If the previous entry includes begin (and we know key.begin > prev.begin) then prev is actually lower
                if (key.begin < prev->first.end) {
                    lower = prev;
                }
            }
            return lower;
        }
        // Key is ill-formed
        return that.impl_end();  // Point safely to nothing.
    }

    ImplIterator lower_bound_impl(const key_type &key) { return lower_bound_impl(*this, key); }

    ImplConstIterator lower_bound_impl(const key_type &key) const { return lower_bound_impl(*this, key); }

    template <typename ThisType, typename WrappedIterator = ConstCorrectImplIterator<ThisType>>
    static WrappedIterator upper_bound_impl(ThisType &that, const key_type &key) {
        if (key.valid()) {
            // the upper bound is the first range that is full greater (upper.begin >= key.end
            // we can get close by looking for the first to exclude key.end, then adjust to account for the fact that key.end is
            // exclusive and we thus ImplMap::upper_bound may be off by one here, i.e. the previous may be the upper bound
            auto upper = that.impl_map_.upper_bound(key_type(key.end, key.end));
            if (!that.at_impl_end(upper) && (upper != that.impl_begin())) {
                auto prev = upper;
                --prev;
                // We know key.end  is >= prev.begin, the only question is whether it's ==
                if (prev->first.begin == key.end) {
                    upper = prev;
                }
            }
            return upper;
        }
        return that.impl_end();  // Point safely to nothing.
    }

    ImplIterator upper_bound_impl(const key_type &key) { return upper_bound_impl(*this, key); }

    ImplConstIterator upper_bound_impl(const key_type &key) const { return upper_bound_impl(*this, key); }

    ImplIterator impl_find(const key_type &key) { return impl_map_.find(key); }
    ImplConstIterator impl_find(const key_type &key) const { return impl_map_.find(key); }
    bool impl_not_found(const key_type &key) const { return impl_end() == impl_find(key); }

    ImplIterator impl_end() { return impl_map_.end(); }
    ImplConstIterator impl_end() const { return impl_map_.end(); }

    ImplIterator impl_begin() { return impl_map_.begin(); }
    ImplConstIterator impl_begin() const { return impl_map_.begin(); }

    inline bool at_impl_end(const ImplIterator &pos) { return pos == impl_end(); }
    inline bool at_impl_end(const ImplConstIterator &pos) const { return pos == impl_end(); }

    inline bool at_impl_begin(const ImplIterator &pos) { return pos == impl_begin(); }
    inline bool at_impl_begin(const ImplConstIterator &pos) const { return pos == impl_begin(); }

    ImplIterator impl_erase(const ImplIterator &pos) { return impl_map_.erase(pos); }

    template <typename Value>
    ImplIterator impl_insert(const ImplIterator &hint, Value &&value) {
        assert(impl_not_found(value.first));
        assert(value.first.non_empty());
        return impl_map_.emplace_hint(hint, std::forward<Value>(value));
    }
    ImplIterator impl_insert(const ImplIterator &hint, const key_type &key, const mapped_type &value) {
        return impl_insert(hint, std::make_pair(key, value));
    }

    ImplIterator impl_insert(const ImplIterator &hint, const index_type &begin, const index_type &end, const mapped_type &value) {
        return impl_insert(hint, key_type(begin, end), value);
    }

    template <typename SplitOp>
    ImplIterator split_impl(const ImplIterator &split_it, const index_type &index, const SplitOp &) {
        // Make sure contains the split point
        if (!split_it->first.includes(index)) return split_it;  // If we don't have a valid split point, just return the iterator

        const auto range = split_it->first;
        key_type lower_range(range.begin, index);
        if (lower_range.empty() && SplitOp::keep_upper()) {
            return split_it;  // this is a noop we're keeping the upper half which is the same as split_it;
        }
        // Save the contents of it and erase it
        auto value = split_it->second;
        auto next_it = impl_map_.erase(split_it);  // Keep this, just in case the split point results in an empty "keep" set

        if (lower_range.empty() && !SplitOp::keep_upper()) {
            // This effectively an erase...
            return next_it;
        }
        // Upper range cannot be empty
        key_type upper_range(index, range.end);
        key_type move_range;
        key_type copy_range;

        // Were either going to keep one or both of the split pieces.  If we keep both, we'll copy value to the upper,
        // and move to the lower, and return the lower, else move to, and return the kept one.
        if (SplitOp::keep_lower() && !lower_range.empty()) {
            move_range = lower_range;
            if (SplitOp::keep_upper()) {
                copy_range = upper_range;  // only need a valid copy range if we keep both.
            }
        } else if (SplitOp::keep_upper()) {  // We're not keeping the lower split because it's either empty or not wanted
            move_range = upper_range;        // this will be non_empty as index is included ( < end) in the original range)
        }

        // we insert from upper to lower because that's what emplace_hint can do in constant time. (not log time in C++11)
        if (!copy_range.empty()) {
            // We have a second range to create, so do it by copy
            assert(impl_map_.find(copy_range) == impl_map_.end());
            next_it = impl_map_.emplace_hint(next_it, std::make_pair(copy_range, value));
        }

        if (!move_range.empty()) {
            // Whether we keep one or both, the one we return gets value moved to it, as the other one already has a copy
            assert(impl_map_.find(move_range) == impl_map_.end());
            next_it = impl_map_.emplace_hint(next_it, std::make_pair(move_range, std::move(value)));
        }

        // point to the beginning of the inserted elements (or the next from the erase
        return next_it;
    }

    // do an ranged insert that splits existing ranges at the boundaries, and writes value to any non-initialized sub-ranges
    range<ImplIterator> infill_and_split(const key_type &bounds, const mapped_type &value, ImplIterator lower, bool split_bounds) {
        auto pos = lower;
        if (at_impl_end(pos)) return range<ImplIterator>(pos, pos);  // defensive...

        // Logic assumes we are starting at lower bound
        assert(lower == lower_bound_impl(bounds));

        // Trim/infil the beginning if needed
        const auto first_begin = pos->first.begin;
        if (bounds.begin > first_begin && split_bounds) {
            pos = split_impl(pos, bounds.begin, split_op_keep_both());
            lower = pos;
            ++lower;
            assert(lower == lower_bound_impl(bounds));
        } else if (bounds.begin < first_begin) {
            pos = impl_insert(pos, bounds.begin, first_begin, value);
            lower = pos;
            assert(lower == lower_bound_impl(bounds));
        }

        // in the trim case pos starts one before lower_bound, but that allows trimming a single entry range in loop.
        // NOTE that the loop is trimming and infilling at pos + 1
        while (!at_impl_end(pos) && pos->first.begin < bounds.end) {
            auto last_end = pos->first.end;
            // check for in-fill
            ++pos;
            if (at_impl_end(pos)) {
                if (last_end < bounds.end) {
                    // Gap after last entry in impl_map and before end,
                    pos = impl_insert(pos, last_end, bounds.end, value);
                    ++pos;  // advances to impl_end, as we're at upper boundary
                    assert(at_impl_end(pos));
                }
            } else if (pos->first.begin != last_end) {
                // we have a gap between last entry and current... fill, but not beyond bounds
                if (bounds.includes(pos->first.begin)) {
                    pos = impl_insert(pos, last_end, pos->first.begin, value);
                    //  don't further advance pos, because we may need to split the next entry and thus can't skip it.
                } else if (last_end < bounds.end) {
                    // Non-zero length final gap in-bounds
                    pos = impl_insert(pos, last_end, bounds.end, value);
                    ++pos;  // advances back to the out of bounds entry which we inserted just before
                    assert(!bounds.includes(pos->first.begin));
                }
            } else if (pos->first.includes(bounds.end)) {
                if (split_bounds) {
                    // extends past the end of the bounds range, snip to only include the bounded section
                    // NOTE: this splits pos, but the upper half of the split should now be considered upper_bound
                    // for the range
                    pos = split_impl(pos, bounds.end, split_op_keep_both());
                }
                // advance to the upper half of the split which will be upper_bound  or to next which will both be out of bounds
                ++pos;
                assert(!bounds.includes(pos->first.begin));
            }
        }
        // Return the current position which should be the upper_bound for bounds
        assert(pos == upper_bound_impl(bounds));
        return range<ImplIterator>(lower, pos);
    }

    template <typename TouchOp>
    ImplIterator impl_erase_range(const key_type &bounds, ImplIterator lower, const TouchOp &touch_mapped_value) {
        // Logic assumes we are starting at a valid lower bound
        assert(!at_impl_end(lower));
        assert(lower == lower_bound_impl(bounds));

        // Trim/infill the beginning if needed
        auto current = lower;
        const auto first_begin = current->first.begin;
        if (bounds.begin > first_begin) {
            // Preserve the portion of lower bound excluded from bounds
            if (current->first.end <= bounds.end) {
                // If current ends within the erased bound we can discard the the upper portion of current
                current = split_impl(current, bounds.begin, split_op_keep_lower());
            } else {
                // Keep the upper portion of current for the later split below
                current = split_impl(current, bounds.begin, split_op_keep_both());
            }
            // Exclude the preserved portion
            ++current;
            assert(current == lower_bound_impl(bounds));
        }

        // Loop over completely contained entries and erase them
        while (!at_impl_end(current) && (current->first.end <= bounds.end)) {
            if (touch_mapped_value(current->second)) {
                current = impl_erase(current);
            } else {
                ++current;
            }
        }

        if (!at_impl_end(current) && current->first.includes(bounds.end)) {
            // last entry extends past the end of the bounds range, snip to only erase the bounded section
            current = split_impl(current, bounds.end, split_op_keep_both());
            // test if lower_bound (eventually) computed in split_impl is not empty.
            // If it is not empty, then it contains values inside the bounds range,
            // they need to be touched
            if ((current->first & bounds).non_empty()) {
                if (touch_mapped_value(current->second)) {
                    current = impl_erase(current);
                } else {
                    // make current point to upper bound
                    ++current;
                }
            }
        }

        assert(current == upper_bound_impl(bounds));
        return current;
    }

    template <typename ValueType, typename WrappedIterator_>
    struct iterator_impl {
      public:
        friend class range_map;
        using WrappedIterator = WrappedIterator_;

      private:
        WrappedIterator pos_;

        // Create an iterator at a specific internal state -- only from the parent container
        iterator_impl(const WrappedIterator &pos) : pos_(pos) {}

      public:
        iterator_impl() : iterator_impl(WrappedIterator()) {}
        iterator_impl(const iterator_impl &other) : pos_(other.pos_) {}

        iterator_impl &operator=(const iterator_impl &rhs) {
            pos_ = rhs.pos_;
            return *this;
        }

        inline bool operator==(const iterator_impl &rhs) const { return pos_ == rhs.pos_; }

        inline bool operator!=(const iterator_impl &rhs) const { return pos_ != rhs.pos_; }

        ValueType &operator*() const { return *pos_; }
        ValueType *operator->() const { return &*pos_; }

        iterator_impl &operator++() {
            ++pos_;
            return *this;
        }

        iterator_impl &operator--() {
            --pos_;
            return *this;
        }

        // To allow for iterator -> const_iterator construction
        // NOTE: while it breaks strict encapsulation, it does so less than friend
        const WrappedIterator &get_pos() const { return pos_; }
    };

  public:
    using iterator = iterator_impl<value_type, ImplIterator>;

    // The const iterator must be derived to allow the conversion from iterator, which iterator doesn't support
    class const_iterator : public iterator_impl<const value_type, ImplConstIterator> {
        using Base = iterator_impl<const value_type, ImplConstIterator>;
        friend range_map;

      public:
        const_iterator &operator=(const const_iterator &other) {
            Base::operator=(other);
            return *this;
        }
        const_iterator(const const_iterator &other) : Base(other) {}
        const_iterator(const iterator &it) : Base(ImplConstIterator(it.get_pos())) {}
        const_iterator() : Base() {}

      private:
        const_iterator(const ImplConstIterator &pos) : Base(pos) {}
    };

  protected:
    inline bool at_end(const iterator &it) { return at_impl_end(it.pos_); }
    inline bool at_end(const const_iterator &it) const { return at_impl_end(it.pos_); }
    inline bool at_begin(const iterator &it) { return at_impl_begin(it.pos_); }

    template <typename That, typename Iterator>
    static bool is_contiguous_impl(That *const that, const key_type &range, const Iterator &lower) {
        // Search range or intersection is empty
        if (lower == that->impl_end() || lower->first.excludes(range)) return false;

        if (lower->first.includes(range)) {
            return true;  // there is one entry that contains the whole key range
        }

        bool contiguous = true;
        for (auto pos = lower; contiguous && pos != that->impl_end() && range.includes(pos->first.begin); ++pos) {
            // if current doesn't cover the rest of the key range, check to see that the next is extant and abuts
            if (pos->first.end < range.end) {
                auto next = pos;
                ++next;
                contiguous = (next != that->impl_end()) && pos->first.is_prior_to(next->first);
            }
        }
        return contiguous;
    }

  public:
    iterator end() { return iterator(impl_map_.end()); }                          //  policy and bounds don't matter for end
    const_iterator end() const { return const_iterator(impl_map_.end()); }        //  policy and bounds don't matter for end
    iterator begin() { return iterator(impl_map_.begin()); }                      // with default policy, and thus no bounds
    const_iterator begin() const { return const_iterator(impl_map_.begin()); }    // with default policy, and thus no bounds
    const_iterator cbegin() const { return const_iterator(impl_map_.cbegin()); }  // with default policy, and thus no bounds
    const_iterator cend() const { return const_iterator(impl_map_.cend()); }      // with default policy, and thus no bounds

    iterator erase(const iterator &pos) {
        assert(!at_end(pos));
        return iterator(impl_erase(pos.pos_));
    }

    iterator erase(range<iterator> bounds) {
        auto current = bounds.begin.pos_;
        while (current != bounds.end.pos_) {
            assert(!at_impl_end(current));
            current = impl_map_.erase(current);
        }
        assert(current == bounds.end.pos_);
        return current;
    }

    iterator erase(iterator first, iterator last) { return erase(range<iterator>(first, last)); }

    // Before trying to erase a range, function touch_mapped_value is called on the mapped value.
    // touch_mapped_value is allowed to have it's parameter type to be non const reference.
    // If it returns true, regular erase will occur.
    // Else, range is kept.
    template <typename TouchOp>
    iterator erase_range_or_touch(const key_type &bounds, const TouchOp &touch_mapped_value) {
        auto lower = lower_bound_impl(bounds);

        if (at_impl_end(lower) || !bounds.intersects(lower->first)) {
            // There is nothing in this range lower bound is above bound
            return iterator(lower);
        }
        auto next = impl_erase_range(bounds, lower, touch_mapped_value);
        return iterator(next);
    }

    iterator erase_range(const key_type &bounds) {
        return erase_range_or_touch(bounds, [](const auto &) { return true; });
    }

    void clear() { impl_map_.clear(); }

    iterator find(const key_type &key) { return iterator(impl_map_.find(key)); }

    const_iterator find(const key_type &key) const { return const_iterator(impl_map_.find(key)); }

    iterator find(const index_type &index) {
        auto lower = lower_bound(range<index_type>(index, index + 1));
        if (!at_end(lower) && lower->first.includes(index)) {
            return lower;
        }
        return end();
    }

    const_iterator find(const index_type &index) const {
        auto lower = lower_bound(key_type(index, index + 1));
        if (!at_end(lower) && lower->first.includes(index)) {
            return lower;
        }
        return end();
    }

    iterator lower_bound(const key_type &key) { return iterator(lower_bound_impl(key)); }

    const_iterator lower_bound(const key_type &key) const { return const_iterator(lower_bound_impl(key)); }

    iterator upper_bound(const key_type &key) { return iterator(upper_bound_impl(key)); }

    const_iterator upper_bound(const key_type &key) const { return const_iterator(upper_bound_impl(key)); }

    range<iterator> bounds(const key_type &key) { return {lower_bound(key), upper_bound(key)}; }
    range<const_iterator> cbounds(const key_type &key) const { return {lower_bound(key), upper_bound(key)}; }
    range<const_iterator> bounds(const key_type &key) const { return cbounds(key); }

    using insert_pair = std::pair<iterator, bool>;

    // This is traditional no replacement insert.
    insert_pair insert(const value_type &value) {
        const auto &key = value.first;
        if (!key.non_empty()) {
            // It's an invalid key, early bail pointing to end
            return std::make_pair(end(), false);
        }

        // Look for range conflicts (and an insertion point, which makes the lower_bound *not* wasted work)
        // we don't have to check upper if just check that lower doesn't intersect (which it would if lower != upper)
        auto lower = lower_bound_impl(key);
        if (at_impl_end(lower) || !lower->first.intersects(key)) {
            // range is not even partially overlapped, and lower is strictly > than key
            auto impl_insert = impl_map_.emplace_hint(lower, value);
            // auto impl_insert = impl_map_.emplace(value);
            iterator wrap_it(impl_insert);
            return std::make_pair(wrap_it, true);
        }
        // We don't replace
        return std::make_pair(iterator(lower), false);
    }

    iterator insert(const_iterator hint, const value_type &value) {
        bool hint_open;
        ImplConstIterator impl_next = hint.pos_;
        if (impl_map_.empty()) {
            hint_open = true;
        } else if (impl_next == impl_map_.cbegin()) {
            hint_open = value.first.strictly_less(impl_next->first);
        } else if (impl_next == impl_map_.cend()) {
            auto impl_prev = impl_next;
            --impl_prev;
            hint_open = value.first.strictly_greater(impl_prev->first);
        } else {
            auto impl_prev = impl_next;
            --impl_prev;
            hint_open = value.first.strictly_greater(impl_prev->first) && value.first.strictly_less(impl_next->first);
        }

        if (!hint_open) {
            // Hint was unhelpful, fall back to the non-hinted version
            auto plain_insert = insert(value);
            return plain_insert.first;
        }

        auto impl_insert = impl_map_.insert(impl_next, value);
        return iterator(impl_insert);
    }

    // Try to insert value. If insertion failed, recursively split union of retrieved stored range with inserted range.
    // Split at intersection of stored range and inserted range.
    // Range intersection is merged using merge_op.
    // Ranges before and after this intersection are recursively inserted.
    // merge_pos should have this signature: (mapped_type& current_value, const mapped_type& new_value) -> void
    template <typename MergeOp>
    iterator split_and_merge_insert(const value_type &value, const MergeOp &merge_op) {
        if (!value.first.non_empty()) {
            return end();
        }

        if (auto [it, was_inserted] = insert(value); !was_inserted) {
            // insert failed, so at least one stored range intersects with new range
            const RangeKey it_range = it->first;
            const auto &[inserted_range, insert_mapped_value] = value;
            const RangeKey intersection = it_range & inserted_range;
            // if intersection is empty or invalid, insertion should have succeeded
            assert(intersection.non_empty());

            const iterator split_point_it = split(it, *this, intersection);
            // given it->first and instersection do intersect, split should have succeeded
            assert(split_point_it != end());
            // merge values at inserted range and retrieved range intersection
            merge_op(split_point_it->second, insert_mapped_value);

            // Recursively insert ranges before and after intersection
            const RangeKey range_after_intersection(intersection.end, std::max(it_range.end, inserted_range.end));
            const RangeKey range_before_intersection(std::min(it_range.begin, inserted_range.begin), intersection.begin);
            split_and_merge_insert({range_after_intersection, insert_mapped_value}, merge_op);
            if (range_before_intersection.non_empty()) {
                return split_and_merge_insert({range_before_intersection, insert_mapped_value}, merge_op);
            } else {
                return split_point_it;
            }
        } else {
            return it;
        }
    }

    template <typename SplitOp>
    iterator split(const iterator whole_it, const index_type &index, const SplitOp &split_op) {
        auto split_it = split_impl(whole_it.pos_, index, split_op);
        return iterator(split_it);
    }

    // The overwrite hint here is lower.... and if it's not right... this fails
    template <typename Value>
    iterator overwrite_range(const iterator &lower, Value &&value) {
        // We're not robust to a bad hint, so detect it with extreme prejudice
        // TODO: Add bad hint test to make this robust...
        auto lower_impl = lower.pos_;
        auto insert_hint = lower_impl;
        if (!at_impl_end(lower_impl)) {
            // If we're at end (and the hint is good, there's nothing to erase
            assert(lower == lower_bound(value.first));
            insert_hint = impl_erase_range(value.first, lower_impl, [](const auto &) { return true; });
        }
        auto inserted = impl_insert(insert_hint, std::forward<Value>(value));
        return iterator(inserted);
    }

    template <typename Value>
    iterator overwrite_range(Value &&value) {
        auto lower = lower_bound(value.first);
        return overwrite_range(lower, value);
    }

    bool empty() const { return impl_map_.empty(); }
    size_type size() const { return impl_map_.size(); }

    // For configuration/debug use // Use with caution...
    ImplMap &get_implementation_map() { return impl_map_; }
    const ImplMap &get_implementation_map() const { return impl_map_; }
};

template <typename Container>
using const_correct_iterator = decltype(std::declval<Container>().begin());

// The an array based small ordered map for range keys for use as the range map "ImplMap" as an alternate to std::map
//
// Assumes RangeKey::index_type is unsigned (TBD is it useful to generalize to unsigned?)
// Assumes RangeKey implements begin, end, < and (TBD) from template range above
template <typename Key, typename T, typename RangeKey = range<Key>, size_t N = 64, typename SmallIndex = uint8_t>
class small_range_map {
    using SmallRange = range<SmallIndex>;

  public:
    using mapped_type = T;
    using key_type = RangeKey;
    using value_type = std::pair<const key_type, mapped_type>;
    using index_type = typename key_type::index_type;

    using size_type = SmallIndex;
    template <typename Map_, typename Value_>
    struct IteratorImpl {
      public:
        using Map = Map_;
        using Value = Value_;
        friend Map;
        Value *operator->() const { return map_->get_value(pos_); }
        Value &operator*() const { return *(map_->get_value(pos_)); }
        IteratorImpl &operator++() {
            pos_ = map_->next_range(pos_);
            return *this;
        }
        IteratorImpl &operator--() {
            pos_ = map_->prev_range(pos_);
            return *this;
        }
        IteratorImpl &operator=(const IteratorImpl &other) {
            map_ = other.map_;
            pos_ = other.pos_;
            return *this;
        }
        bool operator==(const IteratorImpl &other) const {
            if (at_end() && other.at_end()) {
                return true;  // all ends are equal
            }
            return (map_ == other.map_) && (pos_ == other.pos_);
        }
        bool operator!=(const IteratorImpl &other) const { return !(*this == other); }

        // At end()
        IteratorImpl() : map_(nullptr), pos_(N) {}
        IteratorImpl(const IteratorImpl &other) : map_(other.map_), pos_(other.pos_) {}

        // Raw getters to allow for const_iterator conversion below
        Map *get_map() const { return map_; }
        SmallIndex get_pos() const { return pos_; }

        bool at_end() const { return (map_ == nullptr) || (pos_ >= map_->get_limit()); }

      protected:
        IteratorImpl(Map *map, SmallIndex pos) : map_(map), pos_(pos) {}

      private:
        Map *map_;
        SmallIndex pos_;  // the begin of the current small_range
    };
    using iterator = IteratorImpl<small_range_map, value_type>;

    // The const iterator must be derived to allow the conversion from iterator, which iterator doesn't support
    class const_iterator : public IteratorImpl<const small_range_map, const value_type> {
        using Base = IteratorImpl<const small_range_map, const value_type>;
        friend small_range_map;

      public:
        const_iterator(const iterator &it) : Base(it.get_map(), it.get_pos()) {}
        const_iterator() : Base() {}

      private:
        const_iterator(const small_range_map *map, SmallIndex pos) : Base(map, pos) {}
    };

    iterator begin() {
        // Either ranges of 0 is valid and begin is 0 and begin *or* it's invalid an points to the first valid range (or end)
        return iterator(this, ranges_[0].begin);
    }
    const_iterator cbegin() const { return const_iterator(this, ranges_[0].begin); }
    const_iterator begin() const { return cbegin(); }
    iterator end() { return iterator(); }
    const_iterator cend() const { return const_iterator(); }
    const_iterator end() const { return cend(); }

    void clear() {
        const SmallRange clear_range(limit_, 0);
        for (SmallIndex i = 0; i < limit_; ++i) {
            auto &range = ranges_[i];
            if (range.begin == i) {
                // Clean up the backing store
                destruct_value(i);
            }
            range = clear_range;
        }
        size_ = 0;
    }

    // Find entry with an exact key match (uncommon use case)
    iterator find(const key_type &key) {
        assert(in_bounds(key));
        if (key.begin < limit_) {
            const SmallIndex small_begin = static_cast<SmallIndex>(key.begin);
            const auto &range = ranges_[small_begin];
            if (range.begin == small_begin) {
                const auto small_end = static_cast<SmallIndex>(key.end);
                if (range.end == small_end) return iterator(this, small_begin);
            }
        }
        return end();
    }
    const_iterator find(const key_type &key) const {
        assert(in_bounds(key));
        if (key.begin < limit_) {
            const SmallIndex small_begin = static_cast<SmallIndex>(key.begin);
            const auto &range = ranges_[small_begin];
            if (range.begin == small_begin) {
                const auto small_end = static_cast<SmallIndex>(key.end);
                if (range.end == small_end) return const_iterator(this, small_begin);
            }
        }
        return end();
    }

    iterator find(const index_type &index) {
        if (index < get_limit()) {
            const SmallIndex small_index = static_cast<SmallIndex>(index);
            const auto &range = ranges_[small_index];
            if (range.valid()) {
                return iterator(this, range.begin);
            }
        }
        return end();
    }

    const_iterator find(const index_type &index) const {
        if (index < get_limit()) {
            const SmallIndex small_index = static_cast<SmallIndex>(index);
            const auto &range = ranges_[small_index];
            if (range.valid()) {
                return const_iterator(this, range.begin);
            }
        }
        return end();
    }

    size_type size() const { return size_; }
    bool empty() const { return 0 == size_; }

    iterator erase(const_iterator pos) {
        assert(pos.map_ == this);
        return erase_impl(pos.get_pos());
    }

    iterator erase(iterator pos) {
        assert(pos.map_ == this);
        return erase_impl(pos.get_pos());
    }

    // Must be called with rvalue or lvalue of value_type
    template <typename Value>
    iterator emplace(Value &&value) {
        const auto &key = value.first;
        assert(in_bounds(key));
        if (key.begin >= limit_) return end();  // Invalid key (end is checked in "is_open")
        const SmallRange range(static_cast<SmallIndex>(key.begin), static_cast<SmallIndex>(key.end));
        if (is_open(key)) {
            // This needs to be the fast path, but I don't see how we can avoid the sanity checks above
            for (auto i = range.begin; i < range.end; ++i) {
                ranges_[i] = range;
            }
            // Update the next information for the previous unused slots (as stored in begin invalidly)
            auto prev = range.begin;
            while (prev > 0) {
                --prev;
                if (ranges_[prev].valid()) break;
                ranges_[prev].begin = range.begin;
            }
            // Placement new into the storage interpreted as Value
            construct_value(range.begin, value_type(std::forward<Value>(value)));
            auto next = range.end;
            // update the previous range information for the next unsed slots (as stored in end invalidly)
            while (next < limit_) {
                // End is exclusive... increment *after* update
                if (ranges_[next].valid()) break;
                ranges_[next].end = range.end;
                ++next;
            }
            return iterator(this, range.begin);
        } else {
            // Can't insert into occupied ranges.
            // if ranges_[key.begin] is valid then this is the collision (starting at .begin
            // if it's invalid .begin points to the overlapping entry from is_open (or end if key was out of range)
            return iterator(this, ranges_[range.begin].begin);
        }
    }

    // As hint is going to be ignored, make it as lightweight as possible, by reference and no conversion construction
    template <typename Value>
    iterator emplace_hint([[maybe_unused]] const const_iterator &hint, Value &&value) {
        // We have direct access so we can drop the hint
        return emplace(std::forward<Value>(value));
    }

    template <typename Value>
    iterator emplace_hint([[maybe_unused]] const iterator &hint, Value &&value) {
        // We have direct access so we can drop the hint
        return emplace(std::forward<Value>(value));
    }

    // Again, hint is going to be ignored, make it as lightweight as possible, by reference and no conversion construction
    iterator insert([[maybe_unused]] const const_iterator &hint, const value_type &value) { return emplace(value); }
    iterator insert([[maybe_unused]] const iterator &hint, const value_type &value) { return emplace(value); }

    std::pair<iterator, bool> insert(const value_type &value) {
        const auto &key = value.first;
        assert(in_bounds(key));
        if (key.begin >= limit_) return std::make_pair(end(), false);  // Invalid key, not inserted.
        if (is_open(key)) {
            return std::make_pair(emplace(value), true);
        }
        // If invalid we point to the subsequent range that collided, if valid begin is the start of the valid range
        const auto &collision_begin = ranges_[key.begin].begin;
        assert(ranges_[collision_begin].valid());
        return std::make_pair(iterator(this, collision_begin), false);
    }

    template <typename SplitOp>
    iterator split(const iterator whole_it, const index_type &index, [[maybe_unused]] const SplitOp &split_op) {
        if (!whole_it->first.includes(index)) return whole_it;  // If we don't have a valid split point, just return the iterator

        const auto &key = whole_it->first;
        const auto small_key = make_small_range(key);
        key_type lower_key(key.begin, index);
        if (lower_key.empty() && SplitOp::keep_upper()) {
            return whole_it;  // this is a noop we're keeping the upper half which is the same as whole_it;
        }

        if ((lower_key.empty() && !SplitOp::keep_upper()) || !(SplitOp::keep_lower() || SplitOp::keep_upper())) {
            // This effectively an erase... so erase.
            return erase(whole_it);
        }

        // Upper range cannot be empty (because the split point would be included...
        const auto small_lower_key = make_small_range(lower_key);
        const SmallRange small_upper_key{small_lower_key.end, small_key.end};
        if (SplitOp::keep_upper()) {
            // Note: create the upper section before the lower, as processing the lower may erase it
            assert(!small_upper_key.empty());
            const key_type upper_key{lower_key.end, key.end};
            if (SplitOp::keep_lower()) {
                construct_value(small_upper_key.begin, std::make_pair(upper_key, get_value(small_key.begin)->second));
            } else {
                // If we aren't keeping the lower, move instead of copy
                construct_value(small_upper_key.begin, std::make_pair(upper_key, std::move(get_value(small_key.begin)->second)));
            }
            for (auto i = small_upper_key.begin; i < small_upper_key.end; ++i) {
                ranges_[i] = small_upper_key;
            }
        } else {
            // rewrite "end" to the next valid range (or end)
            assert(SplitOp::keep_lower());
            auto next = next_range(small_key.begin);
            rerange(small_upper_key, SmallRange(next, small_lower_key.end));
            // for any already invalid, we just rewrite the end.
            rerange_end(small_upper_key.end, next, small_lower_key.end);
        }
        SmallIndex split_index;
        if (SplitOp::keep_lower()) {
            resize_value(small_key.begin, lower_key.end);
            rerange_end(small_lower_key.begin, small_lower_key.end, small_lower_key.end);
            split_index = small_lower_key.begin;
        } else {
            // Remove lower and rewrite empty space
            assert(SplitOp::keep_upper());
            destruct_value(small_key.begin);

            // Rewrite prior empty space (if any)
            auto prev = prev_range(small_key.begin);
            SmallIndex limit = small_lower_key.end;
            SmallIndex start = 0;
            if (small_key.begin != 0) {
                const auto &prev_start = ranges_[prev];
                if (prev_start.valid()) {
                    // If there is a previous used range, the empty space starts after it.
                    start = prev_start.end;
                } else {
                    assert(prev == 0);  // prev_range only returns invalid ranges "off the front"
                    start = prev;
                }
                // for the section *prior* to key begin only need to rewrite the "invalid" begin (i.e. next "in use" begin)
                rerange_begin(start, small_lower_key.begin, limit);
            }
            // for the section being erased rewrite the invalid range reflecting the empty space
            rerange(small_lower_key, SmallRange(limit, start));
            split_index = small_lower_key.end;
        }

        return iterator(this, split_index);
    }

    // For the value.first range rewrite the range...
    template <typename Value>
    iterator overwrite_range(Value &&value) {
        const auto &key = value.first;

        // Small map only has a restricted range supported
        assert(in_bounds(key));
        if (key.end > get_limit()) {
            return end();
        }

        const auto small_key = make_small_range(key);
        clear_out_range(small_key, /* valid clear range */ true);
        construct_value(small_key.begin, std::forward<Value>(value));
        return iterator(this, small_key.begin);
    }

    // We don't need a hint...
    template <typename Value>
    iterator overwrite_range([[maybe_unused]] const iterator &hint, Value &&value) {
        return overwrite_range(std::forward<Value>(value));
    }

    // For the range erase all contents within range, trimming any overlapping ranges
    iterator erase_range(const key_type &range) {
        // Small map only has a restricted range supported
        assert(in_bounds(range));
        if (range.end > get_limit() || range.empty()) {
            return end();
        }
        const auto empty = clear_out_range(make_small_range(range), /* valid clear range */ false);
        return iterator(this, empty.end);
    }

    template <typename Iterator>
    iterator erase(const Iterator &first, const Iterator &last) {
        assert(this == first.map_);
        assert(this == last.map_);
        auto first_pos = !first.at_end() ? first.pos_ : limit_;
        auto last_pos = !last.at_end() ? last.pos_ : limit_;
        assert(first_pos <= last_pos);
        const SmallRange clear_me(first_pos, last_pos);
        if (!clear_me.empty()) {
            const SmallRange empty_range(find_empty_left(clear_me), last_pos);
            clear_and_set_range(empty_range.begin, empty_range.end, make_invalid_range(empty_range));
        }
        return iterator(this, last_pos);
    }

    iterator lower_bound(const key_type &key) { return iterator(this, lower_bound_impl(this, key)); }
    const_iterator lower_bound(const key_type &key) const { return const_iterator(this, lower_bound_impl(this, key)); }

    iterator upper_bound(const key_type &key) { return iterator(this, upper_bound_impl(this, key)); }
    const_iterator upper_bound(const key_type &key) const { return const_iterator(this, upper_bound_impl(this, key)); }

    small_range_map(index_type limit = N) : size_(0), limit_(static_cast<SmallIndex>(limit)) {
        assert(limit <= std::numeric_limits<SmallIndex>::max());
        init_range();
    }

    // Only valid for empty maps
    void set_limit(size_t limit) {
        assert(size_ == 0);
        assert(limit <= std::numeric_limits<SmallIndex>::max());
        limit_ = static_cast<SmallIndex>(limit);
        init_range();
    }
    inline index_type get_limit() const { return static_cast<index_type>(limit_); }

  private:
    inline bool in_bounds(index_type index) const { return index < get_limit(); }
    inline bool in_bounds(const RangeKey &key) const { return key.begin < get_limit() && key.end <= get_limit(); }

    inline SmallRange make_small_range(const RangeKey &key) const {
        assert(in_bounds(key));
        return SmallRange(static_cast<SmallIndex>(key.begin), static_cast<SmallIndex>(key.end));
    }

    inline SmallRange make_invalid_range(const SmallRange &key) const { return SmallRange(key.end, key.begin); }

    bool is_open(const key_type &key) const {
        // Remebering that invalid range.begin is the beginning the next used range.
        const auto small_key = make_small_range(key);
        const auto &range = ranges_[small_key.begin];
        return range.invalid() && small_key.end <= range.begin;
    }
    // Only call this with a valid beginning index
    iterator erase_impl(SmallIndex erase_index) {
        assert(erase_index == ranges_[erase_index].begin);
        auto &range = ranges_[erase_index];
        destruct_value(erase_index);
        // Need to update the ranges to accommodate the erasure
        SmallIndex prev = 0;  // This is correct for the case erase_index is 0....
        if (erase_index != 0) {
            prev = prev_range(erase_index);
            // This works if prev is valid or invalid, because the invalid end will be either 0 (and correct) or the end of the
            // prior valid range and the valid end will be the end of the previous range (and thus correct)
            prev = ranges_[prev].end;
        }
        auto next = next_range(erase_index);
        // We have to be careful of next == limit_...
        if (next < limit_) {
            next = ranges_[next].begin;
        }
        // Rewrite both adjoining and newly empty entries
        SmallRange infill(next, prev);
        for (auto i = prev; i < next; ++i) {
            ranges_[i] = infill;
        }
        return iterator(this, next);
    }
    // This implements the "range lower bound logic" directly on the ranges
    template <typename Map>
    static SmallIndex lower_bound_impl(Map *const that, const key_type &key) {
        if (!that->in_bounds(key.begin)) return that->limit_;
        // If range is invalid, then begin points to the next valid (or end) with must be the lower bound
        // If range is valid, the begin points to a the lowest range that interects key
        const auto lb = that->ranges_[static_cast<SmallIndex>(key.begin)].begin;
        return lb;
    }

    template <typename Map>
    static SmallIndex upper_bound_impl(Map *that, const key_type &key) {
        const auto limit = that->get_limit();
        if (key.end >= limit) return that->limit_;  //  at end
        const auto &end_range = that->ranges_[key.end];
        // If range is invalid, then begin points to the next valid (or end) with must be the upper bound (key < range because
        auto ub = end_range.begin;
        // If range is valid, the begin points to a range that may interects key, which is be upper iff range.begin == key.end
        if (end_range.valid() && (key.end > end_range.begin)) {
            // the ub candidate *intersects* the key, so we have to go to the next range.
            ub = that->next_range(end_range.begin);
        }
        return ub;
    }

    // This is and inclusive "inuse", the entry itself
    SmallIndex find_inuse_right(const SmallRange &range) const {
        if (range.end >= limit_) return limit_;
        // if range is valid, begin is the first use (== range.end), else it's the first used after the invalid range
        return ranges_[range.end].begin;
    }
    // This is an exclusive "inuse", the end of the previous range
    SmallIndex find_inuse_left(const SmallRange &range) const {
        if (range.begin == 0) return 0;
        // if range is valid, end is the end of the first use (== range.begin), else it's the end of the in use range before the
        // invalid range
        return ranges_[range.begin - 1].end;
    }
    SmallRange find_empty(const SmallRange &range) const { return SmallRange(find_inuse_left(range), find_inuse_right(range)); }

    // Clear out the given range, trimming as needed.  The clear_range can be set as valid or invalid
    SmallRange clear_out_range(const SmallRange &clear_range, bool valid_clear_range) {
        // By copy to avoid reranging side affect
        auto first_range = ranges_[clear_range.begin];

        // fast path for matching ranges...
        if (first_range == clear_range) {
            // clobber the existing value
            destruct_value(clear_range.begin);
            if (valid_clear_range) {
                return clear_range;  // This is the overwrite fastpath for matching range
            } else {
                const auto empty_range = find_empty(clear_range);
                rerange(empty_range, make_invalid_range(empty_range));
                return empty_range;
            }
        }

        SmallRange empty_left(clear_range.begin, clear_range.begin);
        SmallRange empty_right(clear_range.end, clear_range.end);

        // The clearout is entirely within a single extant range, trim and set.
        if (first_range.valid() && first_range.includes(clear_range)) {
            // Shuffle around first_range, three cases...
            if (first_range.begin < clear_range.begin) {
                // We have a lower trimmed area to preserve.
                resize_value(first_range.begin, clear_range.begin);
                rerange_end(first_range.begin, clear_range.begin, clear_range.begin);
                if (first_range.end > clear_range.end) {
                    // And an upper portion of first that needs to copy from the lower
                    construct_value(clear_range.end, std::make_pair(key_type(clear_range.end, first_range.end),
                                                                    get_value(first_range.begin)->second));
                    rerange_begin(clear_range.end, first_range.end, clear_range.end);
                } else {
                    assert(first_range.end == clear_range.end);
                    empty_right.end = find_inuse_right(clear_range);
                }
            } else {
                assert(first_range.end > clear_range.end);
                assert(first_range.begin == clear_range.begin);
                // Only an upper trimmed area to preserve, so move the first range value to the upper trim zone.
                resize_value_right(first_range, clear_range.end);
                rerange_begin(clear_range.end, first_range.end, clear_range.end);
                empty_left.begin = find_inuse_left(clear_range);
            }
        } else {
            if (first_range.valid()) {
                if (first_range.begin < clear_range.begin) {
                    // Trim left.
                    assert(first_range.end < clear_range.end);  // we handled the "includes" case above
                    resize_value(first_range.begin, clear_range.begin);
                    rerange_end(first_range.begin, clear_range.begin, clear_range.begin);
                }
            } else {
                empty_left.begin = find_inuse_left(clear_range);
            }

            // rewrite excluded portion of final range
            if (clear_range.end < limit_) {
                const auto &last_range = ranges_[clear_range.end];
                if (last_range.valid()) {
                    // for a valid adjoining range we don't have to change empty_right, but we may have to trim
                    if (last_range.begin < clear_range.end) {
                        resize_value_right(last_range, clear_range.end);
                        rerange_begin(clear_range.end, last_range.end, clear_range.end);
                    }
                } else {
                    // Note: invalid ranges "begin" and the next inuse range (or end)
                    empty_right.end = last_range.begin;
                }
            }
        }

        const SmallRange empty(empty_left.begin, empty_right.end);
        // Clear out the contents
        for (auto i = empty.begin; i < empty.end; ++i) {
            const auto &range = ranges_[i];
            if (range.begin == i) {
                assert(range.valid());
                // Clean up the backing store
                destruct_value(i);
            }
        }

        // Rewrite the ranges
        if (valid_clear_range) {
            rerange_begin(empty_left.begin, empty_left.end, clear_range.begin);
            rerange(clear_range, clear_range);
            rerange_end(empty_right.begin, empty_right.end, clear_range.end);
        } else {
            rerange(empty, make_invalid_range(empty));
        }
        assert(empty.end == limit_ || ranges_[empty.end].valid());
        assert(empty.begin == 0 || ranges_[empty.begin - 1].valid());
        return empty;
    }

    void init_range() {
        const SmallRange init_val(limit_, 0);
        for (SmallIndex i = 0; i < limit_; ++i) {
            ranges_[i] = init_val;
            in_use_[i] = false;
        }
    }
    value_type *get_value(SmallIndex index) {
        assert(index < limit_);  // Must be inbounds
        return reinterpret_cast<value_type *>(&(backing_store_[index]));
    }
    const value_type *get_value(SmallIndex index) const {
        assert(index < limit_);                 // Must be inbounds
        assert(index == ranges_[index].begin);  // Must be the record at begin
        return reinterpret_cast<const value_type *>(&(backing_store_[index]));
    }

    template <typename Value>
    void construct_value(SmallIndex index, Value &&value) {
        assert(!in_use_[index]);
        new (get_value(index)) value_type(std::forward<Value>(value));
        in_use_[index] = true;
        ++size_;
    }

    void destruct_value(SmallIndex index) {
        // there are times when the range and destruct logic clash... allow for double attempted deletes
        if (in_use_[index]) {
            assert(size_ > 0);
            --size_;
            get_value(index)->~value_type();
            in_use_[index] = false;
        }
    }

    // No need to move around the value, when just the key is moving
    // Use the destructor/placement new just in case of a complex key with range's semantics
    // Note: Call resize before rewriting ranges_
    void resize_value(SmallIndex current_begin, index_type new_end) {
        // Destroy and rewrite the key in place
        assert(ranges_[current_begin].end != new_end);
        key_type new_key(current_begin, new_end);
        key_type *key = const_cast<key_type *>(&get_value(current_begin)->first);
        key->~key_type();
        new (key) key_type(new_key);
    }

    inline void rerange_end(SmallIndex old_begin, SmallIndex new_end, SmallIndex new_end_value) {
        for (auto i = old_begin; i < new_end; ++i) {
            ranges_[i].end = new_end_value;
        }
    }
    inline void rerange_begin(SmallIndex new_begin, SmallIndex old_end, SmallIndex new_begin_value) {
        for (auto i = new_begin; i < old_end; ++i) {
            ranges_[i].begin = new_begin_value;
        }
    }
    inline void rerange(const SmallRange &range, const SmallRange &range_value) {
        for (auto i = range.begin; i < range.end; ++i) {
            ranges_[i] = range_value;
        }
    }

    // for resize right need both begin and end...
    void resize_value_right(const SmallRange &current_range, index_type new_begin) {
        // Use move semantics for (potentially) heavyweight mapped_type's
        assert(current_range.begin != new_begin);
        // Move second from it's current location and update the first at the same time
        construct_value(static_cast<SmallIndex>(new_begin),
                        std::make_pair(key_type(new_begin, current_range.end), std::move(get_value(current_range.begin)->second)));
        destruct_value(current_range.begin);
    }

    // Now we can walk a range and rewrite it cleaning up any live contents
    void clear_and_set_range(SmallIndex rewrite_begin, SmallIndex rewrite_end, const SmallRange &new_range) {
        for (auto i = rewrite_begin; i < rewrite_end; ++i) {
            auto &range = ranges_[i];
            if (i == range.begin) {
                destruct_value(i);
            }
            range = new_range;
        }
    }

    SmallIndex next_range(SmallIndex current) const {
        SmallIndex next = ranges_[current].end;
        // If the next range is invalid, skip to the next range, which *must* be (or be end)
        if ((next < limit_) && ranges_[next].invalid()) {
            // For invalid ranges, begin is the beginning of the next range
            next = ranges_[next].begin;
            assert(next == limit_ || ranges_[next].valid());
        }
        return next;
    }

    SmallIndex prev_range(SmallIndex current) const {
        if (current == 0) {
            return 0;
        }

        auto prev = current - 1;
        if (ranges_[prev].valid()) {
            // For valid ranges, the range denoted by begin (as that's where the backing store keeps values
            prev = ranges_[prev].begin;
        } else if (prev != 0) {
            // Invalid but not off the front, we can recur (only once) from the end of the prev range to get the answer
            // For invalid ranges this is the end of the previous range
            prev = prev_range(ranges_[prev].end);
        }
        return prev;
    }

    friend iterator;
    friend const_iterator;
    // Stores range boundaries only
    //     open ranges, stored as inverted, invalid range (begining of next, end of prev]
    //     inuse(begin, end) for all entries  on (begin, end]
    // Used for placement new of T for each range begin.
    struct alignas(alignof(value_type)) BackingStore {
        uint8_t data[sizeof(value_type)];
    };

    SmallIndex size_;
    SmallIndex limit_;
    std::array<SmallRange, N> ranges_;
    std::array<BackingStore, N> backing_store_;
    std::array<bool, N> in_use_;
};

// Forward index iterator, tracking an index value and the appropos lower bound
// returns an index_type, lower_bound pair.  Supports ++,  offset, and seek affecting the index,
// lower bound updates as needed. As the index may specify a range for which no entry exist, dereferenced
// iterator includes an "valid" field, true IFF the lower_bound is not end() and contains [index, index +1)
//
// Must be explicitly invalidated when the underlying map is changed.
template <typename Map>
class cached_lower_bound_impl {
    using plain_map_type = typename std::remove_const<Map>::type;  // Allow instatiation with const or non-const Map
  public:
    using iterator = const_correct_iterator<Map>;
    using key_type = typename plain_map_type::key_type;
    using mapped_type = typename plain_map_type::mapped_type;
    // Both sides of the return pair are const'd because we're returning references/pointers to the *internal* state
    // and we don't want and caller altering internal state.
    using index_type = typename Map::index_type;
    struct value_type {
        const index_type &index;
        const iterator &lower_bound;
        const bool &valid;
        value_type(const index_type &index_, const iterator &lower_bound_, bool &valid_)
            : index(index_), lower_bound(lower_bound_), valid(valid_) {}
    };

  private:
    Map *const map_;
    const iterator end_;
    value_type pos_;

    index_type index_;
    iterator lower_bound_;
    bool valid_;

    bool is_valid() const { return includes(index_); }

    // Allow reuse of a type with const semantics
    void set_value(const index_type &index, const iterator &it) {
        assert(it == lower_bound(index));
        index_ = index;
        lower_bound_ = it;
        valid_ = is_valid();
    }

    void update(const index_type &index) {
        assert(lower_bound_ == lower_bound(index));
        index_ = index;
        valid_ = is_valid();
    }

    inline iterator lower_bound(const index_type &index) { return map_->lower_bound(key_type(index, index + 1)); }
    inline bool at_end(const iterator &it) const { return it == end_; }

    bool is_lower_than(const index_type &index, const iterator &it) { return at_end(it) || (index < it->first.end); }

  public:
    // The cached lower bound knows the parent map, and thus can tell us this...
    inline bool at_end() const { return at_end(lower_bound_); }
    // includes(index) is a convenience function to test if the index would be in the currently cached lower bound
    bool includes(const index_type &index) const { return !at_end() && lower_bound_->first.includes(index); }

    // The return is const because we are sharing the internal state directly.
    const value_type &operator*() const { return pos_; }
    const value_type *operator->() const { return &pos_; }

    // Advance the cached location by 1
    cached_lower_bound_impl &operator++() {
        const index_type next = index_ + 1;
        if (is_lower_than(next, lower_bound_)) {
            update(next);
        } else {
            // if we're past pos_->second, next *must* be the new lower bound.
            // NOTE: that next can't be past end, so lower_bound_ isn't end.
            auto next_it = lower_bound_;
            ++next_it;
            set_value(next, next_it);

            // However we *must* not be past next.
            assert(is_lower_than(next, next_it));
        }

        return *this;
    }

    // seek(index) updates lower_bound for index, updating lower_bound_ as needed.
    cached_lower_bound_impl &seek(const index_type &seek_to) {
        // Optimize seeking to  forward
        if (index_ == seek_to) {
            // seek to self is a NOOP.  To reset lower bound after a map change, use invalidate
        } else if (index_ < seek_to) {
            // See if the current or next ranges are the appropriate lower_bound... should be a common use case
            if (is_lower_than(seek_to, lower_bound_)) {
                // lower_bound_ is still the correct lower bound
                update(seek_to);
            } else {
                // Look to see if the next range is the new lower_bound (and we aren't at end)
                auto next_it = lower_bound_;
                ++next_it;
                if (is_lower_than(seek_to, next_it)) {
                    // next_it is the correct new lower bound
                    set_value(seek_to, next_it);
                } else {
                    // We don't know where we are...  and we aren't going to walk the tree looking for seek_to.
                    set_value(seek_to, lower_bound(seek_to));
                }
            }
        } else {
            // General case... this is += so we're not implmenting optimized negative offset logic
            set_value(seek_to, lower_bound(seek_to));
        }
        return *this;
    }

    // Advance the cached location by offset.
    cached_lower_bound_impl &offset(const index_type &offset) {
        const index_type next = index_ + offset;
        return seek(next);
    }

    // invalidate() resets the the lower_bound_ cache, needed after insert/erase/overwrite/split operations
    // Pass index by value in case we are invalidating to index_ and set_value does a modify-in-place on index_
    cached_lower_bound_impl &invalidate(index_type index) {
        set_value(index, lower_bound(index));
        return *this;
    }

    // For times when the application knows what it's done to the underlying map... (with assert in set_value)
    cached_lower_bound_impl &invalidate(const iterator &hint, index_type index) {
        set_value(index, hint);
        return *this;
    }

    cached_lower_bound_impl &invalidate() { return invalidate(index_); }

    // Allow a hint for a *valid* lower bound for current index
    // TODO: if the fail-over becomes a hot-spot, the hint logic could be far more clever (looking at previous/next...)
    cached_lower_bound_impl &invalidate(const iterator &hint) {
        if ((hint != end_) && hint->first.includes(index_)) {
            auto index = index_;  // by copy set modifies in place
            set_value(index, hint);
        } else {
            invalidate();
        }
        return *this;
    }

    // The offset in index type to the next change (the end of the current range, or the transition from invalid to
    // valid.  If invalid and at_end, returns index_type(0)
    index_type distance_to_edge() {
        if (valid_) {
            // Distance to edge of
            return lower_bound_->first.end - index_;
        } else if (at_end()) {
            return index_type(0);
        } else {
            return lower_bound_->first.begin - index_;
        }
    }

    Map &map() { return *map_; }
    const Map &map() const { return *map_; }

    // Default constructed object reports valid (correctly) as false, but otherwise will fail (assert) under nearly any use.
    cached_lower_bound_impl()
        : map_(nullptr), end_(), pos_(index_, lower_bound_, valid_), index_(0), lower_bound_(), valid_(false) {}
    cached_lower_bound_impl(Map &map, const index_type &index)
        : map_(&map),
          end_(map.end()),
          pos_(index_, lower_bound_, valid_),
          index_(index),
          lower_bound_(lower_bound(index)),
          valid_(is_valid()) {}
};

template <typename CachedLowerBound, typename MappedType = typename CachedLowerBound::mapped_type>
const MappedType &evaluate(const CachedLowerBound &clb, const MappedType &default_value) {
    if (clb->valid) {
        return clb->lower_bound->second;
    }
    return default_value;
}

// Split a range into pieces bound by the intersection of the iterator's range and the supplied range
template <typename Iterator, typename Map, typename Range>
Iterator split(Iterator in, Map &map, const Range &range) {
    assert(in != map.end());  // Not designed for use with invalid iterators...
    const auto in_range = in->first;
    const auto split_range = in_range & range;

    if (split_range.empty()) return map.end();

    auto pos = in;
    if (split_range.begin != in_range.begin) {
        pos = map.split(pos, split_range.begin, split_op_keep_both());
        ++pos;
    }
    if (split_range.end != in_range.end) {
        pos = map.split(pos, split_range.end, split_op_keep_both());
    }
    return pos;
}

// Apply an operation over a range map, infilling where content is absent, updating where content is present.
// The passed pos must *either* be strictly less than range or *is* lower_bound (which may be end)
// Trims to range boundaries.
// infill op doesn't have to alter map, but mustn't invalidate iterators passed to it. (i.e. no erasure)
// infill data (default mapped value or other initial value) is contained with ops.
// update allows existing ranges to be updated (merged, whatever) based on data contained in ops.  All iterators
// passed to update are already trimmed to fit within range.
template <typename RangeMap, typename InfillUpdateOps, typename Iterator = typename RangeMap::iterator>
Iterator infill_update_range(RangeMap &map, Iterator pos, const typename RangeMap::key_type &range, const InfillUpdateOps &ops) {
    using KeyType = typename RangeMap::key_type;
    using IndexType = typename RangeMap::index_type;

    const auto end = map.end();
    assert((pos == end) || (pos == map.lower_bound(range)) || pos->first.strictly_less(range));

    if (range.empty()) return pos;
    if (pos == end) {
        // Only pass pos == end for range tail after last entry
        assert(end == map.lower_bound(range));
    } else if (pos->first.strictly_less(range)) {
        // pos isn't lower_bound for range (it's less than range), however, if range is monotonically increasing it's likely
        // the next entry in the map will be the lower bound.

        // If the new (pos + 1) *isn't* stricly_less and pos is,
        // (pos + 1) must be the lower_bound, otherwise we have to look for it O(log n)
        ++pos;
        if ((pos != end) && pos->first.strictly_less(range)) {
            pos = map.lower_bound(range);
        }
        assert(pos == map.lower_bound(range));
    }

    if ((pos != end) && (range.begin > pos->first.begin)) {
        // lower bound starts before the range, trim and advance
        pos = map.split(pos, range.begin, split_op_keep_both());
        ++pos;
    }

    IndexType current_begin = range.begin;
    while ((pos != end) && (current_begin < range.end)) {
        // The current_begin is either pointing to the next existing value to update or the beginning of a gap to infill
        assert(pos->first.begin >= current_begin);

        if (current_begin < pos->first.begin) {
            // We have a gap to infill (we supply pos for ("insert in front of" calls)
            ops.infill(map, pos, KeyType(current_begin, std::min(range.end, pos->first.begin)));
            // Advance current begin, but *not* pos as it's the next valid value. (infill shall not invalidate pos)
            current_begin = pos->first.begin;
        } else {
            // We need to run the update operation on the valid portion of the current value
            if (pos->first.end > range.end) {
                // If this entry overlaps end-of-range we need to trim it to the range
                pos = map.split(pos, range.end, split_op_keep_both());
            }

            // We have a valid fully contained range, merge with it
            ops.update(pos);

            // Advance the current location and map entry
            current_begin = pos->first.end;
            ++pos;
        }
    }

    // Fill to the end as needed
    if (current_begin < range.end) {
        ops.infill(map, pos, KeyType(current_begin, range.end));
    }

    return pos;
}

template <typename RangeMap, typename InfillUpdateOps>
void infill_update_range(RangeMap &map, const typename RangeMap::key_type &range, const InfillUpdateOps &ops) {
    if (range.empty()) return;
    auto pos = map.lower_bound(range);
    infill_update_range(map, pos, range, ops);
}

template <typename RangeMap, typename RangeGen, typename InfillUpdateOps>
void infill_update_rangegen(RangeMap &map, RangeGen &range_gen, const InfillUpdateOps &ops) {
    auto pos = map.lower_bound(*range_gen);
    for (; range_gen->non_empty(); ++range_gen) {
        pos = infill_update_range(map, pos, *range_gen, ops);
    }
}

// Parallel iterator
// Traverse to range maps over the the same range, but without assumptions of aligned ranges.
// ++ increments to the next point where on of the two maps changes range, giving a range over which the two
// maps do not transition ranges
template <typename MapA, typename MapB = MapA, typename KeyType = typename MapA::key_type>
class parallel_iterator {
  public:
    using key_type = KeyType;
    using index_type = typename key_type::index_type;

    // The traits keep the iterator/const_interator consistent with the constness of the map.
    using map_type_A = MapA;
    using plain_map_type_A = typename std::remove_const<MapA>::type;  // Allow instatiation with const or non-const Map
    using key_type_A = typename plain_map_type_A::key_type;
    using index_type_A = typename plain_map_type_A::index_type;
    using iterator_A = const_correct_iterator<map_type_A>;
    using lower_bound_A = cached_lower_bound_impl<map_type_A>;

    using map_type_B = MapB;
    using plain_map_type_B = typename std::remove_const<MapB>::type;
    using key_type_B = typename plain_map_type_B::key_type;
    using index_type_B = typename plain_map_type_B::index_type;
    using iterator_B = const_correct_iterator<map_type_B>;
    using lower_bound_B = cached_lower_bound_impl<map_type_B>;

    // This is the value we'll always be returning, but the referenced object will be updated by the operations
    struct value_type {
        const key_type &range;
        const lower_bound_A &pos_A;
        const lower_bound_B &pos_B;
        value_type(const key_type &range_, const lower_bound_A &pos_A_, const lower_bound_B &pos_B_)
            : range(range_), pos_A(pos_A_), pos_B(pos_B_) {}
    };

  private:
    lower_bound_A pos_A_;
    lower_bound_B pos_B_;
    key_type range_;
    value_type pos_;
    index_type compute_delta() {
        auto delta_A = pos_A_.distance_to_edge();
        auto delta_B = pos_B_.distance_to_edge();
        index_type delta_min;

        // If either A or B are at end, there distance is *0*, so shouldn't be considered in the "distance to edge"
        if (delta_A == 0) {  // lower A is at end
            delta_min = static_cast<index_type>(delta_B);
        } else if (delta_B == 0) {  // lower B is at end
            delta_min = static_cast<index_type>(delta_A);
        } else {
            // Neither are at end, use the nearest edge, s.t. over this range A and B are both constant
            delta_min = std::min(static_cast<index_type>(delta_A), static_cast<index_type>(delta_B));
        }
        return delta_min;
    }

  public:
    // Default constructed object will report range empty (for end checks), but otherwise is unsafe to use
    parallel_iterator() : pos_A_(), pos_B_(), range_(), pos_(range_, pos_A_, pos_B_) {}
    parallel_iterator(map_type_A &map_A, map_type_B &map_B, index_type index)
        : pos_A_(map_A, static_cast<index_type_A>(index)),
          pos_B_(map_B, static_cast<index_type_B>(index)),
          range_(index, index + compute_delta()),
          pos_(range_, pos_A_, pos_B_) {}

    // Advance to the next spot one of the two maps changes
    parallel_iterator &operator++() {
        const auto start = range_.end;         // we computed this the last time we set range
        const auto delta = range_.distance();  // we computed this the last time we set range
        assert(delta != 0);                    // Trying to increment past end

        pos_A_.offset(static_cast<index_type_A>(delta));
        pos_B_.offset(static_cast<index_type_B>(delta));

        range_ = key_type(start, start + compute_delta());  // find the next boundary (must be after offset)
        assert(pos_A_->index == start);
        assert(pos_B_->index == start);

        return *this;
    }

    // Seeks to a specific index in both maps reseting range.  Cannot guarantee range.begin is on edge boundary,
    /// but range.end will be.  Lower bound objects assumed to invalidate their cached lower bounds on seek.
    parallel_iterator &seek(const index_type &index) {
        pos_A_.seek(static_cast<index_type_A>(index));
        pos_B_.seek(static_cast<index_type_B>(index));
        range_ = key_type(index, index + compute_delta());
        assert(pos_A_->index == index);
        assert(pos_A_->index == pos_B_->index);
        return *this;
    }

    // Invalidates the lower_bound caches, reseting range.  Cannot guarantee range.begin is on edge boundary,
    // but range.end will be.
    parallel_iterator &invalidate() {
        const index_type start = range_.begin;
        seek(start);
        return *this;
    }

    parallel_iterator &invalidate_A() {
        const index_type index = range_.begin;
        pos_A_.invalidate(static_cast<index_type_A>(index));
        range_ = key_type(index, index + compute_delta());
        return *this;
    }

    parallel_iterator &invalidate_A(const iterator_A &hint) {
        const index_type index = range_.begin;
        pos_A_.invalidate(hint, static_cast<index_type_A>(index));
        range_ = key_type(index, index + compute_delta());
        return *this;
    }

    parallel_iterator &invalidate_B() {
        const index_type index = range_.begin;
        pos_B_.invalidate(static_cast<index_type_B>(index));
        range_ = key_type(index, index + compute_delta());
        return *this;
    }

    parallel_iterator &invalidate_B(const iterator_B &hint) {
        const index_type index = range_.begin;
        pos_B_.invalidate(hint, static_cast<index_type_B>(index));
        range_ = key_type(index, index + compute_delta());
        return *this;
    }

    parallel_iterator &trim_A() {
        if (pos_A_->valid && (range_ != pos_A_->lower_bound->first)) {
            split(pos_A_->lower_bound, pos_A_.map(), range_);
            invalidate_A();
        }
        return *this;
    }

    // The return is const because we are sharing the internal state directly.
    const value_type &operator*() const { return pos_; }
    const value_type *operator->() const { return &pos_; }
};

template <typename DstRangeMap, typename SrcRangeMap, typename Updater,
          typename SourceIterator = typename SrcRangeMap::const_iterator>
bool splice(DstRangeMap &to, const SrcRangeMap &from, SourceIterator begin, SourceIterator end, const Updater &updater) {
    if (from.empty() || (begin == end) || (begin == from.cend())) return false;  // nothing to merge.

    using ParallelIterator = parallel_iterator<DstRangeMap, const SrcRangeMap>;
    using Key = typename SrcRangeMap::key_type;
    using CachedLowerBound = cached_lower_bound_impl<DstRangeMap>;
    using ConstCachedLowerBound = cached_lower_bound_impl<const SrcRangeMap>;
    ParallelIterator par_it(to, from, begin->first.begin);
    bool updated = false;
    while (par_it->range.non_empty() && par_it->pos_B->lower_bound != end) {
        const Key &range = par_it->range;
        const CachedLowerBound &to_lb = par_it->pos_A;
        const ConstCachedLowerBound &from_lb = par_it->pos_B;
        if (from_lb->valid) {
            auto read_it = from_lb->lower_bound;
            auto write_it = to_lb->lower_bound;
            // Because of how the parallel iterator walk, "to" is valid over the whole range or it isn't (ranges don't span
            // transitions between map entries or between valid and invalid ranges)
            if (to_lb->valid) {
                if (write_it->first == range) {
                    // if the source and destination ranges match we can overwrite everything
                    updated |= updater.update(write_it->second, read_it->second);
                } else {
                    // otherwise we need to split the destination range.
                    auto value_to_update = write_it->second;  // intentional copy
                    updated |= updater.update(value_to_update, read_it->second);
                    auto intersected_range = write_it->first & range;
                    to.overwrite_range(to_lb->lower_bound, std::make_pair(intersected_range, value_to_update));
                    par_it.invalidate_A();  // we've changed map 'to' behind to_lb's back... let it know.
                }
            } else {
                // Insert into the gap.
                auto opt = updater.insert(read_it->second);
                if (opt) {
                    to.insert(write_it, std::make_pair(range, std::move(*opt)));
                    updated = true;
                    par_it.invalidate_A();  // we've changed map 'to' behind to_lb's back... let it know.
                }
            }
        }
        ++par_it;  // next range over which both 'to' and 'from' stay constant
    }
    return updated;
}
// And short hand for "from begin to end"
template <typename DstRangeMap, typename SrcRangeMap, typename Updater>
bool splice(DstRangeMap &to, const SrcRangeMap &from, const Updater &updater) {
    return splice(to, from, from.cbegin(), from.cend(), updater);
}

template <typename T>
struct update_prefer_source {
    bool update(T &dst, const T &src) const {
        if (dst != src) {
            dst = src;
            return true;
        }
        return false;
    }

    std::optional<T> insert(const T &src) const { return std::optional<T>(std::in_place, src); }
};

template <typename T>
struct update_prefer_dest {
    bool update([[maybe_unused]] T &dst, [[maybe_unused]] const T &src) const { return false; }

    std::optional<T> insert(const T &src) const { return std::optional<T>(std::in_place, src); }
};

template <typename RangeMap, typename SourceIterator = typename RangeMap::const_iterator>
bool splice(RangeMap &to, const RangeMap &from, value_precedence arbiter, [[maybe_unused]] SourceIterator begin,
            [[maybe_unused]] SourceIterator end) {
    if (arbiter == value_precedence::prefer_source) {
        return splice(to, from, from.cbegin(), from.cend(), update_prefer_source<typename RangeMap::mapped_type>());
    } else {
        return splice(to, from, from.cbegin(), from.cend(), update_prefer_dest<typename RangeMap::mapped_type>());
    }
}

// And short hand for "from begin to end"
template <typename RangeMap>
bool splice(RangeMap &to, const RangeMap &from, value_precedence arbiter) {
    return splice(to, from, arbiter, from.cbegin(), from.cend());
}

template <typename Map, typename Range = typename Map::key_type, typename MapValue = typename Map::mapped_type>
bool update_range_value(Map &map, const Range &range, MapValue &&value, value_precedence precedence) {
    using CachedLowerBound = typename vku::sparse::cached_lower_bound_impl<Map>;
    CachedLowerBound pos(map, range.begin);

    bool updated = false;
    while (range.includes(pos->index)) {
        if (!pos->valid) {
            if (precedence == value_precedence::prefer_source) {
                // We can convert this into and overwrite...
                map.overwrite_range(pos->lower_bound, std::make_pair(range, std::forward<MapValue>(value)));
                return true;
            }
            // Fill in the leading space (or in the case of pos at end the trailing space
            const auto start = pos->index;
            auto it = pos->lower_bound;
            const auto limit = (it != map.end()) ? std::min(it->first.begin, range.end) : range.end;
            map.insert(it, std::make_pair(Range(start, limit), value));
            // We inserted before pos->lower_bound, so pos->lower_bound isn't invalid, but the associated index *is* and seek
            // will fix this (and move the state to valid)
            pos.seek(limit);
            updated = true;
        }
        // Note that after the "fill" operation pos may have become valid so we check again
        if (pos->valid) {
            if ((precedence == value_precedence::prefer_source) && (pos->lower_bound->second != value)) {
                // We've found a place where we're changing the value, at this point might as well simply over write the range
                // and be done with it. (save on later merge operations....)
                pos.seek(range.begin);
                map.overwrite_range(pos->lower_bound, std::make_pair(range, std::forward<MapValue>(value)));
                return true;

            } else {
                // "prefer_dest" means don't overwrite existing values, so we'll skip this interval.
                // Point just past the end of this section,  if it's within the given range, it will get filled next iteration
                // ++pos could move us past the end of range (which would exit the loop) so we don't use it.
                pos.seek(pos->lower_bound->first.end);
            }
        }
    }
    return updated;
}

//  combines directly adjacent ranges with equal RangeMap::mapped_type .
template <typename RangeMap>
void consolidate(RangeMap &map) {
    using Value = typename RangeMap::value_type;
    using Key = typename RangeMap::key_type;
    using It = typename RangeMap::iterator;

    It current = map.begin();
    const It map_end = map.end();

    // To be included in a merge range there must be no gap in the Key space, and the mapped_type values must match
    auto can_merge = [](const It &last, const It &cur) {
        return cur->first.begin == last->first.end && cur->second == last->second;
    };

    while (current != map_end) {
        // Establish a trival merge range at the current location, advancing current. Merge range is inclusive of merge_last
        const It merge_first = current;
        It merge_last = current;
        ++current;

        // Expand the merge range as much as possible
        while (current != map_end && can_merge(merge_last, current)) {
            merge_last = current;
            ++current;
        }

        // Current isn't in the active merge range. If there is a non-trivial merge range, we resolve it here.
        if (merge_first != merge_last) {
            // IFF there is more than one range in (merge_first, merge_last)  <- again noting the *inclusive* last
            // Create a new Val spanning (first, last), substitute it for the multiple entries.
            Value merged_value = std::make_pair(Key(merge_first->first.begin, merge_last->first.end), merge_last->second);
            // Note that current points to merge_last + 1, and is valid even if at map_end for these operations
            map.erase(merge_first, current);
            map.insert(current, std::move(merged_value));
        }
    }
}

// Returns the intersection of the ranges [x, x + x_size) and [y, y + y_size)
static inline range<int64_t> GetRangeIntersection(int64_t x, uint64_t x_size, int64_t y, uint64_t y_size) {
    int64_t intersection_min = std::max(x, y);
    int64_t intersection_max = std::min(x + static_cast<int64_t>(x_size), y + static_cast<int64_t>(y_size));

    return {intersection_min, intersection_max};
}

}  // namespace sparse
}  // namespace vku
