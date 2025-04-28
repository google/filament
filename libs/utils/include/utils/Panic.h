/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_UTILS_PANIC_H
#define TNT_UTILS_PANIC_H

#ifdef FILAMENT_PANIC_USES_ABSL
#   if FILAMENT_PANIC_USES_ABSL
#       include "absl/log/log.h"
#       define  FILAMENT_CHECK_PRECONDITION      CHECK
#       define  FILAMENT_CHECK_POSTCONDITION     CHECK
#       define  FILAMENT_CHECK_ARITHMETIC        CHECK
#   endif
#endif

#include <utils/CallStack.h>
#include <utils/compiler.h>
#include <utils/sstream.h>

#include <string>
#include <string_view>

#ifdef __EXCEPTIONS
#   define UTILS_EXCEPTIONS 1
#else
#   ifdef UTILS_EXCEPTIONS
#       error  UTILS_EXCEPTIONS is already defined!
#   endif
#endif

/**
 * @defgroup errors Handling Catastrophic Failures (Panics)
 *
 * @brief Failure detection and reporting facilities
 *
 * ## What's a Panic? ##
 *
 * In the context of this document, a _panic_ is a type of error due to a _contract violation_,
 * it shouldn't be confused with a _result_ or _status_ code. The POSIX API for instance,
 * unfortunately often conflates the two.
 * @see <http://en.wikipedia.org/wiki/Design_by_contract>
 *
 *
 * Here we give the following definition of a _panic_:
 *
 *   1. Failures to meet a function's own **postconditions**\n
 *      The function cannot establish one of its own postconditions, such as (but not limited to)
 *      producing a valid return value object.
 *
 *      Often these failures are only detectable at runtime, for instance they can be caused by
 *      arithmetic errors, as it was the case for the Ariane 5 rocket. Ariane 5 crashed because it
 *      reused an inertial module from Ariane 4, which didn't account for the greater horizontal
 *      acceleration of Ariane 5 and caused an overflow in the computations. Ariane 4's module
 *      wasn't per-say buggy, but was improperly used and failed to meet, obviously, certain
 *      postconditions.
 *      @see <http://en.wikipedia.org/wiki/Cluster_(spacecraft)>
 *
 *   2. Failures to meet the **preconditions** of any of a function's callees\n
 *      The function cannot meet a precondition of another function it must call, such as a
 *      restriction on a parameter.
 *
 *      Not to be confused with the case where the preconditions of a function are already
 *      violated upon entry, which indicates a programming error from the caller.
 *
 *      Typically these failures can be avoided and arise because of programming errors.
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * ## Failure reporting vs. handling ##
 *
 * Very often when a panic, as defined above, is detected, the program has little other choice
 * but to terminate.\n
 * Typically these situations can be handled by _assert()_. However, _assert()_ also conflates two
 * very different concepts: detecting and handling failures.\n
 * The place where a failure is detected is rarely the place where there is enough
 * context to decide what to do. _assert()_ terminates the program which, may or may not be
 * appropriate. At the very least the failure must be logged (which _assert()_ does in a crude way),
 * but some other actions might need to happen, such as:\n
 *
 *   - logging the failure in the system-wide logger
 *   - providing enough information in development builds to analyze/debug the problem
 *   - cleanly releasing some resources, such as communication channels with other processes\n
 *     e.g.: to avoid their pre- or postconditions from being violated as well.
 *
 * In some _rare_ cases, the failure might even be ignored altogether because it doesn't matter in
 * the context where it happened. This decision clearly doesn't always lie at the failure-site.
 *
 * It is therefore important to separate failure detection from handling.
 *
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * ## Failure detection and handling facilities ##
 *
 * Clearly, catastrophic failures should be **rare**; in fact they should
 * never happen, except possibly for "failures to meet a function's own postconditions", which
 * may depend on external factors and should still be very rare. Yet, when a failure happens, it
 * must be detected and handled appropriately.\n
 * Since panics are rare, it is desirable that the handling mechanism be as unobtrusive
 * as possible, without allowing such failures to go unnoticed or swallowed by mistake. Ideally, the
 * programmer using an API should have nothing special to do to handle that API's failure
 * conditions.\n\n
 *
 * An important feature of the Panic Handling facilities here is that **panics are not part of
 * the API of a function or method**\n\n
 *
 *
 * The panic handling facility has the following benefits:
 *   - provides an easy way to detect and report failure
 *   - separates failure detection from handling
 *   - makes it hard for detected failures to be ignored (i.e.: not handled)
 *   - doesn't add burden on the API design
 *   - doesn't add overhead (visual or otherwise) at call sites
 *   - has very little performance overhead for failure detection
 *   - has little to no performance impact for failure handling in the common (success) case
 *   - is flexible and extensible
 *
 * Since we have established that failures are **rare**, **exceptional** situations, it would be
 * appropriate to handle them with an _assert_ mechanism and that's what the API below
 * provides. However, under-the-hood it uses C++ exceptions as a means to separate
 * _reporting_ from _handling_.
 *
 *   \note On devices where exceptions are not supported or appropriate, these APIs can be turned
 *   into a regular _std::terminate()_.
 *
 *
 *          ASSERT_PRECONDITION(condition, format, ...)
 *          ASSERT_POSTCONDITION(condition, format, ...)
 *          ASSERT_ARITHMETIC(condition, format, ...)
 *          ASSERT_DESTRUCTOR(condition, format, ...)
 *
 *
 * @see ASSERT_PRECONDITION, ASSERT_POSTCONDITION, ASSERT_ARITHMETIC
 * @see ASSERT_DESTRUCTOR
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * ## Writing code that can assert ##
 *
 * Because we've separated failure reporting from failure handling, there are some considerations
 * that need to be thought about when writing code that calls the macros above (i.e.: the program
 * won't terminate at the point where the failure is detected).\n\n
 *
 *
 * ### Panic guarantees ###
 *
 * After the failure condition is reported by a function, additional guarantees may be provided
 * with regards to the state of the program. The following four levels of guarantee are
 * generally recognized, each of which is a strict superset of its successors:
 *
 *  1. Nothrow exception guarantee\n
 *     The function never asserts. e.g.: This should always be the case with destructors.\n\n
 *
 *  2. Strong exception guarantee\n
 *     If the function asserts, the state of the program is rolled back to the state just before
 *     the function call.\n\n
 *
 *  3. Basic exception guarantee\n
 *     If the function asserts, the program is in a valid state. It may require cleanup,
 *     but all invariants are intact.\n\n
 *
 *  4. No exception guarantee\n
 *     If the function asserts, the program may not be in a valid state: resource leaks, memory
 *     corruption, or other invariant-destroying failures may have occurred.
 *
 * In each function, give the **strongest** safety guarantee that won't penalize callers who
 * don't need it, but **always give at least the basic guarantee**. The RAII (Resource
 * Acquisition Is Initialization) pattern can help with achieving these guarantees.
 *
 * @see [RAII](http://en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization)
 *
 * ### Special considerations for Constructors ###
 *
 * Constructors are a bit special because if a failure occurs during their execution, the
 * destructor won't be called (how could it? since the object wasn't constructed!). This can lead
 * to leaked resources allocated in the constructor prior to the failure. Thankfully there is
 * a nice C++ syntax to handle this case:
 *
 * @code
 *          Foo::Foo(size_t s) try : m_size(s), m_buf(new uint32_t[s]) {
 *              ASSERT_POSTCONDITION(s&0xF==0,
 *                  "object size is %u, but must be multiple of 16", s);
 *          } catch (...) {
 *              delete [] m_buf;
 *              // the exception will be automatically re-thrown
 *          }
 * @endcode
 *
 * Unfortunately, this usage leaks the underlying, exception-based, implementation of the
 * panic handling macros. For this reason, it is best to keep constructors simple and guarantee
 * they can't fail. An _init()_ function with a factory can be used for actual initialization.
 *
 *
 * ### Special considerations for Destructors ###
 *
 * In C++ destructors cannot throw exceptions and since the above macros internally use exceptions
 * they cannot be used in destructors. Doing so will result in immediate termination of the
 * program by _std::terminate()_.\n
 * It is therefore best to always guarantee that destructors won't fail. In case of such a
 * failure in a destructor the ASSERT_DESTRUCTOR() macro can be used instead, it
 * will log the failure but won't terminate the program, instead it'll proceed as if nothing
 * happened. Generally this will result in some resource leak which, eventually, will cause
 * another failure (typically a postcondition violation).\n\n
 *
 * Rationale for this behavior: There are fundamentally no way to report a failure from a
 * destructor in C++, violently terminating the process is inadequate because it again conflates
 * failure reporting and failure handling; for instance a failure in glDeleteTextures() shouldn't
 * be necessarily fatal (certainly not without saving the user's data first). The alternative
 * would be for the caller to swallow the failure entirely, but that's not great either because the
 * failure would go unnoticed. The solution retained here is a compromise.
 *
 * @see ASSERT_DESTRUCTOR
 *
 * ### Testing Code that Uses Panics ###
 *
 * Since panics use exceptions for their underlying implementation, you can test code that uses
 * panics with EXPECT_THROW by doing the following things:
 * \li Set panic mode to THROW (default is TERMINATE)
 * \li Pass Panic to EXPECT_THROW as the exception type
 *
 * Example code for your test file:
 *
 * @code
 * #include <MyClass.hpp>  // since your code uses panics, this should include utils/Panic.hpp
 *
 * using utils::Panic;
 *
 * TEST(MyClassTest, value_that_causes_panic) {
 *     EXPECT_THROW(MyClass::function(value_that_causes_panic), Panic);
 * }
 *
 * // ... other tests ...
 *
 * int main(int argc, char** argv) {
 *     ::testing::InitGoogleTest(&argc, argv);
 *     Panic::setMode(Panic::Mode::THROW);
 *     return RUN_ALL_TESTS();
 * }
 * @endcode
 *
 */

