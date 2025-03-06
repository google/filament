///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// AllocatorTest.cpp                                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/WinIncludes.h"
#include "dxc/Test/HlslTestUtils.h"

#include "dxc/HLSL/DxilSpanAllocator.h"
#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <map>
#include <random>
#include <set>
#include <vector>

using namespace hlsl;

/*  Test Case Breakdown:

Dimensions:
  1. Allocator (min, max)
    Success cases
    - 0, 10
    - 0, 0
    - 0, UINT_MAX
    - UINT_MAX, UINT_MAX
    Failure cases
    - 10, 9
    - 1, 0
    - UINT_MAX, 0
    - UINT_MAX, UINT_MAX - 1
  2. Span scenarios
    Generate some legal spans first:
      - randomly choose whether to reserve space at end for unbounded, reduce
range by this value
      - few:
        set num = 5;
        add spans in order with random offsets from previous begin/end until
full or num reached: offset = rand(1, space left)
      - lots:
        set num = min(range, 1000)
        add spans in order with random offsets from previous begin/end until
full or num reached: offset = rand(1, min(space left, max(1, range / num))) Copy
and shuffle spans Add spans to allocator, expect all success
      - randomly choose whether to attempt to add unbounded
        Should pass if space avail, fail if space filled
    Select starting and ending spans (one and two):
      - gather unique span cases:
        one = two = first
        one = two = second
        one = two = random
        one = two = last - 1
        one = two = last
      - add these if one < two:
        one = first, two = second
        one = first, two = last-1
        one = first, two = last
        one = first, two = random
        one = random, two = random
        one = random, two = last
        one = second, two = last-1
        one = second, two = last
    Add overlapping span, with start and end scenarios as follows:
    should have (one <= conflict && conflict <= two)
      for start:
        if prev = span before one:
          - prev.end < start, start < one.start
        else:
          - Min <= start, start < one.start
        - start == one.start
        - start > one.start, start < one.end
        - start == one.end
      for end (when start <= end, and unique from other cases):
        - end == max(start, two.start)
        - end > two.start, end < two.end
        - end == two.end
        if next = span after two:
          - end > two.end, end < next.start
        else:
          - end > two.end, end <= Max

*/

// Modifies input pos and returns false on overflow, or exceeds max
bool Align(unsigned &pos, unsigned end, unsigned align) {
  if (end < pos)
    return false;
  unsigned original = pos;
  unsigned rem = (1 < align) ? pos % align : 0;
  pos = rem ? pos + (align - rem) : pos;
  return original <= pos && pos <= end;
}

struct Element {
  Element() = default;
  Element(const Element &) = default;
  Element &operator=(const Element &) = default;
  Element(unsigned id, unsigned start, unsigned end)
      : id(id), start(start), end(end) {}
  bool operator<(const Element &other) { return id < other.id; }
  unsigned id; // index in original ordered vector
  unsigned start, end;
};
typedef std::vector<Element> ElementVector;
typedef SpanAllocator<unsigned, Element> Allocator;

struct IntersectionTestCase {
  IntersectionTestCase(unsigned one, unsigned two, unsigned start, unsigned end)
      : idOne(one), idTwo(two), element(UINT_MAX, start, end) {
    DXASSERT_NOMSG(one <= two && start <= end);
  }
  unsigned idOne, idTwo;
  Element element;
  bool operator<(const IntersectionTestCase &other) const {
    if (element.start < other.element.start)
      return true;
    else if (other.element.start < element.start)
      return false;
    if (element.end < other.element.end)
      return true;
    else if (other.element.end < element.end)
      return false;
    return false;
  }
};
typedef std::set<IntersectionTestCase> IntersectionSet;

struct Gap {
  Gap(unsigned start, unsigned end, const Element *eBefore,
      const Element *eAfter)
      : start(start), end(end), sizeLess1(end - start), eBefore(eBefore),
        eAfter(eAfter) {}
  unsigned start, end;
  unsigned sizeLess1;
  const Element *eBefore = nullptr;
  const Element *eAfter = nullptr;
};
typedef std::vector<Gap> GapVector;
const Gap *GetGapBeforeFirst(const GapVector &G) {
  if (!G.empty() && !G.front().eBefore)
    return &G.front();
  return nullptr;
}
const Gap *GetGapBetweenFirstAndSecond(const GapVector &G) {
  for (auto &gap : G) {
    if (gap.eBefore) {
      if (gap.eAfter)
        return &gap;
      break;
    }
  }
  return nullptr;
}
const Gap *GetGapBetweenLastAndSecondLast(const GapVector &G) {
  if (!G.empty()) {
    auto it = G.end();
    for (--it; it != G.begin(); --it) {
      const Gap &gap = *it;
      if (gap.eAfter) {
        if (gap.eBefore)
          return &gap;
        break;
      }
    }
  }
  return nullptr;
}
const Gap *GetGapAfterLast(const GapVector &G) {
  if (!G.empty() && !G.back().eAfter)
    return &G.back();
  return nullptr;
}

