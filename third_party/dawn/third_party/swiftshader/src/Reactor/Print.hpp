// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef rr_Print_hpp
#define rr_Print_hpp

#ifdef ENABLE_RR_PRINT

#	include "Reactor.hpp"
#	include "SIMD.hpp"

#	include <string>
#	include <vector>

namespace rr {

// PrintValue holds the printf format and value(s) for a single argument
// to Print(). A single argument can be expanded into multiple printf
// values - for example a Float4 will expand to "%f %f %f %f" and four
// scalar values.
// The PrintValue constructor accepts the following:
//   * Reactor LValues, RValues, Pointers.
//   * Standard Plain-Old-Value types (int, float, bool, etc)
//   * Custom types that specialize the PrintValue::Ty template struct.
//   * Static arrays in the form T[N] where T can be any of the above.
class PrintValue
{
public:
	// Ty is a template that can be specialized for printing type T.
	// Each specialization must expose:
	//  * A 'static std::string fmt(const T& v)' method that provides the
	//    printf format specifier.
	//  * A 'static std::vector<rr::Value*> val(const T& v)' method that
	//    returns all the printf format values.
	template<typename T>
	struct Ty
	{
		// static std::string fmt(const T& v);
		// static std::vector<rr::Value*> val(const T& v);
	};

	// returns the printf values for all the values in the given array.
	template<typename T>
	static std::vector<Value *> val(const T *list, int count)
	{
		std::vector<Value *> values;
		values.reserve(count);
		for(int i = 0; i < count; i++)
		{
			auto v = val(list[i]);
			values.insert(values.end(), v.begin(), v.end());
		}
		return values;
	}

	// fmt returns the comma-delimited list of printf format strings for
	// every element in the provided list, all enclosed in square brackets.
	template<typename T>
	static std::string fmt(const T *list, int count)
	{
		std::string out = "[";
		for(int i = 0; i < count; i++)
		{
			if(i > 0) { out += ", "; }
			out += fmt(list[i]);
		}
		return out + "]";
	}

	static std::string addr(const void *ptr)
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%p", ptr);
		return buf;
	}

	const std::string format;
	const std::vector<Value *> values;

	// Constructs a PrintValue for the given value.
	template<typename T>
	PrintValue(const T &v)
	    : format(fmt(v))
	    , values(val(v))
	{}

	// Constructs a PrintValue for the given static array.
	template<typename T, int N>
	PrintValue(const T (&v)[N])
	    : format(fmt(&v[0], N))
	    , values(val(&v[0], N))
	{}

	// Constructs a PrintValue for the given array starting at arr of length
	// len.
	template<typename T>
	PrintValue(const T *arr, int len)
	    : format(fmt(arr, len))
	    , values(val(arr, len))
	{}

	// PrintValue constructors for plain-old-data values.
	PrintValue(bool v)
	    : format(v ? "true" : "false")
	{}
	PrintValue(int8_t v)
	    : format(std::to_string(v))
	{}
	PrintValue(uint8_t v)
	    : format(std::to_string(v))
	{}
	PrintValue(int16_t v)
	    : format(std::to_string(v))
	{}
	PrintValue(uint16_t v)
	    : format(std::to_string(v))
	{}
	PrintValue(int32_t v)
	    : format(std::to_string(v))
	{}
	PrintValue(uint32_t v)
	    : format(std::to_string(v))
	{}
	PrintValue(int64_t v)
	    : format(std::to_string(v))
	{}
	PrintValue(uint64_t v)
	    : format(std::to_string(v))
	{}
	PrintValue(float v)
	    : format(std::to_string(v))
	{}
	PrintValue(double v)
	    : format(std::to_string(v))
	{}
	PrintValue(const char *v)
	    : format(v)
	{}

	template<typename T>
	PrintValue(T *v)
	    : format(addr(v))
	{}

