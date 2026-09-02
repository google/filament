// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef BENCHMARK_REGISTRATION_H_
#define BENCHMARK_REGISTRATION_H_

#include "benchmark/benchmark_api.h"
#include "benchmark/macros.h"

#if defined(__clang__)
#define BENCHMARK_DISABLE_COUNTER_WARNING                            \
  _Pragma("GCC diagnostic push")                                     \
      _Pragma("GCC diagnostic ignored \"-Wunknown-warning-option\"") \
          _Pragma("GCC diagnostic ignored \"-Wc2y-extensions\"")
#define BENCHMARK_RESTORE_COUNTER_WARNING _Pragma("GCC diagnostic pop")
#else
#define BENCHMARK_DISABLE_COUNTER_WARNING
#define BENCHMARK_RESTORE_COUNTER_WARNING
#endif

BENCHMARK_DISABLE_COUNTER_WARNING
#if defined(__COUNTER__) && (__COUNTER__ + 1 == __COUNTER__ + 0)
#define BENCHMARK_PRIVATE_UNIQUE_ID __COUNTER__
#else
#define BENCHMARK_PRIVATE_UNIQUE_ID __LINE__
#endif
BENCHMARK_RESTORE_COUNTER_WARNING

#define BENCHMARK_PRIVATE_NAME(...)                                      \
  BENCHMARK_PRIVATE_CONCAT(benchmark_uniq_, BENCHMARK_PRIVATE_UNIQUE_ID, \
                           __VA_ARGS__)

#define BENCHMARK_PRIVATE_CONCAT(a, b, c) BENCHMARK_PRIVATE_CONCAT2(a, b, c)
#define BENCHMARK_PRIVATE_CONCAT2(a, b, c) a##b##c
#define BENCHMARK_PRIVATE_CONCAT_NAME(BaseClass, Method) \
  BaseClass##_##Method##_Benchmark

#define BENCHMARK_PRIVATE_DECLARE(n)                                   \
  BENCHMARK_DISABLE_COUNTER_WARNING                                    \
  static ::benchmark::Benchmark const* const BENCHMARK_PRIVATE_NAME(n) \
      BENCHMARK_RESTORE_COUNTER_WARNING BENCHMARK_UNUSED

#define BENCHMARK(...)                                   \
  BENCHMARK_PRIVATE_DECLARE(_benchmark_) =               \
      (::benchmark::internal::RegisterBenchmarkInternal( \
          ::benchmark::internal::make_unique<            \
              ::benchmark::internal::FunctionBenchmark>( \
              #__VA_ARGS__,                              \
              static_cast<::benchmark::internal::Function*>(__VA_ARGS__))))

#define BENCHMARK_WITH_ARG(n, a) BENCHMARK(n)->Arg((a))
#define BENCHMARK_WITH_ARG2(n, a1, a2) BENCHMARK(n)->Args({(a1), (a2)})
#define BENCHMARK_WITH_UNIT(n, t) BENCHMARK(n)->Unit((t))
#define BENCHMARK_RANGE(n, lo, hi) BENCHMARK(n)->Range((lo), (hi))
#define BENCHMARK_RANGE2(n, l1, h1, l2, h2) \
  BENCHMARK(n)->RangePair({{(l1), (h1)}, {(l2), (h2)}})

#define BENCHMARK_CAPTURE(func, test_case_name, ...)     \
  BENCHMARK_PRIVATE_DECLARE(_benchmark_) =               \
      (::benchmark::internal::RegisterBenchmarkInternal( \
          ::benchmark::internal::make_unique<            \
              ::benchmark::internal::FunctionBenchmark>( \
              #func "/" #test_case_name,                 \
              [](::benchmark::State& st) { func(st, __VA_ARGS__); })))

#define BENCHMARK_NAMED(func, test_case_name)            \
  BENCHMARK_PRIVATE_DECLARE(_benchmark_) =               \
      (::benchmark::internal::RegisterBenchmarkInternal( \
          ::benchmark::internal::make_unique<            \
              ::benchmark::internal::FunctionBenchmark>( \
              #func "/" #test_case_name,                 \
              static_cast<::benchmark::internal::Function*>(func))))

#define BENCHMARK_TEMPLATE1(n, a)                        \
  BENCHMARK_PRIVATE_DECLARE(n) =                         \
      (::benchmark::internal::RegisterBenchmarkInternal( \
          ::benchmark::internal::make_unique<            \
              ::benchmark::internal::FunctionBenchmark>( \
              #n "<" #a ">",                             \
              static_cast<::benchmark::internal::Function*>(n<a>))))

