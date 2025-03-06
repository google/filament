// Copyright 2022 The Dawn & Tint Authors
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

#include <memory>

#include "benchmark/benchmark.h"

#include "src/tint/utils/rtti/switch.h"

namespace tint {
namespace {

struct Base : public Castable<Base> {};
struct A : public Castable<A, Base> {};
struct AA : public Castable<AA, A> {};
struct AAA : public Castable<AAA, AA> {};
struct AAB : public Castable<AAB, AA> {};
struct AAC : public Castable<AAC, AA> {};
struct AB : public Castable<AB, A> {};
struct ABA : public Castable<ABA, AB> {};
struct ABB : public Castable<ABB, AB> {};
struct ABC : public Castable<ABC, AB> {};
struct AC : public Castable<AC, A> {};
struct ACA : public Castable<ACA, AC> {};
struct ACB : public Castable<ACB, AC> {};
struct ACC : public Castable<ACC, AC> {};
struct B : public Castable<B, Base> {};
struct BA : public Castable<BA, B> {};
struct BAA : public Castable<BAA, BA> {};
struct BAB : public Castable<BAB, BA> {};
struct BAC : public Castable<BAC, BA> {};
struct BB : public Castable<BB, B> {};
struct BBA : public Castable<BBA, BB> {};
struct BBB : public Castable<BBB, BB> {};
struct BBC : public Castable<BBC, BB> {};
struct BC : public Castable<BC, B> {};
struct BCA : public Castable<BCA, BC> {};
struct BCB : public Castable<BCB, BC> {};
struct BCC : public Castable<BCC, BC> {};
struct C : public Castable<C, Base> {};
struct CA : public Castable<CA, C> {};
struct CAA : public Castable<CAA, CA> {};
struct CAB : public Castable<CAB, CA> {};
struct CAC : public Castable<CAC, CA> {};
struct CB : public Castable<CB, C> {};
struct CBA : public Castable<CBA, CB> {};
struct CBB : public Castable<CBB, CB> {};
struct CBC : public Castable<CBC, CB> {};
struct CC : public Castable<CC, C> {};
struct CCA : public Castable<CCA, CC> {};
struct CCB : public Castable<CCB, CC> {};
struct CCC : public Castable<CCC, CC> {};

using AllTypes = std::tuple<Base,
                            A,
                            AA,
                            AAA,
                            AAB,
                            AAC,
                            AB,
                            ABA,
                            ABB,
                            ABC,
                            AC,
                            ACA,
                            ACB,
                            ACC,
                            B,
                            BA,
                            BAA,
                            BAB,
                            BAC,
                            BB,
                            BBA,
                            BBB,
                            BBC,
                            BC,
                            BCA,
                            BCB,
                            BCC,
                            C,
                            CA,
                            CAA,
                            CAB,
                            CAC,
                            CB,
                            CBA,
                            CBB,
                            CBC,
                            CC,
                            CCA,
                            CCB,
                            CCC>;

std::vector<std::unique_ptr<Base>> MakeObjects() {
    std::vector<std::unique_ptr<Base>> out;
    out.emplace_back(std::make_unique<Base>());
    out.emplace_back(std::make_unique<A>());
    out.emplace_back(std::make_unique<AA>());
    out.emplace_back(std::make_unique<AAA>());
    out.emplace_back(std::make_unique<AAB>());
    out.emplace_back(std::make_unique<AAC>());
    out.emplace_back(std::make_unique<AB>());
    out.emplace_back(std::make_unique<ABA>());
    out.emplace_back(std::make_unique<ABB>());
    out.emplace_back(std::make_unique<ABC>());
    out.emplace_back(std::make_unique<AC>());
    out.emplace_back(std::make_unique<ACA>());
    out.emplace_back(std::make_unique<ACB>());
    out.emplace_back(std::make_unique<ACC>());
    out.emplace_back(std::make_unique<B>());
    out.emplace_back(std::make_unique<BA>());
    out.emplace_back(std::make_unique<BAA>());
    out.emplace_back(std::make_unique<BAB>());
    out.emplace_back(std::make_unique<BAC>());
    out.emplace_back(std::make_unique<BB>());
    out.emplace_back(std::make_unique<BBA>());
    out.emplace_back(std::make_unique<BBB>());
    out.emplace_back(std::make_unique<BBC>());
    out.emplace_back(std::make_unique<BC>());
    out.emplace_back(std::make_unique<BCA>());
    out.emplace_back(std::make_unique<BCB>());
    out.emplace_back(std::make_unique<BCC>());
    out.emplace_back(std::make_unique<C>());
    out.emplace_back(std::make_unique<CA>());
    out.emplace_back(std::make_unique<CAA>());
    out.emplace_back(std::make_unique<CAB>());
    out.emplace_back(std::make_unique<CAC>());
    out.emplace_back(std::make_unique<CB>());
    out.emplace_back(std::make_unique<CBA>());
    out.emplace_back(std::make_unique<CBB>());
    out.emplace_back(std::make_unique<CBC>());
    out.emplace_back(std::make_unique<CC>());
    out.emplace_back(std::make_unique<CCA>());
    out.emplace_back(std::make_unique<CCB>());
    out.emplace_back(std::make_unique<CCC>());
    return out;
}

void CastableLargeSwitch(::benchmark::State& state) {
    auto objects = MakeObjects();
    size_t i = 0;
    for (auto _ : state) {
        auto* object = objects[i % objects.size()].get();
        Switch(
            object,  //
            [&](const AAA*) { ::benchmark::DoNotOptimize(i += 40); },
            [&](const AAB*) { ::benchmark::DoNotOptimize(i += 50); },
            [&](const AAC*) { ::benchmark::DoNotOptimize(i += 60); },
            [&](const ABA*) { ::benchmark::DoNotOptimize(i += 80); },
            [&](const ABB*) { ::benchmark::DoNotOptimize(i += 90); },
            [&](const ABC*) { ::benchmark::DoNotOptimize(i += 100); },
            [&](const ACA*) { ::benchmark::DoNotOptimize(i += 120); },
            [&](const ACB*) { ::benchmark::DoNotOptimize(i += 130); },
            [&](const ACC*) { ::benchmark::DoNotOptimize(i += 140); },
            [&](const BAA*) { ::benchmark::DoNotOptimize(i += 170); },
            [&](const BAB*) { ::benchmark::DoNotOptimize(i += 180); },
            [&](const BAC*) { ::benchmark::DoNotOptimize(i += 190); },
            [&](const BBA*) { ::benchmark::DoNotOptimize(i += 210); },
            [&](const BBB*) { ::benchmark::DoNotOptimize(i += 220); },
            [&](const BBC*) { ::benchmark::DoNotOptimize(i += 230); },
            [&](const BCA*) { ::benchmark::DoNotOptimize(i += 250); },
            [&](const BCB*) { ::benchmark::DoNotOptimize(i += 260); },
            [&](const BCC*) { ::benchmark::DoNotOptimize(i += 270); },
            [&](const CA*) { ::benchmark::DoNotOptimize(i += 290); },
            [&](const CAA*) { ::benchmark::DoNotOptimize(i += 300); },
            [&](const CAB*) { ::benchmark::DoNotOptimize(i += 310); },
            [&](const CAC*) { ::benchmark::DoNotOptimize(i += 320); },
            [&](const CBA*) { ::benchmark::DoNotOptimize(i += 340); },
            [&](const CBB*) { ::benchmark::DoNotOptimize(i += 350); },
            [&](const CBC*) { ::benchmark::DoNotOptimize(i += 360); },
            [&](const CCA*) { ::benchmark::DoNotOptimize(i += 380); },
            [&](const CCB*) { ::benchmark::DoNotOptimize(i += 390); },
            [&](const CCC*) { ::benchmark::DoNotOptimize(i += 400); },
            [&](Default) { ::benchmark::DoNotOptimize(i += 123); });
        i = (i * 31) ^ (i << 5);
    }
}

BENCHMARK(CastableLargeSwitch);

void CastableMediumSwitch(::benchmark::State& state) {
    auto objects = MakeObjects();
    size_t i = 0;
    for (auto _ : state) {
        auto* object = objects[i % objects.size()].get();
        Switch(
            object,  //
            [&](const ACB*) { ::benchmark::DoNotOptimize(i += 130); },
            [&](const BAA*) { ::benchmark::DoNotOptimize(i += 170); },
            [&](const BAB*) { ::benchmark::DoNotOptimize(i += 180); },
            [&](const BBA*) { ::benchmark::DoNotOptimize(i += 210); },
            [&](const BBB*) { ::benchmark::DoNotOptimize(i += 220); },
            [&](const CAA*) { ::benchmark::DoNotOptimize(i += 300); },
            [&](const CCA*) { ::benchmark::DoNotOptimize(i += 380); },
            [&](const CCB*) { ::benchmark::DoNotOptimize(i += 390); },
            [&](const CCC*) { ::benchmark::DoNotOptimize(i += 400); },
            [&](Default) { ::benchmark::DoNotOptimize(i += 123); });
        i = (i * 31) ^ (i << 5);
    }
}

BENCHMARK(CastableMediumSwitch);

void CastableSmallSwitch(::benchmark::State& state) {
    auto objects = MakeObjects();
    size_t i = 0;
    for (auto _ : state) {
        auto* object = objects[i % objects.size()].get();
        Switch(
            object,  //
            [&](const AAB*) { ::benchmark::DoNotOptimize(i += 30); },
            [&](const CAC*) { ::benchmark::DoNotOptimize(i += 290); },
            [&](const CAA*) { ::benchmark::DoNotOptimize(i += 300); });
        i = (i * 31) ^ (i << 5);
    }
}

BENCHMARK(CastableSmallSwitch);

}  // namespace
}  // namespace tint