	// vals is a helper to build composite value lists.
	// vals returns the full, sequential list of printf argument values used
	// to print all the provided variadic values.
	// vals() is intended to be used by implementations of
	// PrintValue::Ty<>::vals() to help declare aggregate types.
	// For example, if you were declaring a PrintValue::Ty<> specialization
	// for a custom Mat4x4 matrix formed from four Vector4 values, you'd
	// write:
	//
	// namespace rr
	// {
	//		template <> struct PrintValue::Ty<Mat4x4>
	//		{
	//			static std::string fmt(const Mat4x4& v)
	//			{
	//				return	"[a: <%f, %f, %f, %f>,"
	//				        " b: <%f, %f, %f, %f>,"
	//				        " c: <%f, %f, %f, %f>,"
	//				        " d: <%f, %f, %f, %f>]";
	//			}
	//			static std::vector<rr::Value*> val(const Mat4x4& v)
	//			{
	//				return PrintValue::vals(v.a, v.b, v.c, v.d);
	//			}
	//		};
	//	}
	template<typename... ARGS>
	static std::vector<Value *> vals(ARGS... v)
	{
		std::vector<std::vector<Value *>> lists = { val(v)... };
		std::vector<Value *> joined;
		for(const auto &list : lists)
		{
			joined.insert(joined.end(), list.begin(), list.end());
		}
		return joined;
	}

	// returns the printf format specifier for the given type via the
	// PrintValue::Ty<T> specialization.
	template<typename T>
	static std::string fmt(const T &v)
	{
		return Ty<T>::fmt(v);
	}

	// returns the printf value for the given type with a
	// PrintValue::Ty<T> specialization.
	template<typename T>
	static std::vector<Value *> val(const T &v)
	{
		return Ty<T>::val(v);
	}
};

// PrintValue::Ty<T> specializations for basic types.
template<>
struct PrintValue::Ty<const char *>
{
	static std::string fmt(const char *v) { return "%s"; }
	static std::vector<Value *> val(const char *v);
};
template<>
struct PrintValue::Ty<std::string>
{
	static std::string fmt(const std::string &v) { return PrintValue::Ty<const char *>::fmt(v.c_str()); }
	static std::vector<Value *> val(const std::string &v) { return PrintValue::Ty<const char *>::val(v.c_str()); }
};

