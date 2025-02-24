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

#include "content_stream.h"

#include "dap/any.h"
#include "dap/session.h"

#include "chan.h"
#include "json_serializer.h"
#include "socket.h"

#include <stdarg.h>
#include <stdio.h>
#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

namespace {

class Impl : public dap::Session {
 public:
  void onError(const ErrorHandler& handler) override { handlers.put(handler); }

  void registerHandler(const dap::TypeInfo* typeinfo,
                       const GenericRequestHandler& handler) override {
    handlers.put(typeinfo, handler);
  }

  void registerHandler(const dap::TypeInfo* typeinfo,
                       const GenericEventHandler& handler) override {
    handlers.put(typeinfo, handler);
  }

  void registerHandler(const dap::TypeInfo* typeinfo,
                       const GenericResponseSentHandler& handler) override {
    handlers.put(typeinfo, handler);
  }

  void bind(const std::shared_ptr<dap::Reader>& r,
            const std::shared_ptr<dap::Writer>& w) override {
    if (isBound.exchange(true)) {
      handlers.error("Session is already bound!");
      return;
    }

    reader = dap::ContentReader(r);
    writer = dap::ContentWriter(w);

    recvThread = std::thread([this] {
      while (reader.isOpen()) {
        auto request = reader.read();
        if (request.size() > 0) {
          if (auto payload = processMessage(request)) {
            inbox.put(std::move(payload));
          }
        }
      }
    });

    dispatchThread = std::thread([this] {
      while (auto payload = inbox.take()) {
        payload.value()();
      }
    });
  }

  bool send(const dap::TypeInfo* requestTypeInfo,
            const dap::TypeInfo* responseTypeInfo,
            const void* request,
            const GenericResponseHandler& responseHandler) override {
    int seq = nextSeq++;

    handlers.put(seq, responseTypeInfo, responseHandler);

    dap::json::Serializer s;
    if (!s.object([&](dap::FieldSerializer* fs) {
          return fs->field("seq", dap::integer(seq)) &&
                 fs->field("type", "request") &&
                 fs->field("command", requestTypeInfo->name()) &&
                 fs->field("arguments", [&](dap::Serializer* s) {
                   return requestTypeInfo->serialize(s, request);
                 });
        })) {
      return false;
    }
    return send(s.dump());
  }

  bool send(const dap::TypeInfo* typeinfo, const void* event) override {
    dap::json::Serializer s;
    if (!s.object([&](dap::FieldSerializer* fs) {
          return fs->field("seq", dap::integer(nextSeq++)) &&
                 fs->field("type", "event") &&
                 fs->field("event", typeinfo->name()) &&
                 fs->field("body", [&](dap::Serializer* s) {
                   return typeinfo->serialize(s, event);
                 });
        })) {
      return false;
    }
    return send(s.dump());
  }

  ~Impl() {
    inbox.close();
    reader.close();
    writer.close();
    if (recvThread.joinable()) {
      recvThread.join();
    }
    if (dispatchThread.joinable()) {
      dispatchThread.join();
    }
  }

 private:
  using Payload = std::function<void()>;

  class EventHandlers {
   public:
    void put(const ErrorHandler& handler) {
      std::unique_lock<std::mutex> lock(errorMutex);
      errorHandler = handler;
    }

    void error(const char* format, ...) {
      va_list vararg;
      va_start(vararg, format);
      std::unique_lock<std::mutex> lock(errorMutex);
      errorLocked(format, vararg);
      va_end(vararg);
    }

    std::pair<const dap::TypeInfo*, GenericRequestHandler> request(
        const std::string& name) {
      std::unique_lock<std::mutex> lock(requestMutex);
      auto it = requestMap.find(name);
      return (it != requestMap.end()) ? it->second : decltype(it->second){};
    }

    void put(const dap::TypeInfo* typeinfo,
             const GenericRequestHandler& handler) {
      std::unique_lock<std::mutex> lock(requestMutex);
      auto added =
          requestMap
              .emplace(typeinfo->name(), std::make_pair(typeinfo, handler))
              .second;
      if (!added) {
        errorfLocked("Request handler for '%s' already registered",
                     typeinfo->name().c_str());
      }
    }

    std::pair<const dap::TypeInfo*, GenericResponseHandler> response(
        int64_t seq) {
      std::unique_lock<std::mutex> lock(responseMutex);
      auto responseIt = responseMap.find(seq);
      if (responseIt == responseMap.end()) {
        errorfLocked("Unknown response with sequence %d", seq);
        return {};
      }
      auto out = std::move(responseIt->second);
      responseMap.erase(seq);
      return out;
    }

