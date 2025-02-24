// Copyright 2016 Google Inc. All rights reserved.
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

#include "src/mutator.h"

#include <algorithm>
#include <bitset>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "src/field_instance.h"
#include "src/utf8_fix.h"
#include "src/weighted_reservoir_sampler.h"

namespace protobuf_mutator {

using google::protobuf::Any;
using protobuf::Descriptor;
using protobuf::FieldDescriptor;
using protobuf::FileDescriptor;
using protobuf::Message;
using protobuf::OneofDescriptor;
using protobuf::Reflection;
using protobuf::util::MessageDifferencer;
using std::placeholders::_1;

namespace {

const int kMaxInitializeDepth = 200;
const uint64_t kDefaultMutateWeight = 1000000;

enum class Mutation : uint8_t {
  None,
  Add,     // Adds new field with default value.
  Mutate,  // Mutates field contents.
  Delete,  // Deletes field.
  Copy,    // Copy values copied from another field.
  Clone,   // Create new field with value copied from another.

  Last = Clone,
};

using MutationBitset = std::bitset<static_cast<size_t>(Mutation::Last) + 1>;

using Messages = std::vector<Message*>;
using ConstMessages = std::vector<const Message*>;

// Return random integer from [0, count)
size_t GetRandomIndex(RandomEngine* random, size_t count) {
  assert(count > 0);
  if (count == 1) return 0;
  return std::uniform_int_distribution<size_t>(0, count - 1)(*random);
}

// Flips random bit in the buffer.
void FlipBit(size_t size, uint8_t* bytes, RandomEngine* random) {
  size_t bit = GetRandomIndex(random, size * 8);
  bytes[bit / 8] ^= (1u << (bit % 8));
}

// Flips random bit in the value.
template <class T>
T FlipBit(T value, RandomEngine* random) {
  FlipBit(sizeof(value), reinterpret_cast<uint8_t*>(&value), random);
  return value;
}

// Return true with probability about 1-of-n.
bool GetRandomBool(RandomEngine* random, size_t n = 2) {
  return GetRandomIndex(random, n) == 0;
}

bool IsProto3SimpleField(const FieldDescriptor& field) {
  assert(field.file()->syntax() == FileDescriptor::SYNTAX_PROTO3 ||
         field.file()->syntax() == FileDescriptor::SYNTAX_PROTO2);
  return field.file()->syntax() == FileDescriptor::SYNTAX_PROTO3 &&
         field.cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE &&
         !field.containing_oneof() && !field.is_repeated();
}

struct CreateDefaultField : public FieldFunction<CreateDefaultField> {
  template <class T>
  void ForType(const FieldInstance& field) const {
    T value;
    field.GetDefault(&value);
    field.Create(value);
  }
};

struct DeleteField : public FieldFunction<DeleteField> {
  template <class T>
  void ForType(const FieldInstance& field) const {
    field.Delete();
  }
};

struct CopyField : public FieldFunction<CopyField> {
  template <class T>
  void ForType(const ConstFieldInstance& source,
               const FieldInstance& field) const {
    T value;
    source.Load(&value);
    field.Store(value);
  }
};

struct AppendField : public FieldFunction<AppendField> {
  template <class T>
  void ForType(const ConstFieldInstance& source,
               const FieldInstance& field) const {
    T value;
    source.Load(&value);
    field.Create(value);
  }
};

class CanCopyAndDifferentField
    : public FieldFunction<CanCopyAndDifferentField, bool> {
 public:
  template <class T>
  bool ForType(const ConstFieldInstance& src, const ConstFieldInstance& dst,
               int size_increase_hint) const {
    T s;
    src.Load(&s);
    if (!dst.CanStore(s)) return false;
    T d;
    dst.Load(&d);
    return SizeDiff(s, d) <= size_increase_hint && !IsEqual(s, d);
  }

 private:
  bool IsEqual(const ConstFieldInstance::Enum& a,
               const ConstFieldInstance::Enum& b) const {
    assert(a.count == b.count);
    return a.index == b.index;
  }

  bool IsEqual(const std::unique_ptr<Message>& a,
               const std::unique_ptr<Message>& b) const {
    return MessageDifferencer::Equals(*a, *b);
  }

  template <class T>
  bool IsEqual(const T& a, const T& b) const {
    return a == b;
  }

  int64_t SizeDiff(const std::unique_ptr<Message>& src,
                   const std::unique_ptr<Message>& dst) const {
    return src->ByteSizeLong() - dst->ByteSizeLong();
  }

  int64_t SizeDiff(const std::string& src, const std::string& dst) const {
    return src.size() - dst.size();
  }

  template <class T>
  int64_t SizeDiff(const T&, const T&) const {
    return 0;
  }
};

// Selects random field and mutation from the given proto message.
class MutationSampler {
 public:
  MutationSampler(bool keep_initialized, MutationBitset allowed_mutations,
                  RandomEngine* random)
      : keep_initialized_(keep_initialized),
        allowed_mutations_(allowed_mutations),
        random_(random),
        sampler_(random) {}