// PrintValue::Ty<T> specializations for standard Reactor types.
template<>
struct PrintValue::Ty<Bool>
{
	static std::string fmt(const RValue<Bool> &v) { return "%s"; }
	static std::vector<Value *> val(const RValue<Bool> &v);
};
template<>
struct PrintValue::Ty<Byte>
{
	static std::string fmt(const RValue<Byte> &v) { return "%d"; }
	static std::vector<Value *> val(const RValue<Byte> &v);
};
template<>
struct PrintValue::Ty<Byte4>
{
	static std::string fmt(const RValue<Byte4> &v) { return "[%d, %d, %d, %d]"; }
	static std::vector<Value *> val(const RValue<Byte4> &v);
};
template<>
struct PrintValue::Ty<Int>
{
	static std::string fmt(const RValue<Int> &v) { return "%d"; }
	static std::vector<Value *> val(const RValue<Int> &v);
};
template<>
struct PrintValue::Ty<Int2>
{
	static std::string fmt(const RValue<Int2> &v) { return "[%d, %d]"; }
	static std::vector<Value *> val(const RValue<Int2> &v);
};
template<>
struct PrintValue::Ty<Int4>
{
	static std::string fmt(const RValue<Int4> &v) { return "[%d, %d, %d, %d]"; }
	static std::vector<Value *> val(const RValue<Int4> &v);
};
template<>
struct PrintValue::Ty<UInt>
{
	static std::string fmt(const RValue<UInt> &v) { return "%u"; }
	static std::vector<Value *> val(const RValue<UInt> &v);
};
template<>
struct PrintValue::Ty<UInt2>
{
	static std::string fmt(const RValue<UInt2> &v) { return "[%u, %u]"; }
	static std::vector<Value *> val(const RValue<UInt2> &v);
};
template<>
struct PrintValue::Ty<UInt4>
{
	static std::string fmt(const RValue<UInt4> &v) { return "[%u, %u, %u, %u]"; }
	static std::vector<Value *> val(const RValue<UInt4> &v);
};
template<>
struct PrintValue::Ty<Short>
{
	static std::string fmt(const RValue<Short> &v) { return "%d"; }
	static std::vector<Value *> val(const RValue<Short> &v);
};
template<>
struct PrintValue::Ty<Short4>
{
	static std::string fmt(const RValue<Short4> &v) { return "[%d, %d, %d, %d]"; }
	static std::vector<Value *> val(const RValue<Short4> &v);
};
template<>
struct PrintValue::Ty<UShort>
{
	static std::string fmt(const RValue<UShort> &v) { return "%u"; }
	static std::vector<Value *> val(const RValue<UShort> &v);
};
template<>
struct PrintValue::Ty<UShort4>
{
	static std::string fmt(const RValue<UShort4> &v) { return "[%u, %u, %u, %u]"; }
	static std::vector<Value *> val(const RValue<UShort4> &v);
};
template<>
struct PrintValue::Ty<Float>
{
	static std::string fmt(const RValue<Float> &v) { return "%f"; }
	static std::vector<Value *> val(const RValue<Float> &v);
};
template<>
struct PrintValue::Ty<Float4>
{
	static std::string fmt(const RValue<Float4> &v) { return "[%f, %f, %f, %f]"; }
	static std::vector<Value *> val(const RValue<Float4> &v);
};
template<>
struct PrintValue::Ty<Long>
{
	static std::string fmt(const RValue<Long> &v) { return "%lld"; }
	static std::vector<Value *> val(const RValue<Long> &v) { return { v.value() }; }
};
template<typename T>
struct PrintValue::Ty<Pointer<T>>
{
	static std::string fmt(const RValue<Pointer<T>> &v) { return "%p"; }
	static std::vector<Value *> val(const RValue<Pointer<T>> &v) { return { v.value() }; }
};
template<>
struct PrintValue::Ty<SIMD::Pointer>
{
	static std::string fmt(const SIMD::Pointer &v)
	{
		if(v.isBasePlusOffset)
		{
			std::string format;
			for(int i = 1; i < SIMD::Width; i++) { format += ", %d"; }
			return "{%p + [%d" + format + "]}";
		}
		else
		{
			std::string format;
			for(int i = 1; i < SIMD::Width; i++) { format += ", %p"; }
			return "{%p" + format + "}";
		}
	}

	static std::vector<Value *> val(const SIMD::Pointer &v)
	{
		return v.getPrintValues();
	}
};
template<>
struct PrintValue::Ty<SIMD::Int>
{
	static std::string fmt(const RValue<SIMD::Int> &v)
	{
		std::string format;
		for(int i = 1; i < SIMD::Width; i++) { format += ", %d"; }
		return "[%d" + format + "]";
	}
	static std::vector<Value *> val(const RValue<SIMD::Int> &v);
};
template<>
struct PrintValue::Ty<SIMD::UInt>
{
	static std::string fmt(const RValue<SIMD::UInt> &v)
	{
		std::string format;
		for(int i = 1; i < SIMD::Width; i++) { format += ", %u"; }
		return "[%u" + format + "]";
	}
	static std::vector<Value *> val(const RValue<SIMD::UInt> &v);
};
template<>
struct PrintValue::Ty<SIMD::Float>
{
	static std::string fmt(const RValue<SIMD::Float> &v)
	{
		std::string format;
		for(int i = 1; i < SIMD::Width; i++) { format += ", %f"; }
		return "[%f" + format + "]";
	}
	static std::vector<Value *> val(const RValue<SIMD::Float> &v);
};
template<typename T>
struct PrintValue::Ty<Reference<T>>
{
	static std::string fmt(const Reference<T> &v) { return PrintValue::Ty<T>::fmt(v); }
	static std::vector<Value *> val(const Reference<T> &v) { return PrintValue::Ty<T>::val(v); }
};
template<typename T>
struct PrintValue::Ty<RValue<T>>
{
	static std::string fmt(const RValue<T> &v) { return PrintValue::Ty<T>::fmt(v); }
	static std::vector<Value *> val(const RValue<T> &v) { return PrintValue::Ty<T>::val(v); }
};