#define BENCHMARK_TEMPLATE2(n, a, b)                     \
  BENCHMARK_PRIVATE_DECLARE(n) =                         \
      (::benchmark::internal::RegisterBenchmarkInternal( \
          ::benchmark::internal::make_unique<            \
              ::benchmark::internal::FunctionBenchmark>( \
              #n "<" #a "," #b ">",                      \
              static_cast<::benchmark::internal::Function*>(n<a, b>))))

#define BENCHMARK_TEMPLATE(n, ...)                       \
  BENCHMARK_PRIVATE_DECLARE(n) =                         \
      (::benchmark::internal::RegisterBenchmarkInternal( \
          ::benchmark::internal::make_unique<            \
              ::benchmark::internal::FunctionBenchmark>( \
              #n "<" #__VA_ARGS__ ">",                   \
              static_cast<::benchmark::internal::Function*>(n<__VA_ARGS__>))))

#define BENCHMARK_TEMPLATE1_CAPTURE(func, a, test_case_name, ...) \
  BENCHMARK_CAPTURE(func<a>, test_case_name, __VA_ARGS__)

#define BENCHMARK_TEMPLATE2_CAPTURE(func, a, b, test_case_name, ...) \
  BENCHMARK_PRIVATE_DECLARE(func) =                                  \
      (::benchmark::internal::RegisterBenchmarkInternal(             \
          ::benchmark::internal::make_unique<                        \
              ::benchmark::internal::FunctionBenchmark>(             \
              #func "<" #a "," #b ">"                                \
                    "/" #test_case_name,                             \
              [](::benchmark::State& st) { func<a, b>(st, __VA_ARGS__); })))

#define BENCHMARK_PRIVATE_DECLARE_F(BaseClass, Method)        \
  class BaseClass##_##Method##_Benchmark : public BaseClass { \
   public:                                                    \
    BaseClass##_##Method##_Benchmark() {                      \
      this->SetName(#BaseClass "/" #Method);                  \
    }                                                         \
                                                              \
   protected:                                                 \
    void BenchmarkCase(::benchmark::State&) override;         \
  };

#define BENCHMARK_TEMPLATE1_PRIVATE_DECLARE_F(BaseClass, Method, a) \
  class BaseClass##_##Method##_Benchmark : public BaseClass<a> {    \
   public:                                                          \
    BaseClass##_##Method##_Benchmark() {                            \
      this->SetName(#BaseClass "<" #a ">/" #Method);                \
    }                                                               \
                                                                    \
   protected:                                                       \
    void BenchmarkCase(::benchmark::State&) override;               \
  };

#define BENCHMARK_TEMPLATE2_PRIVATE_DECLARE_F(BaseClass, Method, a, b) \
  class BaseClass##_##Method##_Benchmark : public BaseClass<a, b> {    \
   public:                                                             \
    BaseClass##_##Method##_Benchmark() {                               \
      this->SetName(#BaseClass "<" #a "," #b ">/" #Method);            \
    }                                                                  \
                                                                       \
   protected:                                                          \
    void BenchmarkCase(::benchmark::State&) override;                  \
  };

#define BENCHMARK_TEMPLATE_PRIVATE_DECLARE_F(BaseClass, Method, ...)       \
  class BaseClass##_##Method##_Benchmark : public BaseClass<__VA_ARGS__> { \
   public:                                                                 \
    BaseClass##_##Method##_Benchmark() {                                   \
      this->SetName(#BaseClass "<" #__VA_ARGS__ ">/" #Method);             \
    }                                                                      \
                                                                           \
   protected:                                                              \
    void BenchmarkCase(::benchmark::State&) override;                      \
  };

#define BENCHMARK_DEFINE_F(BaseClass, Method)    \
  BENCHMARK_PRIVATE_DECLARE_F(BaseClass, Method) \
  void BENCHMARK_PRIVATE_CONCAT_NAME(BaseClass, Method)::BenchmarkCase

#define BENCHMARK_TEMPLATE1_DEFINE_F(BaseClass, Method, a)    \
  BENCHMARK_TEMPLATE1_PRIVATE_DECLARE_F(BaseClass, Method, a) \
  void BENCHMARK_PRIVATE_CONCAT_NAME(BaseClass, Method)::BenchmarkCase

#define BENCHMARK_TEMPLATE2_DEFINE_F(BaseClass, Method, a, b)    \
  BENCHMARK_TEMPLATE2_PRIVATE_DECLARE_F(BaseClass, Method, a, b) \
  void BENCHMARK_PRIVATE_CONCAT_NAME(BaseClass, Method)::BenchmarkCase

#define BENCHMARK_TEMPLATE_DEFINE_F(BaseClass, Method, ...)            \
  BENCHMARK_TEMPLATE_PRIVATE_DECLARE_F(BaseClass, Method, __VA_ARGS__) \
  void BENCHMARK_PRIVATE_CONCAT_NAME(BaseClass, Method)::BenchmarkCase