  // Returns selected field.
  const FieldInstance& field() const { return sampler_.selected().field; }

  // Returns selected mutation.
  Mutation mutation() const { return sampler_.selected().mutation; }

  void Sample(Message* message) {
    SampleImpl(message);
    assert(mutation() != Mutation::None ||
           !allowed_mutations_[static_cast<size_t>(Mutation::Mutate)] ||
           message->GetDescriptor()->field_count() == 0);
  }

 private:
  void SampleImpl(Message* message) {
    const Descriptor* descriptor = message->GetDescriptor();
    const Reflection* reflection = message->GetReflection();

    int field_count = descriptor->field_count();
    for (int i = 0; i < field_count; ++i) {
      const FieldDescriptor* field = descriptor->field(i);
      if (const OneofDescriptor* oneof = field->containing_oneof()) {
        // Handle entire oneof group on the first field.
        if (field->index_in_oneof() == 0) {
          assert(oneof->field_count());
          const FieldDescriptor* current_field =
              reflection->GetOneofFieldDescriptor(*message, oneof);
          for (;;) {
            const FieldDescriptor* add_field =
                oneof->field(GetRandomIndex(random_, oneof->field_count()));
            if (add_field != current_field) {
              Try({message, add_field}, Mutation::Add);
              Try({message, add_field}, Mutation::Clone);
              break;
            }
            if (oneof->field_count() < 2) break;
          }
          if (current_field) {
            if (current_field->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE)
              Try({message, current_field}, Mutation::Mutate);
            Try({message, current_field}, Mutation::Delete);
            Try({message, current_field}, Mutation::Copy);
          }
        }
      } else {
        if (field->is_repeated()) {
          int field_size = reflection->FieldSize(*message, field);
          size_t random_index = GetRandomIndex(random_, field_size + 1);
          Try({message, field, random_index}, Mutation::Add);
          Try({message, field, random_index}, Mutation::Clone);

          if (field_size) {
            size_t random_index = GetRandomIndex(random_, field_size);
            if (field->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE)
              Try({message, field, random_index}, Mutation::Mutate);
            Try({message, field, random_index}, Mutation::Delete);
            Try({message, field, random_index}, Mutation::Copy);
          }
        } else {
          if (reflection->HasField(*message, field) ||
              IsProto3SimpleField(*field)) {
            if (field->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE)
              Try({message, field}, Mutation::Mutate);
            if (!IsProto3SimpleField(*field) &&
                (!field->is_required() || !keep_initialized_)) {
              Try({message, field}, Mutation::Delete);
            }
            Try({message, field}, Mutation::Copy);
          } else {
            Try({message, field}, Mutation::Add);
            Try({message, field}, Mutation::Clone);
          }
        }
      }

      if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
        if (field->is_repeated()) {
          const int field_size = reflection->FieldSize(*message, field);
          for (int j = 0; j < field_size; ++j)
            SampleImpl(reflection->MutableRepeatedMessage(message, field, j));
        } else if (reflection->HasField(*message, field)) {
          SampleImpl(reflection->MutableMessage(message, field));
        }
      }
    }
  }

  void Try(const FieldInstance& field, Mutation mutation) {
    assert(mutation != Mutation::None);
    if (!allowed_mutations_[static_cast<size_t>(mutation)]) return;
    sampler_.Try(kDefaultMutateWeight, {field, mutation});
  }

  bool keep_initialized_ = false;
  MutationBitset allowed_mutations_;

  RandomEngine* random_;

  struct Result {
    Result() = default;
    Result(const FieldInstance& f, Mutation m) : field(f), mutation(m) {}