namespace utils {

// -----------------------------------------------------------------------------------------------

/**
 * @ingroup errors
 *
 * \brief Base class of all exceptions thrown by all the ASSERT macros
 *
 * The Panic class provides the std::exception protocol, it is the base exception object
 * used for all thrown exceptions.
 */
class UTILS_PUBLIC Panic {
public:

    using PanicHandlerCallback = void(*)(void* user, Panic const& panic);

    /**
     * Sets a user-defined handler for the Panic. If exceptions are enabled, the concrete Panic
     * object will be thrown upon return; moreover it is acceptable to throw from the provided
     * callback, but it is unsafe to throw the Panic object itself, since it's just an interface.
     * It is also acceptable to abort from the callback. If exceptions are not enabled, std::abort()
     * will be automatically called upon return.
     *
     * The PanicHandlerCallback can be called from any thread.
     *
     * Caveat: this API can misbehave if is used as a static library in multiple translation units,
     * some of these translation units might not see the callback.
     *
     * @param handler pointer to the user defined handler for the Panic
     * @param user  user pointer given back to the callback
     */
    static void setPanicHandler(PanicHandlerCallback handler, void* user) noexcept;


    virtual ~Panic() noexcept;

    /**
     * @return a formatted and detailed description of the error including all available
     *         information.
     * @see std::exception
     */
    virtual const char* what() const noexcept = 0;

