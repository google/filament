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

#ifndef dap_io_h
#define dap_io_h

#include <stddef.h>  // size_t
#include <memory>    // std::unique_ptr
#include <utility>   // std::pair

namespace dap {

class Closable {
 public:
  virtual ~Closable() = default;

  // isOpen() returns true if the stream has not been closed.
  virtual bool isOpen() = 0;

  // close() closes the stream.
  virtual void close() = 0;
};

// Reader is an interface for reading from a byte stream.
class Reader : virtual public Closable {
 public:
  // read() attempts to read at most n bytes into buffer, returning the number
  // of bytes read.
  // read() will block until the stream is closed or at least one byte is read.
  virtual size_t read(void* buffer, size_t n) = 0;
};

// Writer is an interface for writing to a byte stream.
class Writer : virtual public Closable {
 public:
  // write() writes n bytes from buffer into the stream.
  // Returns true on success, or false if there was an error or the stream was
  // closed.
  virtual bool write(const void* buffer, size_t n) = 0;
};

// ReaderWriter is an interface that combines the Reader and Writer interfaces.
class ReaderWriter : public Reader, public Writer {
 public:
  // create() returns a ReaderWriter that delegates the interface methods on to
  // the provided Reader and Writer.
  // isOpen() returns true if the Reader and Writer both return true for
  // isOpen().
  // close() closes both the Reader and Writer.
  static std::shared_ptr<ReaderWriter> create(const std::shared_ptr<Reader>&,
                                              const std::shared_ptr<Writer>&);
};

// pipe() returns a ReaderWriter where the Writer streams to the Reader.
// Writes are internally buffered.
// Calling close() on either the Reader or Writer will close both ends of the
// stream.
std::shared_ptr<ReaderWriter> pipe();

// file() wraps file with a ReaderWriter.
// If closable is false, then a call to ReaderWriter::close() will not close the
// underlying file.
std::shared_ptr<ReaderWriter> file(FILE* file, bool closable = true);

// file() opens (or creates) the file with the given path.
std::shared_ptr<ReaderWriter> file(const char* path);

// spy() returns a Reader that copies all reads from the Reader r to the Writer
// s, using the given optional prefix.
std::shared_ptr<Reader> spy(const std::shared_ptr<Reader>& r,
                            const std::shared_ptr<Writer>& s,
                            const char* prefix = "\n->");

// spy() returns a Writer that copies all writes to the Writer w to the Writer
// s, using the given optional prefix.
std::shared_ptr<Writer> spy(const std::shared_ptr<Writer>& w,
                            const std::shared_ptr<Writer>& s,
                            const char* prefix = "\n<-");

// writef writes the printf style string to the writer w.
bool writef(const std::shared_ptr<Writer>& w, const char* msg, ...);

}  // namespace dap

#endif  // dap_io_h