void GatherGaps(GapVector &gaps, const ElementVector &spans, unsigned Min,
                unsigned Max, unsigned alignment = 1) {
  unsigned start, end;
  const Element *eBefore = nullptr;
  const Element *eAfter = nullptr;

  // Gather gaps
  eBefore = nullptr;
  for (auto &&span : spans) {
    eAfter = &span;
    if (!eBefore) {
      start = Min;
      if (start < span.start) {
        end = span.start -
              1; // can underflow, this is the first span, so guarded by if
        if (Align(start, end, alignment))
          gaps.emplace_back(start, end, eBefore, eAfter);
      }
    } else {
      start = eBefore->end + 1;
      end = span.start - 1; // can't underflow, this is the second span
      if (Align(start, end, alignment))
        gaps.emplace_back(start, end, eBefore, eAfter);
    }
    eBefore = &span;
  }
  eAfter = nullptr;
  if (!eBefore) {
    // No spans
    start = Min;
    end = Max;
    if (Align(start, end, alignment))
      gaps.emplace_back(start, end, eBefore, eAfter);
  } else if (eBefore->end < Max) {
    // gap at end
    start = eBefore->end + 1;
    end = Max;
    if (start <= end) {
      if (Align(start, end, alignment))
        gaps.emplace_back(start, end, eBefore, eAfter);
    }
  }
}

void GetGapExtremes(const GapVector &gaps, const Gap *&largest,
                    const Gap *&smallest) {
  largest = nullptr;
  smallest = nullptr;
  for (auto &gap : gaps) {
    if (!largest || largest->sizeLess1 < gap.sizeLess1) {
      largest = &gap;
    }
    if (!smallest || smallest->sizeLess1 > gap.sizeLess1) {
      smallest = &gap;
    }
  }
}

const Gap *FindGap(const GapVector &gaps, unsigned sizeLess1) {
  if (sizeLess1 == UINT_MAX) {
    return GetGapAfterLast(gaps);
  } else {
    for (auto &gap : gaps) {
      if (gap.sizeLess1 >= sizeLess1)
        return &gap;
    }
  }
  return nullptr;
}

const Gap *NextGap(const GapVector &gaps, unsigned pos) {
  for (auto &gap : gaps) {
    if (gap.end < pos)
      continue;
    return &gap;
  }
  return nullptr;
}

// if rand() only uses 15 bits:
// unsigned rand32() { return (rand() << 30) ^ (rand() << 15) ^ rand(); }