    FieldInstance field;
    Mutation mutation = Mutation::None;
  };
  WeightedReservoirSampler<Result, RandomEngine> sampler_;
};

// Selects random field of compatible type to use for clone mutations.
class DataSourceSampler {
 public:
  DataSourceSampler(const ConstFieldInstance& match, RandomEngine* random,
                    int size_increase_hint)
      : match_(match),
        random_(random),
        size_increase_hint_(size_increase_hint),
        sampler_(random) {}

  void Sample(const Message& message) { SampleImpl(message); }

  // Returns selected field.
  const ConstFieldInstance& field() const {
    assert(!IsEmpty());
    return sampler_.selected();
  }

  bool IsEmpty() const { return sampler_.IsEmpty(); }

 private:
  void SampleImpl(const Message& message) {
    const Descriptor* descriptor = message.GetDescriptor();
    const Reflection* reflection = message.GetReflection();

    int field_count = descriptor->field_count();
    for (int i = 0; i < field_count; ++i) {
      const FieldDescriptor* field = descriptor->field(i);
      if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
        if (field->is_repeated()) {
          const int field_size = reflection->FieldSize(message, field);
          for (int j = 0; j < field_size; ++j) {
            SampleImpl(reflection->GetRepeatedMessage(message, field, j));
          }
        } else if (reflection->HasField(message, field)) {
          SampleImpl(reflection->GetMessage(message, field));
        }
      }

      if (field->cpp_type() != match_.cpp_type()) continue;
      if (match_.cpp_type() == FieldDescriptor::CPPTYPE_ENUM) {
        if (field->enum_type() != match_.enum_type()) continue;
      } else if (match_.cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
        if (field->message_type() != match_.message_type()) continue;
      }

      if (field->is_repeated()) {
        if (int field_size = reflection->FieldSize(message, field)) {
          ConstFieldInstance source(&message, field,
                                    GetRandomIndex(random_, field_size));
          if (CanCopyAndDifferentField()(source, match_, size_increase_hint_))
            sampler_.Try(field_size, source);
        }
      } else {
        if (reflection->HasField(message, field)) {
          ConstFieldInstance source(&message, field);
          if (CanCopyAndDifferentField()(source, match_, size_increase_hint_))
            sampler_.Try(1, source);
        }
      }
    }
  }

  ConstFieldInstance match_;
  RandomEngine* random_;
  int size_increase_hint_;

  WeightedReservoirSampler<ConstFieldInstance, RandomEngine> sampler_;
};

using UnpackedAny =
    std::unordered_map<const Message*, std::unique_ptr<Message>>;

const Descriptor* GetAnyTypeDescriptor(const Any& any) {
  std::string type_name;
  if (!Any::ParseAnyTypeUrl(std::string(any.type_url()), &type_name))
    return nullptr;
  return any.descriptor()->file()->pool()->FindMessageTypeByName(type_name);
}

std::unique_ptr<Message> UnpackAny(const Any& any) {
  const Descriptor* desc = GetAnyTypeDescriptor(any);
  if (!desc) return {};
  std::unique_ptr<Message> message(
      any.GetReflection()->GetMessageFactory()->GetPrototype(desc)->New());
  message->ParsePartialFromString(std::string(any.value()));
  return message;
}

const Any* CastToAny(const Message* message) {
  return Any::GetDescriptor() == message->GetDescriptor()
             ? static_cast<const Any*>(message)
             : nullptr;
}

Any* CastToAny(Message* message) {
  return Any::GetDescriptor() == message->GetDescriptor()
             ? static_cast<Any*>(message)
             : nullptr;
}

std::unique_ptr<Message> UnpackIfAny(const Message& message) {
  if (const Any* any = CastToAny(&message)) return UnpackAny(*any);
  return {};
}

void UnpackAny(const Message& message, UnpackedAny* result) {
  if (std::unique_ptr<Message> any = UnpackIfAny(message)) {
    UnpackAny(*any, result);
    result->emplace(&message, std::move(any));
    return;
  }

  const Descriptor* descriptor = message.GetDescriptor();
  const Reflection* reflection = message.GetReflection();

  for (int i = 0; i < descriptor->field_count(); ++i) {
    const FieldDescriptor* field = descriptor->field(i);
    if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
      if (field->is_repeated()) {
        const int field_size = reflection->FieldSize(message, field);
        for (int j = 0; j < field_size; ++j) {
          UnpackAny(reflection->GetRepeatedMessage(message, field, j), result);
        }
      } else if (reflection->HasField(message, field)) {
        UnpackAny(reflection->GetMessage(message, field), result);
      }
    }
  }
}