    /**
     * Get the type of the panic (e.g. "Precondition")
     * @return a C string containing the type of panic
     */
    virtual const char* getType() const noexcept = 0;

    /**
     * Get the reason string for the panic
     * @return a C string containing the reason for the panic
     */
    virtual const char* getReason() const noexcept = 0;

    /**
     * Get a version of the reason string that is guaranteed to be constructed from literal
     * strings only; it will contain no runtime information.
     */
    virtual const char* getReasonLiteral() const noexcept = 0;

    /**
     * Get the function name where the panic was detected. On debug build the fully qualified
     * function name is returned; on release builds only the function name is.
     * @return a C string containing the function name where the panic was detected
     */
    virtual const char* getFunction() const noexcept = 0;

    /**
     * Get the file name where the panic was detected. Only available on debug builds.
     * @return a C string containing the file name where the panic was detected
     */
    virtual const char* getFile() const noexcept = 0;

    /**
     * Get the line number in the file where the panic was detected. Only available on debug builds.
     * @return an integer containing the line number in the file where the panic was detected
     */
    virtual int getLine() const noexcept = 0;

    /**
     * Get the CallStack when the panic was detected if available.
     * @return the CallStack when the panic was detected
     */
    virtual const CallStack& getCallStack() const noexcept = 0;