struct Scenario {
  Scenario(unsigned Min, unsigned Max, unsigned MaxSpans, bool SpaceAtEnd,
           unsigned Seed)
      : Min(Min), Max(Max), randGen(Seed) {

    if (!MaxSpans || (SpaceAtEnd && Min == Max))
      return;
    spans.reserve(MaxSpans);

    {
      unsigned last = SpaceAtEnd ? Max - 1 : Max;
      unsigned next = Min;
      unsigned max_size = std::max(
          (unsigned)(((int64_t)(last - next) + 1) / MaxSpans), (unsigned)1);
      unsigned offset;

      while (spans.size() < MaxSpans && next <= last) {
        if ((randGen() & 3) == 3) // 1 in 4 chance for adjacent span
          offset = 0;
        else
          offset = randGen() % max_size;

        unsigned start = next + offset;
        if (start > last || start < next) // overflow
          start = last;

        if ((randGen() & 3) == 3) // 1 in 4 chance for size 1 (start == end)
          offset = 0;
        else
          offset = randGen() % max_size;

        unsigned end = start + offset;
        if (end >= last || end < start) // overflow
          end = next = last;
        else
          next = end + 1;

        spans.emplace_back(spans.size(), start, end);

        if (end == last)
          break;
      }
    }

    if (spans.empty())
      return;

    shuffledSpans = spans;
    std::shuffle(shuffledSpans.begin(), shuffledSpans.end(), randGen);

    // Create conflict spans
    typedef std::pair<unsigned, unsigned> Test;

    // These are pairs of element indexes to construct intersecting spans from
    std::set<Test> pairs;

    unsigned maxIdx = spans.size() - 1;
    // - gather unique span cases:
    //   one = two = first
    pairs.insert(Test(0, 0));
    //   one = two = second
    if (maxIdx > 0) {
      pairs.insert(Test(1, 1));
    }
    //   one = two = random
    if (maxIdx > 2) {
      unsigned randIdx = 1 + (randGen() % (maxIdx - 2));
      unsigned one = std::min(randIdx, maxIdx);
      pairs.insert(Test(one, one));
    }
    //   one = two = last - 1
    if (maxIdx > 1) {
      pairs.insert(Test(maxIdx - 1, maxIdx - 1));
    }
    //   one = two = last
    pairs.insert(Test(maxIdx, maxIdx));

    // - add these if one < two:
    //   one = first, two = second
    if (maxIdx > 0) {
      pairs.insert(Test(0, 1));
    }
    //   one = first, two = last-1
    if (maxIdx > 1) {
      pairs.insert(Test(0, maxIdx - 1));
    }
    //   one = first, two = last
    if (maxIdx > 1) {
      pairs.insert(Test(0, maxIdx));
    }
    //   one = first, two = random
    if (maxIdx > 3) {
      unsigned randIdx = 1 + (randGen() % (maxIdx - 2));
      unsigned two = std::min(randIdx, maxIdx);
      pairs.insert(Test(0, two));
    }
    //   one = random, two = random
    if (maxIdx > 4) {
      unsigned randIdx = 1 + (randGen() % (maxIdx - 3));
      unsigned one = std::min(randIdx, maxIdx);
      randIdx = one + 1 + (randGen() % (maxIdx - (one + 1)));
      unsigned two = std::min(randIdx, maxIdx);
      pairs.insert(Test(one, two));
    }
    //   one = random, two = last
    if (maxIdx > 3) {
      unsigned randIdx = 1 + (randGen() % (maxIdx - 2));
      unsigned one = std::min(randIdx, maxIdx);
      pairs.insert(Test(one, maxIdx));
    }
    //   one = second, two = last-1
    if (maxIdx > 1) {
      pairs.insert(Test(1, maxIdx - 1));
    }
    //   one = second, two = last
    if (maxIdx > 1) {
      pairs.insert(Test(1, maxIdx));
    }

    // These are start/end pairs that represent the intersecting spans that we
    // will construct
    for (auto &&test : pairs) {
      // prev -> one -> ... -> two -> next
      // Where one and two are indexes into spans where one <= two
      // Where prev and next are only set if they exist
      const Element *prev = test.first ? &spans[test.first - 1] : nullptr;
      const Element *one = &spans[test.first];
      const Element *two = &spans[test.second];
      const Element *next =
          (test.second < spans.size() - 1) ? &spans[test.second + 1] : nullptr;
      unsigned start = 0;
      unsigned space = 0;

      // Function to add intersection tests, given start
      auto AddEnds = [&]() {
        //   for end (when start <= end, and unique from other cases):
        //     - end == max(start, two.start)
        unsigned end = std::max(start, two->start);
        intersectionTests.emplace(test.first, test.second, start, end);

        //     - end > two.start, end < two.end
        if (end < two->end) {
          ++end;
          space = two->end - end;
          if (space > 1)
            end += randGen() % space;
          if (start <= end)
            intersectionTests.emplace(test.first, test.second, start, end);
        }

        //     - end == two.end
        end = two->end;
        if (start <= end)
          intersectionTests.emplace(test.first, test.second, start, end);

        //     if next = span after two:
        //       - end > two.end, end < next.start
        //     else:
        //       - end > two.end, end <= Max
        space = (next ? next->start - 1 : Max) - two->end;
        if (space) {
          end = two->end + 1 + randGen() % space;
          if (start <= end)
            intersectionTests.emplace(test.first, test.second, start, end);
        }
      };

      // Add overlapping span, with start and end scenarios as follows:
      // should have (one <= conflict && conflict <= two)
      //   for start:
      //     if prev = span before one:
      //       - prev.end < start, start < one.start
      //     else:
      //       - Min <= start, start < one.start
      if (prev)
        start = prev->end + 1;
      else
        start = Min;
      if (start < one->start) {
        space = one->start - start;
        if (space > 1)
          start += randGen() % space;
        AddEnds();
      }

      //     - start == one.start
      start = one->start;
      AddEnds();

      //     - start > one.start, start < one.end
      if (one->start < Max && one->start + 1 < one->end) {
        start = one->start + 1;
        AddEnds();
      }

      //     - start == one.end
      start = one->end;
      AddEnds();
    }
  }