class PostProcessing {
 public:
  using PostProcessors =
      std::unordered_multimap<const Descriptor*, Mutator::PostProcess>;

  PostProcessing(bool keep_initialized, const PostProcessors& post_processors,
                 const UnpackedAny& any, RandomEngine* random)
      : keep_initialized_(keep_initialized),
        post_processors_(post_processors),
        any_(any),
        random_(random) {}

  void Run(Message* message, int max_depth) {
    --max_depth;
    const Descriptor* descriptor = message->GetDescriptor();

    // Apply custom mutators in nested messages before packing any.
    const Reflection* reflection = message->GetReflection();
    for (int i = 0; i < descriptor->field_count(); i++) {
      const FieldDescriptor* field = descriptor->field(i);
      if (keep_initialized_ &&
          (field->is_required() || descriptor->options().map_entry()) &&
          !reflection->HasField(*message, field)) {
        CreateDefaultField()(FieldInstance(message, field));
      }

      if (field->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE) continue;

      if (max_depth < 0 && !field->is_required()) {
        // Clear deep optional fields to avoid stack overflow.
        reflection->ClearField(message, field);
        if (field->is_repeated())
          assert(!reflection->FieldSize(*message, field));
        else
          assert(!reflection->HasField(*message, field));
        continue;
      }

      if (field->is_repeated()) {
        const int field_size = reflection->FieldSize(*message, field);
        for (int j = 0; j < field_size; ++j) {
          Message* nested_message =
              reflection->MutableRepeatedMessage(message, field, j);
          Run(nested_message, max_depth);
        }
      } else if (reflection->HasField(*message, field)) {
        Message* nested_message = reflection->MutableMessage(message, field);
        Run(nested_message, max_depth);
      }
    }

    if (Any* any = CastToAny(message)) {
      if (max_depth < 0) {
        // Clear deep Any fields to avoid stack overflow.
        any->Clear();
      } else {
        auto It = any_.find(message);
        if (It != any_.end()) {
          Run(It->second.get(), max_depth);
          std::string value;
          It->second->SerializePartialToString(&value);
          *any->mutable_value() = value;
        }
      }
    }

    // Call user callback after message trimmed, initialized and packed.
    auto range = post_processors_.equal_range(descriptor);
    for (auto it = range.first; it != range.second; ++it)
      it->second(message, (*random_)());
  }

 private:
  bool keep_initialized_;
  const PostProcessors& post_processors_;
  const UnpackedAny& any_;
  RandomEngine* random_;
};

}  // namespace

class FieldMutator {
 public:
  FieldMutator(int size_increase_hint, bool enforce_changes,
               bool enforce_utf8_strings, const ConstMessages& sources,
               Mutator* mutator)
      : size_increase_hint_(size_increase_hint),
        enforce_changes_(enforce_changes),
        enforce_utf8_strings_(enforce_utf8_strings),
        sources_(sources),
        mutator_(mutator) {}

  void Mutate(int32_t* value) const {
    RepeatMutate(value, std::bind(&Mutator::MutateInt32, mutator_, _1));
  }

  void Mutate(int64_t* value) const {
    RepeatMutate(value, std::bind(&Mutator::MutateInt64, mutator_, _1));
  }

  void Mutate(uint32_t* value) const {
    RepeatMutate(value, std::bind(&Mutator::MutateUInt32, mutator_, _1));
  }

  void Mutate(uint64_t* value) const {
    RepeatMutate(value, std::bind(&Mutator::MutateUInt64, mutator_, _1));
  }

  void Mutate(float* value) const {
    RepeatMutate(value, std::bind(&Mutator::MutateFloat, mutator_, _1));
  }

  void Mutate(double* value) const {
    RepeatMutate(value, std::bind(&Mutator::MutateDouble, mutator_, _1));
  }

  void Mutate(bool* value) const {
    RepeatMutate(value, std::bind(&Mutator::MutateBool, mutator_, _1));
  }

  void Mutate(FieldInstance::Enum* value) const {
    RepeatMutate(&value->index,
                 std::bind(&Mutator::MutateEnum, mutator_, _1, value->count));
    assert(value->index < value->count);
  }