    /**
     * Logs this exception to the system-log
     */
    virtual void log() const noexcept = 0;
};

// -----------------------------------------------------------------------------------------------

/**
 * @ingroup errors
 *
 * \brief Concrete implementation of the Panic interface.
 *
 * The TPanic<> class implements the std::exception protocol as well as the Panic
 * interface common to all exceptions thrown by the framework.
 */
template <typename T>
class UTILS_PUBLIC TPanic : public Panic {
public:
    // std::exception protocol
    const char* what() const noexcept override;

    // Panic interface
    const char* getType() const noexcept override;
    const char* getReason() const noexcept override;
    const char* getReasonLiteral() const noexcept override;
    const char* getFunction() const noexcept override;
    const char* getFile() const noexcept override;
    int getLine() const noexcept override;
    const CallStack& getCallStack() const noexcept override;
    void log() const noexcept override;

    /**
     * Depending on the mode set, either throws an exception of type T with the given reason plus
     * extra information about the error-site, or logs the error and calls std::terminate().
     * This function never returns.
     * @param function the name of the function where the error was detected
     * @param file the file where the above function in implemented
     * @param line the line in the above file where the error was detected
     * @param literal a literal version of the error message
     * @param format printf style string describing the error
     * @see ASSERT_PRECONDITION, ASSERT_POSTCONDITION, ASSERT_ARITHMETIC
     * @see PANIC_PRECONDITION, PANIC_POSTCONDITION, PANIC_ARITHMETIC
     * @see setMode()
     */
    static void panic(char const* function, char const* file, int line, char const* literal,
            const char* format, ...) UTILS_NORETURN;

    /**
     * Depending on the mode set, either throws an exception of type T with the given reason plus
     * extra information about the error-site, or logs the error and calls std::terminate().
     * This function never returns.
     * @param function the name of the function where the error was detected
     * @param file the file where the above function in implemented
     * @param line the line in the above file where the error was detected
     * @param literal a literal version of the error message
     * @param reason std::string describing the error
     * @see ASSERT_PRECONDITION, ASSERT_POSTCONDITION, ASSERT_ARITHMETIC
     * @see PANIC_PRECONDITION, PANIC_POSTCONDITION, PANIC_ARITHMETIC
     * @see setMode()
     */
    static inline void panic(
            char const* function, char const* file, int line, char const* literal,
            std::string reason) UTILS_NORETURN;

protected:

    /**
     * Creates a Panic with extra information about the error-site.
     * @param function the name of the function where the error was detected
     * @param file the file where the above function in implemented
     * @param line the line in the above file where the error was detected
     * @param literal a literal version of the error message
     * @param reason a description of the cause of the error
     */
    TPanic(char const* function, char const* file, int line, char const* literal,
            std::string reason);

    ~TPanic() override;

private:
    void buildMessage();

