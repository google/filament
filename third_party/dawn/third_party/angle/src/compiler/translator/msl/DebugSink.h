//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <iostream>

#include "compiler/translator/InfoSink.h"

namespace sh
{

class StringObserver
{
  public:
    StringObserver(const std::string &needle) : needle(needle) { ASSERT(!needle.empty()); }

    bool observe(char c)
    {
        if (needle[currPos] == c)
        {
            ++currPos;
            if (currPos == needle.size())
            {
                reset();
                return true;
            }
        }
        else
        {
            reset();
        }
        return false;
    }

    const std::string &getNeedle() const { return needle; }

    void reset() { currPos = 0; }

  private:
    std::string needle;
    size_t currPos = 0;
};

class DebugSink : angle::NonCopyable
{
  public:
    friend class EscapedSink;
    class EscapedSink : angle::NonCopyable
    {
        friend class DebugSink;

      private:
        EscapedSink(DebugSink &owner) : mOwner(owner), mBegin(owner.size()) {}

      public:
        EscapedSink(EscapedSink &&other) : mOwner(other.mOwner), mBegin(other.mBegin) {}

        ~EscapedSink()
        {
            const char *p = mOwner.c_str();
            const int end = mOwner.size();
            mOwner.onWrite(p + mBegin, p + end);
        }

        TInfoSinkBase &get() { return mOwner.mParent; }

        operator TInfoSinkBase &() { return get(); }

      private:
        DebugSink &mOwner;
        const int mBegin;
    };

  public:
    DebugSink(TInfoSinkBase &parent, bool alsoLogToStdout)
        : mParent(parent), mAlsoLogToStdout(alsoLogToStdout)
    {}

    void watch(std::string const &needle)
    {
        if (!needle.empty())
        {
            mObservers.emplace_back(needle);
        }
    }

    void erase()
    {
        mParent.erase();
        for (StringObserver &observer : mObservers)
        {
            observer.reset();
        }
    }

    int size() { return mParent.size(); }

    const TPersistString &str() const { return mParent.str(); }

    const char *c_str() const { return mParent.c_str(); }

    EscapedSink escape() { return EscapedSink(*this); }

    template <typename T>
    DebugSink &operator<<(const T &value)
    {
        const size_t begin = mParent.size();
        mParent << value;
        const size_t end = mParent.size();

        const char *p = mParent.c_str();
        onWrite(p + begin, p + end);

        return *this;
    }

  private:
    void onWrite(const char *begin, char const *end)
    {
        const char *p = begin;
        while (p != end)
        {
            if (mAlsoLogToStdout)
            {
                std::cout << *p;
            }

            for (StringObserver &observer : mObservers)
            {
                if (observer.observe(*p))
                {
                    if (mAlsoLogToStdout)
                    {
                        std::cout.flush();
                    }
                    const std::string &needle = observer.getNeedle();
                    (void)needle;
                    ASSERT(true);  // place your breakpoint here
                }
            }

            ++p;
        }

        if (mAlsoLogToStdout)
        {
            std::cout.flush();
        }
    }

  private:
    TInfoSinkBase &mParent;
    std::vector<StringObserver> mObservers;
    bool mAlsoLogToStdout;
};

}  // namespace sh