// VPrintf emits a call to printf() using vals[0] as the format string,
// and vals[1..n] as the args.
void VPrintf(const std::vector<Value *> &vals);

// Printv emits a call to printf() using the function, file and line,
// message and optional values.
// See Printv below.
void Printv(const char *function, const char *file, int line, const char *msg, std::initializer_list<PrintValue> vals);

// Printv emits a call to printf() using the provided message and optional
// values.
// Printf replaces any bracketed indices in the message with string
// representations of the corresponding value in vals.
// For example:
//   Printv("{0} and {1}", "red", "green");
// Would print the string:
//   "red and green"
// Arguments can be indexed in any order.
// Invalid indices are not substituted.
inline void Printv(const char *msg, std::initializer_list<PrintValue> vals)
{
	Printv(nullptr, nullptr, 0, msg, vals);
}

// Print is a wrapper over Printv that wraps the variadic arguments into an
// initializer_list before calling Printv.
template<typename... ARGS>
void Print(const char *msg, const ARGS &...vals)
{
	Printv(msg, { vals... });
}

// Print is a wrapper over Printv that wraps the variadic arguments into an
// initializer_list before calling Printv.
template<typename... ARGS>
void Print(const char *function, const char *file, int line, const char *msg, const ARGS &...vals)
{
	Printv(function, file, line, msg, { vals... });
}

// RR_LOG is a macro that calls Print(), automatically populating the
// function, file and line parameters and appending a newline to the string.
//
// RR_LOG() is intended to be used for debugging JIT compiled code, and is
// not intended for production use.
#	if defined(_WIN32)
#		define RR_LOG(msg, ...) Print(__FUNCSIG__, __FILE__, static_cast<int>(__LINE__), msg "\n", ##__VA_ARGS__)
#	else
#		define RR_LOG(msg, ...) Print(__PRETTY_FUNCTION__, __FILE__, static_cast<int>(__LINE__), msg "\n", ##__VA_ARGS__)
#	endif

// Macro magic to perform variadic dispatch.
// See: https://renenyffenegger.ch/notes/development/languages/C-C-plus-plus/preprocessor/macros/__VA_ARGS__/count-arguments
// Note, this doesn't attempt to use the ##__VA_ARGS__ trick to handle 0
#	define RR_MSVC_EXPAND_BUG(X) X  // Helper macro to force expanding __VA_ARGS__ to satisfy MSVC compiler.
#	define RR_GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N
#	define RR_COUNT_ARGUMENTS(...) RR_MSVC_EXPAND_BUG(RR_GET_NTH_ARG(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
static_assert(1 == RR_COUNT_ARGUMENTS(a), "RR_COUNT_ARGUMENTS broken");  // Sanity checks.
static_assert(2 == RR_COUNT_ARGUMENTS(a, b), "RR_COUNT_ARGUMENTS broken");
static_assert(3 == RR_COUNT_ARGUMENTS(a, b, c), "RR_COUNT_ARGUMENTS broken");

// RR_WATCH_FMT(...) resolves to a string literal that lists all the
// arguments by name. This string can be passed to LOG() to print each of
// the arguments with their name and value.
//
// RR_WATCH_FMT(...) uses the RR_COUNT_ARGUMENTS helper macro to delegate to a
// corresponding RR_WATCH_FMT_n specialization macro below.
#	define RR_WATCH_CONCAT(a, b) a##b
#	define RR_WATCH_CONCAT2(a, b) RR_WATCH_CONCAT(a, b)
#	define RR_WATCH_FMT(...) RR_MSVC_EXPAND_BUG(RR_WATCH_CONCAT2(RR_WATCH_FMT_, RR_COUNT_ARGUMENTS(__VA_ARGS__))(__VA_ARGS__))
#	define RR_WATCH_FMT_1(_1) "\n  " #    _1 ": {0}"
#	define RR_WATCH_FMT_2(_1, _2) \
		RR_WATCH_FMT_1(_1)         \
		"\n  " #_2 ": {1}"