    void put(int seq,
             const dap::TypeInfo* typeinfo,
             const GenericResponseHandler& handler) {
      std::unique_lock<std::mutex> lock(responseMutex);
      auto added =
          responseMap.emplace(seq, std::make_pair(typeinfo, handler)).second;
      if (!added) {
        errorfLocked("Response handler for sequence %d already registered",
                     seq);
      }
    }

    std::pair<const dap::TypeInfo*, GenericEventHandler> event(
        const std::string& name) {
      std::unique_lock<std::mutex> lock(eventMutex);
      auto it = eventMap.find(name);
      return (it != eventMap.end()) ? it->second : decltype(it->second){};
    }

    void put(const dap::TypeInfo* typeinfo,
             const GenericEventHandler& handler) {
      std::unique_lock<std::mutex> lock(eventMutex);
      auto added =
          eventMap.emplace(typeinfo->name(), std::make_pair(typeinfo, handler))
              .second;
      if (!added) {
        errorfLocked("Event handler for '%s' already registered",
                     typeinfo->name().c_str());
      }
    }

    GenericResponseSentHandler responseSent(const dap::TypeInfo* typeinfo) {
      std::unique_lock<std::mutex> lock(responseSentMutex);
      auto it = responseSentMap.find(typeinfo);
      return (it != responseSentMap.end()) ? it->second
                                           : decltype(it->second){};
    }

    void put(const dap::TypeInfo* typeinfo,
             const GenericResponseSentHandler& handler) {
      std::unique_lock<std::mutex> lock(responseSentMutex);
      auto added = responseSentMap.emplace(typeinfo, handler).second;
      if (!added) {
        errorfLocked("Response sent handler for '%s' already registered",
                     typeinfo->name().c_str());
      }
    }

   private:
    void errorfLocked(const char* format, ...) {
      va_list vararg;
      va_start(vararg, format);
      errorLocked(format, vararg);
      va_end(vararg);
    }

    void errorLocked(const char* format, va_list args) {
      char buf[2048];
      vsnprintf(buf, sizeof(buf), format, args);
      if (errorHandler) {
        errorHandler(buf);
      }
    }

    std::mutex errorMutex;
    ErrorHandler errorHandler;

    std::mutex requestMutex;
    std::unordered_map<std::string,
                       std::pair<const dap::TypeInfo*, GenericRequestHandler>>
        requestMap;

    std::mutex responseMutex;
    std::unordered_map<int64_t,
                       std::pair<const dap::TypeInfo*, GenericResponseHandler>>
        responseMap;

    std::mutex eventMutex;
    std::unordered_map<std::string,
                       std::pair<const dap::TypeInfo*, GenericEventHandler>>
        eventMap;

    std::mutex responseSentMutex;
    std::unordered_map<const dap::TypeInfo*, GenericResponseSentHandler>
        responseSentMap;
  };  // EventHandlers

  Payload processMessage(const std::string& str) {
    auto d = dap::json::Deserializer(str);
    dap::string type;
    if (!d.field("type", &type)) {
      handlers.error("Message missing string 'type' field");
      return {};
    }

    dap::integer sequence = 0;
    if (!d.field("seq", &sequence)) {
      handlers.error("Message missing number 'seq' field");
      return {};
    }

    if (type == "request") {
      return processRequest(&d, sequence);
    } else if (type == "event") {
      return processEvent(&d);
    } else if (type == "response") {
      processResponse(&d);
      return {};
    } else {
      handlers.error("Unknown message type '%s'", type.c_str());
    }

    return {};
  }

