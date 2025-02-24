/* Copyright (c) 2021-2024 The Khronos Group Inc.
 * Copyright (c) 2021-2024 Valve Corporation
 * Copyright (c) 2021-2024 LunarG, Inc.
 * Copyright (C) 2021-2022 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "error_location.h"

void Location::AppendFields(std::ostream& out) const {
    if (prev) {
        // When apply a .dot(sub_index) we duplicate the field item
        // Instead of dealing with partial non-const Location, just do the check here
        const Location& prev_loc = (prev->field == field && prev->index == kNoIndex && prev->prev) ? *prev->prev : *prev;

        // Work back and print for Locaiton first
        prev_loc.AppendFields(out);

        // check if need connector from last item
        if (prev_loc.structure != vvl::Struct::Empty || prev_loc.field != vvl::Field::Empty) {
            out << ((prev_loc.index == kNoIndex && IsFieldPointer(prev_loc.field)) ? "->" : ".");
        }
    }
    if (isPNext && structure != vvl::Struct::Empty) {
        out << "pNext<" << vvl::String(structure) << (field != vvl::Field::Empty ? ">." : ">");
    }
    if (field != vvl::Field::Empty) {
        out << vvl::String(field);
        if (index != kNoIndex) {
            out << "[" << index << "]";
        }
    }
}

std::string Location::Fields() const {
    std::stringstream out;
    AppendFields(out);
    return out.str();
}

std::string Location::Message() const {
    std::stringstream out;
    if (debug_region && !debug_region->empty()) {
        out << "[ Debug region: " << *debug_region << " ] ";
    }
    out << StringFunc() << "(): ";
    AppendFields(out);
    std::string message = out.str();
    // Remove space in the end when no fields are added
    if (message.back() == ' ') {
        message.pop_back();
    }
    return message;
}

namespace vvl {
LocationCapture::LocationCapture(const Location& loc) { Capture(loc, 1); }

LocationCapture::LocationCapture(const LocationCapture& other)
    : capture(other.capture) {
    if (capture.empty()) {
        return;
    }
    capture[0].prev = nullptr;
    for (CaptureStore::size_type i = 1; i < capture.size(); i++) {
        capture[i].prev = &capture[i - 1];
    }
}

LocationCapture::LocationCapture(LocationCapture&& other)
    : capture(std::move(other.capture)) {
    if (capture.empty()) {
        return;
    }
    capture[0].prev = nullptr;
    for (CaptureStore::size_type i = 1; i < capture.size(); i++) {
        capture[i].prev = &capture[i - 1];
    }
}

const Location* LocationCapture::Capture(const Location& loc, CaptureStore::size_type depth) {
    const Location* prev_capture = nullptr;
    if (loc.prev) {
        prev_capture = Capture(*loc.prev, depth + 1);
    } else {
        capture.reserve(depth);
    }

    capture.emplace_back(loc);
    capture.back().prev = prev_capture;
    return &(capture.back());
}

bool operator<(const Key& lhs, const Key& rhs) {
    if (lhs.function < rhs.function) {
        return true;
    } else if (lhs.function > rhs.function) {
        return false;
    }

    if (lhs.structure < rhs.structure) {
        return true;
    } else if (lhs.structure > rhs.structure) {
        return false;
    }

    if (lhs.field < rhs.field) {
        return true;
    } else if (lhs.field > rhs.field) {
        return false;
    }

    if (lhs.recurse_field < rhs.recurse_field) {
        return true;
    } else if (lhs.recurse_field > rhs.recurse_field) {
        return false;
    }

    return false;
}

bool operator==(const Key& lhs, const Key& rhs) {
    return lhs.function == rhs.function && lhs.structure == rhs.structure && lhs.field == rhs.field &&
           lhs.recurse_field == rhs.recurse_field;
}

bool operator==(const Key& key, const Location& loc) {
    assert(key.function != Func::Empty || key.structure != Struct::Empty);
    assert(loc.function != Func::Empty);
    if (key.function != Func::Empty) {
        if (key.function != loc.function) {
            return false;
        }
    }
    if (key.structure != Struct::Empty) {
        if (key.structure != loc.structure) {
            return false;
        }
    }
    if (key.field == Field::Empty) {
        return true;
    }
    if (key.field == loc.field) {
        return true;
    }
    if (key.recurse_field) {
        const Location *prev = loc.prev;
        while (prev != nullptr) {
            if (key.field == prev->field) {
                return true;
            }
            prev = prev->prev;
        }
    }
    return false;
}

}  // namespace vvl
