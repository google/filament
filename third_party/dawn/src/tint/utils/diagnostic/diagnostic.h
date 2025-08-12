// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_TINT_UTILS_DIAGNOSTIC_DIAGNOSTIC_H_
#define SRC_TINT_UTILS_DIAGNOSTIC_DIAGNOSTIC_H_

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/diagnostic/source.h"
#include "src/tint/utils/result.h"
#include "src/tint/utils/rtti/traits.h"
#include "src/tint/utils/text/styled_text.h"

namespace tint::diag {

/// Severity is an enumerator of diagnostic severities.
enum class Severity : uint8_t { Note, Warning, Error };

/// @return true iff `a` is more than, or of equal severity to `b`
inline bool operator>=(Severity a, Severity b) {
    return static_cast<int>(a) >= static_cast<int>(b);
}

/// Diagnostic holds all the information for a single compiler diagnostic
/// message.
class Diagnostic {
  public:
    /// Constructor
    Diagnostic();
    /// Copy constructor
    Diagnostic(const Diagnostic&);
    /// Destructor
    ~Diagnostic();

    /// Copy assignment operator
    /// @return this diagnostic
    Diagnostic& operator=(const Diagnostic&);

    /// Appends @p msg to the diagnostic's message
    template <typename T>
    Diagnostic& operator<<(T&& msg) {
        message << std::forward<T>(msg);
        return *this;
    }

    /// severity is the severity of the diagnostic message.
    Severity severity = Severity::Error;
    /// source is the location of the diagnostic.
    Source source;
    /// message is the text associated with the diagnostic.
    StyledText message;
    /// A shared pointer to a Source::File. Only used if the diagnostic Source
    /// points to a file that was created specifically for this diagnostic
    /// (usually an ICE).
    std::shared_ptr<Source::File> owned_file = nullptr;
};

/// List is a container of Diagnostic messages.
class List {
  public:
    /// The iterator type for this List
    using iterator = VectorIterator<const Diagnostic>;

    /// Constructs the list with no elements.
    List();

    /// Constructor. Copies the diagnostics from @p list into this list.
    /// @param list the list of diagnostics to copy into this list.
    List(std::initializer_list<Diagnostic> list);

    /// Constructor. Copies the diagnostics from @p list into this list.
    /// @param list the list of diagnostics to copy into this list.
    explicit List(VectorRef<Diagnostic> list);

    /// Copy constructor. Copies the diagnostics from @p list into this list.
    /// @param list the list of diagnostics to copy into this list.
    List(const List& list);

    /// Move constructor. Moves the diagnostics from @p list into this list.
    /// @param list the list of diagnostics to move into this list.
    List(List&& list);

    /// Destructor
    ~List();

    /// Assignment operator. Copies the diagnostics from @p list into this list.
    /// @param list the list to copy into this list.
    /// @return this list.
    List& operator=(const List& list);

    /// Assignment move operator. Moves the diagnostics from @p list into this list.
    /// @param list the list to move into this list.
    /// @return this list.
    List& operator=(List&& list);

    /// Adds a diagnostic to the end of this list.
    /// @param diag the diagnostic to append to this list.
    /// @returns a reference to the new diagnostic.
    /// @note The returned reference must not be used after the list is mutated again.
    diag::Diagnostic& Add(const Diagnostic& diag) {
        if (diag.severity >= Severity::Error) {
            error_count_++;
        }
        entries_.Push(diag);
        return entries_.Back();
    }

    /// Adds a diagnostic to the end of this list.
    /// @param diag the diagnostic to append to this list.
    /// @returns a reference to the new diagnostic.
    /// @note The returned reference must not be used after the list is mutated again.
    diag::Diagnostic& Add(Diagnostic&& diag) {
        if (diag.severity >= Severity::Error) {
            error_count_++;
        }
        entries_.Push(std::move(diag));
        return entries_.Back();
    }

    /// Adds a list of diagnostics to the end of this list.
    /// @param list the diagnostic to append to this list.
    void Add(const List& list) {
        for (auto diag : list) {
            Add(std::move(diag));
        }
    }