TINT_INSTANTIATE_TYPEINFO(tint::Base);
TINT_INSTANTIATE_TYPEINFO(tint::A);
TINT_INSTANTIATE_TYPEINFO(tint::AA);
TINT_INSTANTIATE_TYPEINFO(tint::AAA);
TINT_INSTANTIATE_TYPEINFO(tint::AAB);
TINT_INSTANTIATE_TYPEINFO(tint::AAC);
TINT_INSTANTIATE_TYPEINFO(tint::AB);
TINT_INSTANTIATE_TYPEINFO(tint::ABA);
TINT_INSTANTIATE_TYPEINFO(tint::ABB);
TINT_INSTANTIATE_TYPEINFO(tint::ABC);
TINT_INSTANTIATE_TYPEINFO(tint::AC);
TINT_INSTANTIATE_TYPEINFO(tint::ACA);
TINT_INSTANTIATE_TYPEINFO(tint::ACB);
TINT_INSTANTIATE_TYPEINFO(tint::ACC);
TINT_INSTANTIATE_TYPEINFO(tint::B);
TINT_INSTANTIATE_TYPEINFO(tint::BA);
TINT_INSTANTIATE_TYPEINFO(tint::BAA);
TINT_INSTANTIATE_TYPEINFO(tint::BAB);
TINT_INSTANTIATE_TYPEINFO(tint::BAC);
TINT_INSTANTIATE_TYPEINFO(tint::BB);
TINT_INSTANTIATE_TYPEINFO(tint::BBA);
TINT_INSTANTIATE_TYPEINFO(tint::BBB);
TINT_INSTANTIATE_TYPEINFO(tint::BBC);
TINT_INSTANTIATE_TYPEINFO(tint::BC);
TINT_INSTANTIATE_TYPEINFO(tint::BCA);
TINT_INSTANTIATE_TYPEINFO(tint::BCB);
TINT_INSTANTIATE_TYPEINFO(tint::BCC);
TINT_INSTANTIATE_TYPEINFO(tint::C);
TINT_INSTANTIATE_TYPEINFO(tint::CA);
TINT_INSTANTIATE_TYPEINFO(tint::CAA);
TINT_INSTANTIATE_TYPEINFO(tint::CAB);
TINT_INSTANTIATE_TYPEINFO(tint::CAC);
TINT_INSTANTIATE_TYPEINFO(tint::CB);
TINT_INSTANTIATE_TYPEINFO(tint::CBA);
TINT_INSTANTIATE_TYPEINFO(tint::CBB);
TINT_INSTANTIATE_TYPEINFO(tint::CBC);
TINT_INSTANTIATE_TYPEINFO(tint::CC);
TINT_INSTANTIATE_TYPEINFO(tint::CCA);
TINT_INSTANTIATE_TYPEINFO(tint::CCB);
TINT_INSTANTIATE_TYPEINFO(tint::CCC);