    char const* const mFile = nullptr;      // file where the panic happened
    char const* const mFunction = nullptr;  // function where the panic happened
    int const mLine = -1;                   // line where the panic happened
    std::string mLiteral;                   // reason for the panic, built only from literals
    std::string mReason;                    // reason for the panic
    mutable std::string mWhat;              // fully formatted reason
    CallStack mCallstack;
};

namespace details {
// these are private, don't use
void panicLog(
        char const* function, char const* file, int line, const char* format, ...) noexcept;
}  // namespace details

// -----------------------------------------------------------------------------------------------

/**
 * @ingroup errors
 *
 * ASSERT_PRECONDITION uses this Panic to report a precondition failure.
 * @see ASSERT_PRECONDITION
 */
class UTILS_PUBLIC PreconditionPanic : public TPanic<PreconditionPanic> {
    // Programming error, can be avoided
    // e.g.: invalid arguments
    using TPanic<PreconditionPanic>::TPanic;
    friend class TPanic<PreconditionPanic>;
    constexpr static auto type = "Precondition";
};

/**
 * @ingroup errors
 *
 * ASSERT_POSTCONDITION uses this Panic to report a postcondition failure.
 * @see ASSERT_POSTCONDITION
 */
class UTILS_PUBLIC PostconditionPanic : public TPanic<PostconditionPanic> {
    // Usually only detectable at runtime
    // e.g.: dead-lock would occur, arithmetic errors
    using TPanic<PostconditionPanic>::TPanic;
    friend class TPanic<PostconditionPanic>;
    constexpr static auto type = "Postcondition";
};

/**
 * @ingroup errors
 *
 * ASSERT_ARITHMETIC uses this Panic to report an arithmetic (postcondition) failure.
 * @see ASSERT_ARITHMETIC
 */
class UTILS_PUBLIC ArithmeticPanic : public TPanic<ArithmeticPanic> {
    // A common case of post-condition error
    // e.g.: underflow, overflow, internal computations errors
    using TPanic<ArithmeticPanic>::TPanic;
    friend class TPanic<ArithmeticPanic>;
    constexpr static auto type = "Arithmetic";
};

namespace details {

struct Voidify final {
    template<typename T>
    void operator&&(const T&) const&& {}
};

class UTILS_PUBLIC PanicStream {
public:
    PanicStream(
            char const* function,
            char const* file,
            int line,
            char const* message) noexcept;

    ~PanicStream();

    PanicStream& operator<<(short value) noexcept;
    PanicStream& operator<<(unsigned short value) noexcept;

    PanicStream& operator<<(char value) noexcept;
    PanicStream& operator<<(unsigned char value) noexcept;

    PanicStream& operator<<(int value) noexcept;
    PanicStream& operator<<(unsigned int value) noexcept;

    PanicStream& operator<<(long value) noexcept;
    PanicStream& operator<<(unsigned long value) noexcept;

    PanicStream& operator<<(long long value) noexcept;
    PanicStream& operator<<(unsigned long long value) noexcept;

    PanicStream& operator<<(float value) noexcept;
    PanicStream& operator<<(double value) noexcept;
    PanicStream& operator<<(long double value) noexcept;

    PanicStream& operator<<(bool value) noexcept;

    PanicStream& operator<<(const void* value) noexcept;

    PanicStream& operator<<(const char* string) noexcept;
    PanicStream& operator<<(const unsigned char* string) noexcept;

    PanicStream& operator<<(std::string const& s) noexcept;
    PanicStream& operator<<(std::string_view const& s) noexcept;

protected:
    io::sstream mStream;
    char const* mFunction;
    char const* mFile;
    int mLine;
    char const* mLiteral;
};

template<typename T>
class TPanicStream : public PanicStream {
public:
    using PanicStream::PanicStream;
    ~TPanicStream() noexcept(false) UTILS_NORETURN {
        T::panic(mFunction, mFile, mLine, mLiteral, mStream.c_str());
    }
};

} // namespace details

// -----------------------------------------------------------------------------------------------
}  // namespace utils

#ifndef NDEBUG
#   define PANIC_FILE(F) (F)
#   define PANIC_FUNCTION __PRETTY_FUNCTION__
#else
#   define PANIC_FILE(F) ""
#   define PANIC_FUNCTION __func__
#endif


#define FILAMENT_CHECK_CONDITION_IMPL(cond)                                                        \
    switch (0)                                                                                     \
    case 0:                                                                                        \
    default:                                                                                       \
        UTILS_VERY_LIKELY(cond) ? (void)0 : ::utils::details::Voidify()&&