  void Mutate(std::string* value) const {
    if (enforce_utf8_strings_) {
      RepeatMutate(value, std::bind(&Mutator::MutateUtf8String, mutator_, _1,
                                    size_increase_hint_));
    } else {
      RepeatMutate(value, std::bind(&Mutator::MutateString, mutator_, _1,
                                    size_increase_hint_));
    }
  }

  void Mutate(std::unique_ptr<Message>* message) const {
    assert(!enforce_changes_);
    assert(*message);
    if (GetRandomBool(mutator_->random(), mutator_->random_to_default_ratio_))
      return;
    mutator_->MutateImpl(sources_, {message->get()}, false,
                         size_increase_hint_);
  }

 private:
  template <class T, class F>
  void RepeatMutate(T* value, F mutate) const {
    if (!enforce_changes_ &&
        GetRandomBool(mutator_->random(), mutator_->random_to_default_ratio_)) {
      return;
    }
    T tmp = *value;
    for (int i = 0; i < 10; ++i) {
      *value = mutate(*value);
      if (!enforce_changes_ || *value != tmp) return;
    }
  }

  int size_increase_hint_;
  size_t enforce_changes_;
  bool enforce_utf8_strings_;
  const ConstMessages& sources_;
  Mutator* mutator_;
};

namespace {

struct MutateField : public FieldFunction<MutateField> {
  template <class T>
  void ForType(const FieldInstance& field, int size_increase_hint,
               const ConstMessages& sources, Mutator* mutator) const {
    T value;
    field.Load(&value);
    FieldMutator(size_increase_hint, true, field.EnforceUtf8(), sources,
                 mutator)
        .Mutate(&value);
    field.Store(value);
  }
};

struct CreateField : public FieldFunction<CreateField> {
 public:
  template <class T>
  void ForType(const FieldInstance& field, int size_increase_hint,
               const ConstMessages& sources, Mutator* mutator) const {
    T value;
    field.GetDefault(&value);
    FieldMutator field_mutator(size_increase_hint,
                               false /* defaults could be useful */,
                               field.EnforceUtf8(), sources, mutator);
    field_mutator.Mutate(&value);
    field.Create(value);
  }
};

}  // namespace

void Mutator::Seed(uint32_t value) { random_.seed(value); }

void Mutator::Fix(Message* message) {
  UnpackedAny any;
  UnpackAny(*message, &any);

  PostProcessing(keep_initialized_, post_processors_, any, &random_)
      .Run(message, kMaxInitializeDepth);
  assert(IsInitialized(*message));
}

void Mutator::Mutate(Message* message, size_t max_size_hint) {
  UnpackedAny any;
  UnpackAny(*message, &any);

  Messages messages;
  messages.reserve(any.size() + 1);
  messages.push_back(message);
  for (const auto& kv : any) messages.push_back(kv.second.get());

  ConstMessages sources(messages.begin(), messages.end());
  MutateImpl(sources, messages, false,
             static_cast<int>(max_size_hint) -
                 static_cast<int>(message->ByteSizeLong()));

  PostProcessing(keep_initialized_, post_processors_, any, &random_)
      .Run(message, kMaxInitializeDepth);
  assert(IsInitialized(*message));
}

void Mutator::CrossOver(const Message& message1, Message* message2,
                        size_t max_size_hint) {
  UnpackedAny any;
  UnpackAny(*message2, &any);

  Messages messages;
  messages.reserve(any.size() + 1);
  messages.push_back(message2);
  for (auto& kv : any) messages.push_back(kv.second.get());

  UnpackAny(message1, &any);

  ConstMessages sources;
  sources.reserve(any.size() + 2);
  sources.push_back(&message1);
  sources.push_back(message2);
  for (const auto& kv : any) sources.push_back(kv.second.get());

  MutateImpl(sources, messages, true,
             static_cast<int>(max_size_hint) -
                 static_cast<int>(message2->ByteSizeLong()));

  PostProcessing(keep_initialized_, post_processors_, any, &random_)
      .Run(message2, kMaxInitializeDepth);
  assert(IsInitialized(*message2));
}

void Mutator::RegisterPostProcessor(const Descriptor* desc,
                                    PostProcess callback) {
  post_processors_.emplace(desc, callback);
}