    /// Adds the note message with the given Source to the end of this list.
    /// @param source the source of the note diagnostic
    /// @returns a reference to the new diagnostic.
    /// @note The returned reference must not be used after the list is mutated again.
    diag::Diagnostic& AddNote(const Source& source) {
        diag::Diagnostic note{};
        note.severity = diag::Severity::Note;
        note.source = source;
        return Add(std::move(note));
    }

    /// Adds the warning message with the given Source to the end of this list.
    /// @param source the source of the warning diagnostic
    /// @returns a reference to the new diagnostic.
    /// @note The returned reference must not be used after the list is mutated again.
    diag::Diagnostic& AddWarning(const Source& source) {
        diag::Diagnostic warning{};
        warning.severity = diag::Severity::Warning;
        warning.source = source;
        return Add(std::move(warning));
    }

    /// Adds the error message with the given Source to the end of this list.
    /// @param source the source of the error diagnostic
    /// @returns a reference to the new diagnostic.
    /// @note The returned reference must not be used after the list is mutated again.
    diag::Diagnostic& AddError(const Source& source) {
        diag::Diagnostic error{};
        error.severity = diag::Severity::Error;
        error.source = source;
        return Add(std::move(error));
    }

    /// Ensures that the diagnostic list can fit an additional @p count diagnostics without
    /// resizing. This is useful for ensuring that a reference returned by the AddX() methods is not
    /// invalidated after another Add().
    void ReserveAdditional(size_t count) { entries_.Reserve(entries_.Length() + count); }

    /// @returns true iff the diagnostic list contains errors diagnostics (or of
    /// higher severity).
    bool ContainsErrors() const { return error_count_ > 0; }
    /// @returns the number of error diagnostics (or of higher severity).
    size_t NumErrors() const { return error_count_; }
    /// @returns the number of entries in the list.
    size_t Count() const { return entries_.Length(); }
    /// @returns true if the diagnostics list is empty
    bool IsEmpty() const { return entries_.IsEmpty(); }

    /// @returns a formatted string of all the diagnostics in this list.
    std::string Str() const;

    ////////////////////////////////////////////////////////////////////////////
    /// STL-interface support
    ////////////////////////////////////////////////////////////////////////////
    /// @returns true if the diagnostics list is empty
    bool empty() const { return entries_.IsEmpty(); }
    /// @returns the number of entries in the list.
    size_t size() const { return entries_.Length(); }
    /// @returns the first diagnostic in the list.
    iterator begin() const { return entries_.begin(); }
    /// @returns the last diagnostic in the list.
    iterator end() const { return entries_.end(); }

  private:
    Vector<Diagnostic, 0> entries_;
    size_t error_count_ = 0;
};

/// The default Result error type.
struct Failure {
    /// Constructor with no diagnostics
    Failure();

    /// Constructor with a single diagnostic
    /// @param err the single error diagnostic
    explicit Failure(std::string_view err);

    /// Constructor with a single diagnostic
    /// @param diagnostic the failure diagnostic
    explicit Failure(diag::Diagnostic diagnostic);

    /// Constructor with a list of diagnostics
    /// @param diagnostics the failure diagnostics
    explicit Failure(diag::List diagnostics);

    /// The diagnostics explaining the failure reason
    diag::List reason;
};

template <typename SUCCESS_TYPE>
using Result = ::tint::Result<SUCCESS_TYPE, diag::Failure>;

/// Write the Failure to the given stream
/// @param out the output stream
/// @param failure the Failure
/// @returns the output stream
template <typename STREAM>
    requires(traits::IsOStream<STREAM>)
auto& operator<<(STREAM& out, const Failure& failure) {
    return out << failure.reason.Str();
}

/// Write the diagnostic list to the given stream
/// @param out the output stream
/// @param list the list to emit
/// @returns the output stream
template <typename STREAM>
    requires(traits::IsOStream<STREAM>)
auto& operator<<(STREAM& out, const List& list) {
    return out << list.Str();
}

}  // namespace tint::diag

#endif  // SRC_TINT_UTILS_DIAGNOSTIC_DIAGNOSTIC_H_