#define FILAMENT_PANIC_IMPL(message, TYPE)                                                         \
        ::utils::details::TPanicStream<::utils::TYPE>(PANIC_FUNCTION, PANIC_FILE(__FILE__), __LINE__, message)

#ifndef FILAMENT_CHECK_PRECONDITION
#define FILAMENT_CHECK_PRECONDITION(condition)                                                     \
    FILAMENT_CHECK_CONDITION_IMPL(condition)  FILAMENT_PANIC_IMPL(#condition, PreconditionPanic)
#endif

#ifndef FILAMENT_CHECK_POSTCONDITION
#define FILAMENT_CHECK_POSTCONDITION(condition)                                                    \
    FILAMENT_CHECK_CONDITION_IMPL(condition)  FILAMENT_PANIC_IMPL(#condition, PostconditionPanic)
#endif

#ifndef FILAMENT_CHECK_ARITHMETIC
#define FILAMENT_CHECK_ARITHMETIC(condition)                                                       \
    FILAMENT_CHECK_CONDITION_IMPL(condition)  FILAMENT_PANIC_IMPL(#condition, ArithmeticPanic)
#endif

#define PANIC_PRECONDITION_IMPL(cond, format, ...)                                                 \
    ::utils::PreconditionPanic::panic(PANIC_FUNCTION,                                              \
            PANIC_FILE(__FILE__), __LINE__, #cond, format, ##__VA_ARGS__)

#define PANIC_POSTCONDITION_IMPL(cond, format, ...)                                                \
    ::utils::PostconditionPanic::panic(PANIC_FUNCTION,                                             \
            PANIC_FILE(__FILE__), __LINE__, #cond, format, ##__VA_ARGS__)

#define PANIC_ARITHMETIC_IMPL(cond, format, ...)                                                   \
    ::utils::ArithmeticPanic::panic(PANIC_FUNCTION,                                                \
            PANIC_FILE(__FILE__), __LINE__, #cond, format, ##__VA_ARGS__)

#define PANIC_LOG_IMPL(cond, format, ...)                                                          \
    ::utils::details::panicLog(PANIC_FUNCTION,                                                     \
            PANIC_FILE(__FILE__), __LINE__, format, ##__VA_ARGS__)

/**
 * PANIC_PRECONDITION is a macro that reports a PreconditionPanic
 * @param format printf-style string describing the error in more details
 */
#define PANIC_PRECONDITION(format, ...)     PANIC_PRECONDITION_IMPL(format, format, ##__VA_ARGS__)

/**
 * PANIC_POSTCONDITION is a macro that reports a PostconditionPanic
 * @param format printf-style string describing the error in more details
 */
#define PANIC_POSTCONDITION(format, ...)    PANIC_POSTCONDITION_IMPL(format, format, ##__VA_ARGS__)

/**
 * PANIC_ARITHMETIC is a macro that reports a ArithmeticPanic
 * @param format printf-style string describing the error in more details
 */
#define PANIC_ARITHMETIC(format, ...)       PANIC_ARITHMETIC_IMPL(format, format, ##__VA_ARGS__)

/**
 * PANIC_LOG is a macro that logs a Panic, and continues as usual.
 * @param format printf-style string describing the error in more details
 */
#define PANIC_LOG(format, ...)              PANIC_LOG_IMPL(format, format, ##__VA_ARGS__)

/**
 * @ingroup errors
 *
 * ASSERT_PRECONDITION is a macro that checks the given condition and reports a PreconditionPanic
 * if it evaluates to false.
 * @param cond a boolean expression
 * @param format printf-style string describing the error in more details
 */
#define ASSERT_PRECONDITION(cond, format, ...)                                                     \
    (!UTILS_VERY_LIKELY(cond) ? PANIC_PRECONDITION_IMPL(cond, format, ##__VA_ARGS__) : (void)0)

#if defined(UTILS_EXCEPTIONS) || !defined(NDEBUG)
#define ASSERT_PRECONDITION_NON_FATAL(cond, format, ...)                                           \
    (!UTILS_VERY_LIKELY(cond) ? PANIC_PRECONDITION_IMPL(cond, format, ##__VA_ARGS__), false : true)
#else
#define ASSERT_PRECONDITION_NON_FATAL(cond, format, ...)                                           \
    (!UTILS_VERY_LIKELY(cond) ? PANIC_LOG_IMPL(cond, format, ##__VA_ARGS__), false : true)
#endif


/**
 * @ingroup errors
 *
 * ASSERT_POSTCONDITION is a macro that checks the given condition and reports a PostconditionPanic
 * if it evaluates to false.
 * @param cond a boolean expression
 * @param format printf-style string describing the error in more details
 *
 * Example:
 * @code
 *     int& Foo::operator[](size_t index) {
 *         ASSERT_POSTCONDITION(index<m_size, "cannot produce a valid return value");
 *         return m_array[index];
 *     }
 * @endcode
 */
#define ASSERT_POSTCONDITION(cond, format, ...)                                                    \
    (!UTILS_VERY_LIKELY(cond) ? PANIC_POSTCONDITION_IMPL(cond, format, ##__VA_ARGS__) : (void)0)

#if defined(UTILS_EXCEPTIONS) || !defined(NDEBUG)
#define ASSERT_POSTCONDITION_NON_FATAL(cond, format, ...)                                          \
    (!UTILS_VERY_LIKELY(cond) ? PANIC_POSTCONDITION_IMPL(cond, format, ##__VA_ARGS__), false : true)
#else
#define ASSERT_POSTCONDITION_NON_FATAL(cond, format, ...)                                          \
    (!UTILS_VERY_LIKELY(cond) ? PANIC_LOG_IMPL(cond, format, ##__VA_ARGS__), false : true)
#endif

/**
 * @ingroup errors
 *
 * ASSERT_ARITHMETIC is a macro that checks the given condition and reports a ArithmeticPanic
 * if it evaluates to false.
 * @param cond a boolean expression
 * @param format printf-style string describing the error in more details
 *
 * Example:
 * @code
 *     unt32_t floatToUInt1616(float v) {
 *         v *= 65536;
 *         ASSERT_ARITHMETIC(v>=0 && v<65536, "overflow occurred");
 *         return uint32_t(v);
 *     }
 * @endcode
 */
#define ASSERT_ARITHMETIC(cond, format, ...)                                                       \
    (!(cond) ? PANIC_ARITHMETIC_IMPL(cond, format, ##__VA_ARGS__) : (void)0)

#if defined(UTILS_EXCEPTIONS) || !defined(NDEBUG)
#define ASSERT_ARITHMETIC_NON_FATAL(cond, format, ...)                                             \
    (!UTILS_VERY_LIKELY(cond) ? PANIC_ARITHMETIC_IMPL(cond, format, ##__VA_ARGS__), false : true)
#else
#define ASSERT_ARITHMETIC_NON_FATAL(cond, format, ...)                                             \
    (!UTILS_VERY_LIKELY(cond) ? PANIC_LOG_IMPL(cond, format, ##__VA_ARGS__), false : true)
#endif

/**
 * @ingroup errors
 *
 * ASSERT_DESTRUCTOR is a macro that checks the given condition and logs an error
 * if it evaluates to false.
 * @param cond a boolean expression
 * @param format printf-style string describing the error in more details
 *
 * @warning Use this macro if a destructor can fail, which should be avoided at all costs.
 * Unlike the other ASSERT macros, this will never result in the process termination. Instead,
 * the error will be logged and the program will continue as if nothing happened.
 *
 * Example:
 * @code
 *     Foo::~Foo() {
 *        glDeleteTextures(1, &m_texture);
 *        GLint err = glGetError();
 *        ASSERT_DESTRUCTOR(err == GL_NO_ERROR, "cannot free GL resource!");
 *     }
 * @endcode
 */
#define ASSERT_DESTRUCTOR(cond, format, ...)                                                       \
    (!UTILS_VERY_LIKELY(cond) ? PANIC_LOG_IMPL(cond, format, ##__VA_ARGS__) : (void)0)

#endif  // TNT_UTILS_PANIC_H