  Payload processRequest(dap::json::Deserializer* d, dap::integer sequence) {
    dap::string command;
    if (!d->field("command", &command)) {
      handlers.error("Request missing string 'command' field");
      return {};
    }

    const dap::TypeInfo* typeinfo;
    GenericRequestHandler handler;
    std::tie(typeinfo, handler) = handlers.request(command);
    if (!typeinfo) {
      handlers.error("No request handler registered for command '%s'",
                     command.c_str());
      return {};
    }

    auto data = new uint8_t[typeinfo->size()];
    typeinfo->construct(data);

    if (!d->field("arguments", [&](dap::Deserializer* d) {
          return typeinfo->deserialize(d, data);
        })) {
      handlers.error("Failed to deserialize request");
      typeinfo->destruct(data);
      delete[] data;
      return {};
    }

    return [=] {
      handler(
          data,
          [&](const dap::TypeInfo* typeinfo, const void* data) {
            // onSuccess
            dap::json::Serializer s;
            s.object([&](dap::FieldSerializer* fs) {
              return fs->field("seq", dap::integer(nextSeq++)) &&
                     fs->field("type", "response") &&
                     fs->field("request_seq", sequence) &&
                     fs->field("success", dap::boolean(true)) &&
                     fs->field("command", command) &&
                     fs->field("body", [&](dap::Serializer* s) {
                       return typeinfo->serialize(s, data);
                     });
            });
            send(s.dump());

            if (auto handler = handlers.responseSent(typeinfo)) {
              handler(data, nullptr);
            }
          },
          [&](const dap::TypeInfo* typeinfo, const dap::Error& error) {
            // onError
            dap::json::Serializer s;
            s.object([&](dap::FieldSerializer* fs) {
              return fs->field("seq", dap::integer(nextSeq++)) &&
                     fs->field("type", "response") &&
                     fs->field("request_seq", sequence) &&
                     fs->field("success", dap::boolean(false)) &&
                     fs->field("command", command) &&
                     fs->field("message", error.message);
            });
            send(s.dump());

            if (auto handler = handlers.responseSent(typeinfo)) {
              handler(nullptr, &error);
            }
          });
      typeinfo->destruct(data);
      delete[] data;
    };
  }

  Payload processEvent(dap::json::Deserializer* d) {
    dap::string event;
    if (!d->field("event", &event)) {
      handlers.error("Event missing string 'event' field");
      return {};
    }

    const dap::TypeInfo* typeinfo;
    GenericEventHandler handler;
    std::tie(typeinfo, handler) = handlers.event(event);
    if (!typeinfo) {
      handlers.error("No event handler registered for event '%s'",
                     event.c_str());
      return {};
    }

    auto data = new uint8_t[typeinfo->size()];
    typeinfo->construct(data);

    if (!d->field("body", [&](dap::Deserializer* d) {
          return typeinfo->deserialize(d, data);
        })) {
      handlers.error("Failed to deserialize event '%s' body", event.c_str());
      typeinfo->destruct(data);
      delete[] data;
      return {};
    }

    return [=] {
      handler(data);
      typeinfo->destruct(data);
      delete[] data;
    };
  }

  void processResponse(const dap::Deserializer* d) {
    dap::integer requestSeq = 0;
    if (!d->field("request_seq", &requestSeq)) {
      handlers.error("Response missing int 'request_seq' field");
      return;
    }

    const dap::TypeInfo* typeinfo;
    GenericResponseHandler handler;
    std::tie(typeinfo, handler) = handlers.response(requestSeq);
    if (!typeinfo) {
      handlers.error("Unknown response with sequence %d", requestSeq);
      return;
    }

    dap::boolean success = false;
    if (!d->field("success", &success)) {
      handlers.error("Response missing int 'success' field");
      return;
    }

    if (success) {
      auto data = std::unique_ptr<uint8_t[]>(new uint8_t[typeinfo->size()]);
      typeinfo->construct(data.get());

      // "body" field in Response is an optional field.
      d->field("body", [&](const dap::Deserializer* d) {
        return typeinfo->deserialize(d, data.get());
      });

      handler(data.get(), nullptr);
      typeinfo->destruct(data.get());
    } else {
      std::string message;
      if (!d->field("message", &message)) {
        handlers.error("Failed to deserialize message");
        return;
      }
      auto error = dap::Error("%s", message.c_str());
      handler(nullptr, &error);
    }
  }

  bool send(const std::string& s) {
    std::unique_lock<std::mutex> lock(sendMutex);
    if (!writer.isOpen()) {
      handlers.error("Send failed as the writer is closed");
      return false;
    }
    return writer.write(s);
  }

  std::atomic<bool> isBound = {false};
  dap::ContentReader reader;
  dap::ContentWriter writer;

  std::atomic<bool> shutdown = {false};
  EventHandlers handlers;
  std::thread recvThread;
  std::thread dispatchThread;
  dap::Chan<Payload> inbox;
  std::atomic<uint32_t> nextSeq = {1};
  std::mutex sendMutex;
};

}  // anonymous namespace

namespace dap {

Error::Error(const std::string& message) : message(message) {}

Error::Error(const char* msg, ...) {
  char buf[2048];
  va_list vararg;
  va_start(vararg, msg);
  vsnprintf(buf, sizeof(buf), msg, vararg);
  va_end(vararg);
  message = buf;
}

Session::~Session() = default;

std::unique_ptr<Session> Session::create() {
  return std::unique_ptr<Session>(new Impl());
}

}  // namespace dap