bool Mutator::MutateImpl(const ConstMessages& sources, const Messages& messages,
                         bool copy_clone_only, int size_increase_hint) {
  MutationBitset mutations;
  if (copy_clone_only) {
    mutations[static_cast<size_t>(Mutation::Copy)] = true;
    mutations[static_cast<size_t>(Mutation::Clone)] = true;
  } else if (size_increase_hint <= 16) {
    mutations[static_cast<size_t>(Mutation::Delete)] = true;
  } else {
    mutations.set();
    mutations[static_cast<size_t>(Mutation::Copy)] = false;
    mutations[static_cast<size_t>(Mutation::Clone)] = false;
  }
  while (mutations.any()) {
    MutationSampler mutation(keep_initialized_, mutations, &random_);
    for (Message* message : messages) mutation.Sample(message);

    switch (mutation.mutation()) {
      case Mutation::None:
        return true;
      case Mutation::Add:
        CreateField()(mutation.field(), size_increase_hint, sources, this);
        return true;
      case Mutation::Mutate:
        MutateField()(mutation.field(), size_increase_hint, sources, this);
        return true;
      case Mutation::Delete:
        DeleteField()(mutation.field());
        return true;
      case Mutation::Clone: {
        CreateDefaultField()(mutation.field());
        DataSourceSampler source_sampler(mutation.field(), &random_,
                                         size_increase_hint);
        for (const Message* source : sources) source_sampler.Sample(*source);
        if (source_sampler.IsEmpty()) {
          if (!IsProto3SimpleField(*mutation.field().descriptor()))
            return true;  // CreateField is enough for proto2.
          break;
        }
        CopyField()(source_sampler.field(), mutation.field());
        return true;
      }
      case Mutation::Copy: {
        DataSourceSampler source_sampler(mutation.field(), &random_,
                                         size_increase_hint);
        for (const Message* source : sources) source_sampler.Sample(*source);
        if (source_sampler.IsEmpty()) break;
        CopyField()(source_sampler.field(), mutation.field());
        return true;
      }
      default:
        assert(false && "unexpected mutation");
        return false;
    }

    // Don't try same mutation next time.
    mutations[static_cast<size_t>(mutation.mutation())] = false;
  }
  return false;
}

int32_t Mutator::MutateInt32(int32_t value) { return FlipBit(value, &random_); }

int64_t Mutator::MutateInt64(int64_t value) { return FlipBit(value, &random_); }

uint32_t Mutator::MutateUInt32(uint32_t value) {
  return FlipBit(value, &random_);
}

uint64_t Mutator::MutateUInt64(uint64_t value) {
  return FlipBit(value, &random_);
}

float Mutator::MutateFloat(float value) { return FlipBit(value, &random_); }

double Mutator::MutateDouble(double value) { return FlipBit(value, &random_); }

bool Mutator::MutateBool(bool value) { return !value; }

size_t Mutator::MutateEnum(size_t index, size_t item_count) {
  if (item_count <= 1) return 0;
  return (index + 1 + GetRandomIndex(&random_, item_count - 1)) % item_count;
}

std::string Mutator::MutateString(const std::string& value,
                                  int size_increase_hint) {
  std::string result = value;

  while (!result.empty() && GetRandomBool(&random_)) {
    result.erase(GetRandomIndex(&random_, result.size()), 1);
  }

  while (size_increase_hint > 0 &&
         result.size() < static_cast<size_t>(size_increase_hint) &&
         GetRandomBool(&random_)) {
    size_t index = GetRandomIndex(&random_, result.size() + 1);
    result.insert(result.begin() + index, GetRandomIndex(&random_, 1 << 8));
  }

  if (result != value) return result;

  if (result.empty()) {
    result.push_back(GetRandomIndex(&random_, 1 << 8));
    return result;
  }

  if (!result.empty())
    FlipBit(result.size(), reinterpret_cast<uint8_t*>(&result[0]), &random_);
  return result;
}

std::string Mutator::MutateUtf8String(const std::string& value,
                                      int size_increase_hint) {
  std::string str = MutateString(value, size_increase_hint);
  FixUtf8String(&str, &random_);
  return str;
}

bool Mutator::IsInitialized(const Message& message) const {
  if (!keep_initialized_ || message.IsInitialized()) return true;
  std::cerr << "Uninitialized: " << message.DebugString() << "\n";
  return false;
}

}  // namespace protobuf_mutator