#define BENCHMARK_REGISTER_F(BaseClass, Method) \
  BENCHMARK_PRIVATE_REGISTER_F(BENCHMARK_PRIVATE_CONCAT_NAME(BaseClass, Method))

#define BENCHMARK_PRIVATE_REGISTER_F(TestName)           \
  BENCHMARK_PRIVATE_DECLARE(TestName) =                  \
      (::benchmark::internal::RegisterBenchmarkInternal( \
          ::benchmark::internal::make_unique<TestName>()))

#define BENCHMARK_TEMPLATE_PRIVATE_CONCAT_NAME_F(BaseClass, Method) \
  BaseClass##_##Method##_BenchmarkTemplate

#define BENCHMARK_TEMPLATE_METHOD_F(BaseClass, Method)              \
  template <class... Args>                                          \
  class BENCHMARK_TEMPLATE_PRIVATE_CONCAT_NAME_F(BaseClass, Method) \
      : public BaseClass<Args...> {                                 \
   protected:                                                       \
    using Base = BaseClass<Args...>;                                \
    void BenchmarkCase(::benchmark::State&) override;               \
  };                                                                \
  template <class... Args>                                          \
  void BENCHMARK_TEMPLATE_PRIVATE_CONCAT_NAME_F(                    \
      BaseClass, Method)<Args...>::BenchmarkCase

#define BENCHMARK_TEMPLATE_PRIVATE_INSTANTIATE_F(BaseClass, Method,           \
                                                 UniqueName, ...)             \
  class UniqueName : public BENCHMARK_TEMPLATE_PRIVATE_CONCAT_NAME_F(         \
                         BaseClass, Method)<__VA_ARGS__> {                    \
   public:                                                                    \
    UniqueName() { this->SetName(#BaseClass "<" #__VA_ARGS__ ">/" #Method); } \
  };                                                                          \
  BENCHMARK_PRIVATE_DECLARE(BaseClass##_##Method##_Benchmark) =               \
      (::benchmark::internal::RegisterBenchmarkInternal(                      \
          ::benchmark::internal::make_unique<UniqueName>()))

#define BENCHMARK_TEMPLATE_INSTANTIATE_F(BaseClass, Method, ...)    \
  BENCHMARK_DISABLE_COUNTER_WARNING                                 \
  BENCHMARK_TEMPLATE_PRIVATE_INSTANTIATE_F(                         \
      BaseClass, Method, BENCHMARK_PRIVATE_NAME(BaseClass##Method), \
      __VA_ARGS__)                                                  \
  BENCHMARK_RESTORE_COUNTER_WARNING

#define BENCHMARK_F(BaseClass, Method)           \
  BENCHMARK_PRIVATE_DECLARE_F(BaseClass, Method) \
  BENCHMARK_REGISTER_F(BaseClass, Method);       \
  void BENCHMARK_PRIVATE_CONCAT_NAME(BaseClass, Method)::BenchmarkCase

#define BENCHMARK_TEMPLATE1_F(BaseClass, Method, a)           \
  BENCHMARK_TEMPLATE1_PRIVATE_DECLARE_F(BaseClass, Method, a) \
  BENCHMARK_REGISTER_F(BaseClass, Method);                    \
  void BENCHMARK_PRIVATE_CONCAT_NAME(BaseClass, Method)::BenchmarkCase

#define BENCHMARK_TEMPLATE2_F(BaseClass, Method, a, b)           \
  BENCHMARK_TEMPLATE2_PRIVATE_DECLARE_F(BaseClass, Method, a, b) \
  BENCHMARK_REGISTER_F(BaseClass, Method);                       \
  void BENCHMARK_PRIVATE_CONCAT_NAME(BaseClass, Method)::BenchmarkCase

#define BENCHMARK_TEMPLATE_F(BaseClass, Method, ...)                   \
  BENCHMARK_TEMPLATE_PRIVATE_DECLARE_F(BaseClass, Method, __VA_ARGS__) \
  void BENCHMARK_PRIVATE_CONCAT_NAME(BaseClass, Method)::BenchmarkCase

#define BENCHMARK_MAIN()                                                \
  int main(int argc, char** argv) {                                     \
    benchmark::MaybeReenterWithoutASLR(argc, argv);                     \
    char arg0_default[] = "benchmark";                                  \
    char* args_default = reinterpret_cast<char*>(arg0_default);         \
    if (!argv) {                                                        \
      argc = 1;                                                         \
      argv = &args_default;                                             \
    }                                                                   \
    ::benchmark::Initialize(&argc, argv);                               \
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1; \
    ::benchmark::RunSpecifiedBenchmarks();                              \
    ::benchmark::Shutdown();                                            \
    return 0;                                                           \
  }                                                                     \
  int main(int, char**)

#endif  // BENCHMARK_REGISTRATION_H_
