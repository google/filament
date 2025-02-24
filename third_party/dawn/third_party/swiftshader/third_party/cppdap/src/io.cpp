// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dap/io.h"

#include <atomic>
#include <condition_variable>
#include <cstdarg>
#include <cstdio>
#include <cstring>  // strlen
#include <deque>
#include <mutex>

namespace {

class Pipe : public dap::ReaderWriter {
 public:
  // dap::ReaderWriter compliance
  bool isOpen() override {
    std::unique_lock<std::mutex> lock(mutex);
    return !closed;
  }

  void close() override {
    std::unique_lock<std::mutex> lock(mutex);
    closed = true;
    cv.notify_all();
  }

  size_t read(void* buffer, size_t bytes) override {
    std::unique_lock<std::mutex> lock(mutex);
    auto out = reinterpret_cast<uint8_t*>(buffer);
    size_t n = 0;
    while (true) {
      cv.wait(lock, [&] { return closed || data.size() > 0; });
      if (closed) {
        return n;
      }
      for (; n < bytes && data.size() > 0; n++) {
        out[n] = data.front();
        data.pop_front();
      }
      if (n == bytes) {
        return n;
      }
    }
  }

  bool write(const void* buffer, size_t bytes) override {
    std::unique_lock<std::mutex> lock(mutex);
    if (closed) {
      return false;
    }
    if (bytes == 0) {
      return true;
    }
    auto notify = data.size() == 0;
    auto src = reinterpret_cast<const uint8_t*>(buffer);
    for (size_t i = 0; i < bytes; i++) {
      data.emplace_back(src[i]);
    }
    if (notify) {
      cv.notify_all();
    }
    return true;
  }

 private:
  std::mutex mutex;
  std::condition_variable cv;
  std::deque<uint8_t> data;
  bool closed = false;
};

class RW : public dap::ReaderWriter {
 public:
  RW(const std::shared_ptr<Reader>& r, const std::shared_ptr<Writer>& w)
      : r(r), w(w) {}

  // dap::ReaderWriter compliance
  bool isOpen() override { return r->isOpen() && w->isOpen(); }
  void close() override {
    r->close();
    w->close();
  }
  size_t read(void* buffer, size_t n) override { return r->read(buffer, n); }
  bool write(const void* buffer, size_t n) override {
    return w->write(buffer, n);
  }

 private:
  const std::shared_ptr<dap::Reader> r;
  const std::shared_ptr<dap::Writer> w;
};

class File : public dap::ReaderWriter {
 public:
  File(FILE* f, bool closable) : f(f), closable(closable) {}

  ~File() { close(); }

  // dap::ReaderWriter compliance
  bool isOpen() override { return !closed; }
  void close() override {
    if (closable) {
      if (!closed.exchange(true)) {
        fclose(f);
      }
    }
  }
  size_t read(void* buffer, size_t n) override {
    std::unique_lock<std::mutex> lock(readMutex);
    auto out = reinterpret_cast<char*>(buffer);
    for (size_t i = 0; i < n; i++) {
      int c = fgetc(f);
      if (c == EOF) {
        return i;
      }
      out[i] = char(c);
    }
    return n;
  }
  bool write(const void* buffer, size_t n) override {
    std::unique_lock<std::mutex> lock(writeMutex);
    if (fwrite(buffer, 1, n, f) == n) {
      fflush(f);
      return true;
    }
    return false;
  }

 private:
  FILE* const f;
  const bool closable;
  std::mutex readMutex;
  std::mutex writeMutex;
  std::atomic<bool> closed = { false };
};

class ReaderSpy : public dap::Reader {
 public:
  ReaderSpy(const std::shared_ptr<dap::Reader>& r,
            const std::shared_ptr<dap::Writer>& s,
            const std::string& prefix)
      : r(r), s(s), prefix(prefix) {}

  // dap::Reader compliance
  bool isOpen() override { return r->isOpen(); }
  void close() override { r->close(); }
  size_t read(void* buffer, size_t n) override {
    auto c = r->read(buffer, n);
    if (c > 0) {
      auto chars = reinterpret_cast<const char*>(buffer);
      std::string buf = prefix;
      buf.append(chars, chars + c);
      s->write(buf.data(), buf.size());
    }
    return c;
  }

 private:
  const std::shared_ptr<dap::Reader> r;
  const std::shared_ptr<dap::Writer> s;
  const std::string prefix;
};

class WriterSpy : public dap::Writer {
 public:
  WriterSpy(const std::shared_ptr<dap::Writer>& w,
            const std::shared_ptr<dap::Writer>& s,
            const std::string& prefix)
      : w(w), s(s), prefix(prefix) {}

  // dap::Writer compliance
  bool isOpen() override { return w->isOpen(); }
  void close() override { w->close(); }
  bool write(const void* buffer, size_t n) override {
    if (!w->write(buffer, n)) {
      return false;
    }
    auto chars = reinterpret_cast<const char*>(buffer);
    std::string buf = prefix;
    buf.append(chars, chars + n);
    s->write(buf.data(), buf.size());
    return true;
  }

 private:
  const std::shared_ptr<dap::Writer> w;
  const std::shared_ptr<dap::Writer> s;
  const std::string prefix;
};

}  // anonymous namespace

namespace dap {

std::shared_ptr<ReaderWriter> ReaderWriter::create(
    const std::shared_ptr<Reader>& r,
    const std::shared_ptr<Writer>& w) {
  return std::make_shared<RW>(r, w);
}

std::shared_ptr<ReaderWriter> pipe() {
  return std::make_shared<Pipe>();
}

std::shared_ptr<ReaderWriter> file(FILE* f, bool closable /* = true */) {
  return std::make_shared<File>(f, closable);
}

std::shared_ptr<ReaderWriter> file(const char* path) {
  if (auto f = fopen(path, "wb")) {
    return std::make_shared<File>(f, true);
  }
  return nullptr;
}

// spy() returns a Reader that copies all reads from the Reader r to the Writer
// s, using the given optional prefix.
std::shared_ptr<Reader> spy(const std::shared_ptr<Reader>& r,
                            const std::shared_ptr<Writer>& s,
                            const char* prefix /* = "\n<-" */) {
  return std::make_shared<ReaderSpy>(r, s, prefix);
}

// spy() returns a Writer that copies all writes to the Writer w to the Writer
// s, using the given optional prefix.
std::shared_ptr<Writer> spy(const std::shared_ptr<Writer>& w,
                            const std::shared_ptr<Writer>& s,
                            const char* prefix /* = "\n->" */) {
  return std::make_shared<WriterSpy>(w, s, prefix);
}

bool writef(const std::shared_ptr<Writer>& w, const char* msg, ...) {
  char buf[2048];

  va_list vararg;
  va_start(vararg, msg);
  vsnprintf(buf, sizeof(buf), msg, vararg);
  va_end(vararg);

  return w->write(buf, strlen(buf));
}

}  // namespace dap