  void CreateGaps() {
    GatherGaps(gaps[1], spans, Min, Max, 1);
    GetGapExtremes(gaps[1], gapLargest[1], gapSmallest[1]);
    GatherGaps(gaps[4], spans, Min, Max, 4);
    GetGapExtremes(gaps[4], gapLargest[4], gapSmallest[4]);
  }

  bool InsertSpans(Allocator &alloc) {
    WEX::TestExecution::SetVerifyOutput verifySettings(
        WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
    for (auto &it : shuffledSpans) {
      const Element *e = &it;
      const Element *conflict = alloc.Insert(e, e->start, e->end);
      VERIFY_ARE_EQUAL(conflict, nullptr);
    }
    return true;
  }

  bool VerifySpans(Allocator &alloc) {
    WEX::TestExecution::SetVerifyOutput verifySettings(
        WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
    unsigned index = 0;
    unsigned last = Min;
    bool first = true;
    bool full = !alloc.GetSpans().empty();
    unsigned firstFree = Min;
    for (auto &span : alloc.GetSpans()) {
      VERIFY_IS_TRUE(Min <= span.start && span.start <= span.end &&
                     span.end <= Max);

      if (!first)
        ++last;

      VERIFY_IS_TRUE(last <= span.start);
      if (full && last < span.start) {
        full = false;
        firstFree = last;
      }

      // id == index in spans for original inserted elements only,
      // so skip test elements with UINT_MAX id
      if (span.element->id != UINT_MAX) {
        VERIFY_IS_TRUE(index == span.element->id);
        ++index;
      }

      VERIFY_IS_TRUE(span.start == span.element->start);
      VERIFY_IS_TRUE(span.end == span.element->end);

      last = span.end;
      first = false;
    }
    if (full && last < Max) {
      full = false;
      firstFree = last + 1;
    }
    if (full) {
      VERIFY_IS_TRUE(alloc.IsFull());
    } else {
      VERIFY_IS_FALSE(alloc.IsFull());
      VERIFY_IS_TRUE(alloc.GetFirstFree() == firstFree);
    }
    return true;
  }

  unsigned Min, Max;
  ElementVector spans;
  ElementVector shuffledSpans;
  IntersectionSet intersectionTests;
  // Gaps by alignment
  std::map<unsigned, GapVector> gaps;
  // Smallest gap by alignment
  std::map<unsigned, const Gap *> gapSmallest;
  // Largest gap by alignment
  std::map<unsigned, const Gap *> gapLargest;
  std::mt19937 randGen;
};

// The test fixture.
#ifdef _WIN32
class AllocatorTest {
#else
class AllocatorTest : public ::testing::Test {
protected:
#endif
  std::vector<Scenario> m_Scenarios;

public:
  BEGIN_TEST_CLASS(AllocatorTest)
  TEST_CLASS_PROPERTY(L"Parallel", L"true")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  TEST_CLASS_SETUP(AllocatorTestSetup);

  TEST_METHOD(Intersections)
  TEST_METHOD(GapFilling)
  TEST_METHOD(Allocate)

  void InitScenarios() {
    struct P {
      unsigned Min, Max, MaxSpans;
      bool SpaceAtEnd;
      unsigned SeedOffset;
    };
    static const P params[] = {
        // Min, Max, MaxSpans, SpaceAtEnd, Seed
        // - 0, 0
        {0, 0, 1, false, 0},
        {0, 0, 1, true, 0},
        // - UINT_MAX, UINT_MAX
        {UINT_MAX, UINT_MAX, 1, false, 0},
        {UINT_MAX, UINT_MAX, 1, true, 0},
        // - small, small
        {0, 20, 5, false, 0},
        {0, 20, 5, true, 0},
        {21, 96, 5, false, 0},
        {21, 96, 5, true, 0},
        // - 0, UINT_MAX
        {0, UINT_MAX, 0, false, 0},
        {0, UINT_MAX, 10, false, 0},
        {0, UINT_MAX, 10, true, 0},
        {0, UINT_MAX, 100, false, 0},
        {0, UINT_MAX, 100, true, 0},
    };
    static const unsigned count = _countof(params);

    m_Scenarios.reserve(count);
    for (unsigned i = 0; i < count; ++i) {
      const P &p = params[i];
      m_Scenarios.emplace_back(p.Min, p.Max, p.MaxSpans, p.SpaceAtEnd,
                               i + p.SeedOffset);
      m_Scenarios.back().CreateGaps();
    }
  }
};

bool AllocatorTest::AllocatorTestSetup() {
  InitScenarios();
  return true;
}

TEST_F(AllocatorTest, Intersections) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  for (auto &&scenario : m_Scenarios) {
    Allocator alloc(scenario.Min, scenario.Max);
    VERIFY_IS_TRUE(scenario.InsertSpans(alloc));

    for (auto &&test : scenario.intersectionTests) {
      const Element &e = test.element;
      const Element *conflict = alloc.Insert(&e, e.start, e.end);
      VERIFY_IS_TRUE(conflict != nullptr);

      if (conflict) {
        VERIFY_IS_TRUE(conflict->id >= test.idOne);
        VERIFY_IS_TRUE(conflict->id <= test.idTwo);
      }
    }
    VERIFY_IS_TRUE(scenario.VerifySpans(alloc));
  }
}

TEST_F(AllocatorTest, GapFilling) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  for (auto &&scenario : m_Scenarios) {

    // Fill all gaps with Insert, no alignment, verify first free advances and
    // container is full at the end
    {
      // WEX::TestExecution::SetVerifyOutput
      // verifySettings(WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
      Allocator alloc(scenario.Min, scenario.Max);
      VERIFY_IS_TRUE(scenario.InsertSpans(alloc));
      GapVector &gaps = scenario.gaps[1];
      for (auto &gap : gaps) {
        VERIFY_IS_FALSE(alloc.IsFull());
        VERIFY_ARE_EQUAL(alloc.GetFirstFree(), gap.start);
        Element e(UINT_MAX, gap.start, gap.end);
        VERIFY_IS_NULL(alloc.Insert(&e, gap.start, gap.end));
      }
      VERIFY_IS_TRUE(alloc.IsFull());
    }
    bool InsertSucceeded = true;
    VERIFY_IS_TRUE(InsertSucceeded);

    // Fill all gaps with Allocate, no alignment, verify first free advances and
    // container is full at the end
    {
      // WEX::TestExecution::SetVerifyOutput
      // verifySettings(WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
      Allocator alloc(scenario.Min, scenario.Max);
      VERIFY_IS_TRUE(scenario.InsertSpans(alloc));
      GapVector &gaps = scenario.gaps[1];
      for (auto &gap : gaps) {
        unsigned start = gap.start;
        unsigned end = gap.end;
        VERIFY_IS_FALSE(alloc.IsFull());
        VERIFY_ARE_EQUAL(alloc.GetFirstFree(), start);
        Element e(UINT_MAX, start, end);
        unsigned pos = 0xFEFEFEFE;
        if (gap.sizeLess1 < UINT_MAX) {
          VERIFY_IS_TRUE(alloc.Allocate(&e, gap.sizeLess1 + 1, pos));
        } else {
          const Element *unbounded = &e;
          VERIFY_IS_TRUE(alloc.AllocateUnbounded(unbounded, pos));
          VERIFY_ARE_EQUAL(alloc.GetUnbounded(), unbounded);
        }
        VERIFY_ARE_EQUAL(start, pos);
      }
      VERIFY_IS_TRUE(alloc.IsFull());
    }
    bool AllocSucceeded = true;
    VERIFY_IS_TRUE(AllocSucceeded);
  }
}

TEST_F(AllocatorTest, Allocate) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  for (auto &scenario : m_Scenarios) {

    // Test for alignment 1 (no alignment), then alignment 4
    unsigned alignment = 1;
    const GapVector *pGaps = &scenario.gaps[alignment];
    const Gap *largestGap = scenario.gapLargest[alignment];
    const Gap *smallestGap = scenario.gapSmallest[alignment];

    // Test a particular allocation size with some alignment
    auto TestFn = [&](unsigned sizeLess1) {
      DXASSERT_NOMSG(pGaps);
      const Gap *pEndGap = GetGapAfterLast(*pGaps);
      Allocator alloc(scenario.Min, scenario.Max);
      VERIFY_IS_TRUE(scenario.InsertSpans(alloc));

      // This needs to be allocated outside the control flow because we need the
      // stack allocation to remain valid until the spans are verified.
      Element e;
      if (!largestGap || // no gaps
          (sizeLess1 < UINT_MAX &&
           sizeLess1 >
               largestGap->sizeLess1) || // not unbounded and size too large
          (sizeLess1 == UINT_MAX && !pEndGap)) { // unbounded and no end gap
        // no large enough gap, should fail to allocate
        e = Element(UINT_MAX, 0, 0);
        unsigned pos = 0xFEFEFEFE;
        if (sizeLess1 == UINT_MAX) {
          VERIFY_IS_FALSE(alloc.AllocateUnbounded(&e, pos, alignment));
        } else {
          VERIFY_IS_FALSE(alloc.Allocate(&e, sizeLess1 + 1, pos, alignment));
        }
      } else {
        // large enough gap to allocate, verify:
        //  - allocation occurs where expected
        //  - firstFree is advanced if necessary
        //  - container is marked full if necessary (VerifySpans should do this)
        unsigned firstFree = alloc.GetFirstFree();
        const Gap *expectedGap = FindGap(*pGaps, sizeLess1);
        DXASSERT_NOMSG(expectedGap);
        unsigned start = expectedGap->start;
        unsigned end = expectedGap->start + sizeLess1;
        e = Element(UINT_MAX, start, end);
        unsigned pos = 0xFEFEFEFE;
        if (sizeLess1 == UINT_MAX) {
          e.end = expectedGap->end;
          VERIFY_IS_TRUE(alloc.AllocateUnbounded(&e, pos, alignment));
        } else {
          VERIFY_IS_TRUE(alloc.Allocate(&e, sizeLess1 + 1, pos, alignment));
        }
        VERIFY_IS_TRUE(pos == start);
        if (start <= firstFree && firstFree <= end) {
          if (end < expectedGap->end) {
            VERIFY_IS_FALSE(alloc.IsFull());
            VERIFY_IS_TRUE(alloc.GetFirstFree() == end + 1);
          }
        }
      }

      VERIFY_IS_TRUE(scenario.VerifySpans(alloc));
    };

    auto TestSizesFn = [&] {
      DXASSERT_NOMSG(pGaps);
      const Gap *pStartGap = GetGapBeforeFirst(*pGaps);
      const Gap *pGap1 = GetGapBetweenFirstAndSecond(*pGaps);

      // pass/fail based on fit
      // allocate different sizes (in sizeLess1, UINT_MAX means unbounded):
      std::set<unsigned> sizes;
      //  - size 1
      sizes.insert(0);
      //  - size 2
      sizes.insert(1);
      //  - Unbounded
      sizes.insert(UINT_MAX);
      // - size == smallest gap
      if (smallestGap)
        sizes.insert(smallestGap->sizeLess1);
      //  - size == largest gap
      if (largestGap && largestGap->sizeLess1 < UINT_MAX)
        sizes.insert(largestGap->sizeLess1);
      //  - size == largest gap + 1
      if (largestGap && largestGap->sizeLess1 < UINT_MAX - 1)
        sizes.insert(largestGap->sizeLess1 + 1);
      //  - size > gap before first span if largest gap is large enough
      if (pStartGap && pStartGap->sizeLess1 < largestGap->sizeLess1)
        sizes.insert(pStartGap->sizeLess1 + 1);
      //  - size > gap before first span and size > first gap between spans if
      //  largest gap is large enough
      if (pStartGap && pStartGap->sizeLess1 < largestGap->sizeLess1 && pGap1 &&
          pGap1->sizeLess1 < largestGap->sizeLess1)
        sizes.insert(std::max(pStartGap->sizeLess1, pGap1->sizeLess1) + 1);

      for (auto &size : sizes) {
        TestFn(size);
      }
    };

    // Test without alignment
    TestSizesFn();

    // again with alignment
    alignment = 4;
    pGaps = &scenario.gaps[alignment];
    largestGap = scenario.gapLargest[alignment];
    smallestGap = scenario.gapSmallest[alignment];
    TestSizesFn();
  }
}