#	define RR_WATCH_FMT_3(_1, _2, _3) \
		RR_WATCH_FMT_2(_1, _2)         \
		"\n  " #_3 ": {2}"
#	define RR_WATCH_FMT_4(_1, _2, _3, _4) \
		RR_WATCH_FMT_3(_1, _2, _3)         \
		"\n  " #_4 ": {3}"
#	define RR_WATCH_FMT_5(_1, _2, _3, _4, _5) \
		RR_WATCH_FMT_4(_1, _2, _3, _4)         \
		"\n  " #_5 ": {4}"
#	define RR_WATCH_FMT_6(_1, _2, _3, _4, _5, _6) \
		RR_WATCH_FMT_5(_1, _2, _3, _4, _5)         \
		"\n  " #_6 ": {5}"
#	define RR_WATCH_FMT_7(_1, _2, _3, _4, _5, _6, _7) \
		RR_WATCH_FMT_6(_1, _2, _3, _4, _5, _6)         \
		"\n  " #_7 ": {6}"
#	define RR_WATCH_FMT_8(_1, _2, _3, _4, _5, _6, _7, _8) \
		RR_WATCH_FMT_7(_1, _2, _3, _4, _5, _6, _7)         \
		"\n  " #_8 ": {7}"
#	define RR_WATCH_FMT_9(_1, _2, _3, _4, _5, _6, _7, _8, _9) \
		RR_WATCH_FMT_8(_1, _2, _3, _4, _5, _6, _7, _8)         \
		"\n  " #_9 ": {8}"
#	define RR_WATCH_FMT_10(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10) \
		RR_WATCH_FMT_9(_1, _2, _3, _4, _5, _6, _7, _8, _9)           \
		"\n  " #_10 ": {9}"
#	define RR_WATCH_FMT_11(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11) \
		RR_WATCH_FMT_10(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10)          \
		"\n  " #_11 ": {10}"
#	define RR_WATCH_FMT_12(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12) \
		RR_WATCH_FMT_11(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11)          \
		"\n  " #_12 ": {11}"
#	define RR_WATCH_FMT_13(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13) \
		RR_WATCH_FMT_12(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12)          \
		"\n  " #_13 ": {12}"
#	define RR_WATCH_FMT_14(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14) \
		RR_WATCH_FMT_13(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13)          \
		"\n  " #_14 ": {13}"
#	define RR_WATCH_FMT_15(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15) \
		RR_WATCH_FMT_14(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14)          \
		"\n  " #_15 ": {14}"
#	define RR_WATCH_FMT_16(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16) \
		RR_WATCH_FMT_15(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15)          \
		"\n  " #_16 ": {15}"
#	define RR_WATCH_FMT_17(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17) \
		RR_WATCH_FMT_16(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16)          \
		"\n  " #_17 ": {16}"
#	define RR_WATCH_FMT_18(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18) \
		RR_WATCH_FMT_17(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17)          \
		"\n  " #_18 ": {17}"

// RR_WATCH() is a helper that prints the name and value of all the supplied
// arguments.
// For example, if you had the Int and bool variables 'foo' and 'bar' that
// you want to print, you can simply write:
//    RR_WATCH(foo, bar)
// When this JIT compiled code is executed, it will print the string
// "foo: 1, bar: true" to stdout.
//
// RR_WATCH() is intended to be used for debugging JIT compiled code, and
// is not intended for production use.
#	define RR_WATCH(...) RR_LOG(RR_WATCH_FMT(__VA_ARGS__), __VA_ARGS__)

}  // namespace rr

#	define RR_PRINT_ONLY(x) x
#else
#	define RR_PRINT_ONLY(x)
#endif  // ENABLE_RR_PRINT

#endif  // rr_Print_hpp
