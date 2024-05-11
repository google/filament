#include "draco/io/file_writer_factory.h"

#include <cstdint>
#include <memory>
#include <string>

#include "draco/core/draco_test_base.h"
#include "draco/io/file_writer_interface.h"

namespace draco {
namespace {

class AlwaysFailFileWriter : public FileWriterInterface {
 public:
  static std::unique_ptr<FileWriterInterface> Open(
      const std::string & /*file_name*/) {
    return nullptr;
  }

  AlwaysFailFileWriter() = delete;
  AlwaysFailFileWriter(const AlwaysFailFileWriter &) = delete;
  AlwaysFailFileWriter &operator=(const AlwaysFailFileWriter &) = delete;
  // Note this isn't overridden as the class can never be instantiated. This
  // avoids an unused function warning.
  // ~AlwaysFailFileWriter() override = default;

  bool Write(const char * /*buffer*/, size_t /*size*/) override {
    return false;
  }

 private:
  static bool is_registered_;
};

class AlwaysOkFileWriter : public FileWriterInterface {
 public:
  static std::unique_ptr<FileWriterInterface> Open(
      const std::string & /*file_name*/) {
    return std::unique_ptr<AlwaysOkFileWriter>(new AlwaysOkFileWriter());
  }

  AlwaysOkFileWriter(const AlwaysOkFileWriter &) = delete;
  AlwaysOkFileWriter &operator=(const AlwaysOkFileWriter &) = delete;
  ~AlwaysOkFileWriter() override = default;

  bool Write(const char * /*buffer*/, size_t /*size*/) override { return true; }

 private:
  AlwaysOkFileWriter() = default;
  static bool is_registered_;
};

bool AlwaysFailFileWriter::is_registered_ =
    FileWriterFactory::RegisterWriter(AlwaysFailFileWriter::Open);

bool AlwaysOkFileWriter::is_registered_ =
    FileWriterFactory::RegisterWriter(AlwaysOkFileWriter::Open);

TEST(FileWriterFactoryTest, RegistrationFail) {
  EXPECT_FALSE(FileWriterFactory::RegisterWriter(nullptr));
}

TEST(FileWriterFactoryTest, OpenWriter) {
  auto writer = FileWriterFactory::OpenWriter("fake file");
  EXPECT_NE(writer, nullptr);
  EXPECT_TRUE(writer->Write(nullptr, 0u));
}

}  // namespace
}  // namespace draco
