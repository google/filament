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

#include "dap/session.h"
#include "dap/io.h"
#include "dap/protocol.h"

#include "chan.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <array>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace dap {

struct TestResponse : public Response {
  boolean b;
  integer i;
  number n;
  array<integer> a;
  object o;
  string s;
  optional<integer> o1;
  optional<integer> o2;
};

DAP_STRUCT_TYPEINFO(TestResponse,
                    "test-response",
                    DAP_FIELD(b, "res_b"),
                    DAP_FIELD(i, "res_i"),
                    DAP_FIELD(n, "res_n"),
                    DAP_FIELD(a, "res_a"),
                    DAP_FIELD(o, "res_o"),
                    DAP_FIELD(s, "res_s"),
                    DAP_FIELD(o1, "res_o1"),
                    DAP_FIELD(o2, "res_o2"));

struct TestRequest : public Request {
  using Response = TestResponse;

  boolean b;
  integer i;
  number n;
  array<integer> a;
  object o;
  string s;
  optional<integer> o1;
  optional<integer> o2;
};

DAP_STRUCT_TYPEINFO(TestRequest,
                    "test-request",
                    DAP_FIELD(b, "req_b"),
                    DAP_FIELD(i, "req_i"),
                    DAP_FIELD(n, "req_n"),
                    DAP_FIELD(a, "req_a"),
                    DAP_FIELD(o, "req_o"),
                    DAP_FIELD(s, "req_s"),
                    DAP_FIELD(o1, "req_o1"),
                    DAP_FIELD(o2, "req_o2"));

struct TestEvent : public Event {
  boolean b;
  integer i;
  number n;
  array<integer> a;
  object o;
  string s;
  optional<integer> o1;
  optional<integer> o2;
};

DAP_STRUCT_TYPEINFO(TestEvent,
                    "test-event",
                    DAP_FIELD(b, "evt_b"),
                    DAP_FIELD(i, "evt_i"),
                    DAP_FIELD(n, "evt_n"),
                    DAP_FIELD(a, "evt_a"),
                    DAP_FIELD(o, "evt_o"),
                    DAP_FIELD(s, "evt_s"),
                    DAP_FIELD(o1, "evt_o1"),
                    DAP_FIELD(o2, "evt_o2"));

};  // namespace dap

namespace {

dap::TestRequest createRequest() {
  dap::TestRequest request;
  request.b = false;
  request.i = 72;
  request.n = 9.87;
  request.a = {2, 5, 7, 8};
  request.o = {
      std::make_pair("a", dap::integer(1)),
      std::make_pair("b", dap::number(2)),
      std::make_pair("c", dap::string("3")),
  };
  request.s = "request";
  request.o2 = 42;
  return request;
}

dap::TestResponse createResponse() {
  dap::TestResponse response;
  response.b = true;
  response.i = 99;
  response.n = 123.456;
  response.a = {5, 4, 3, 2, 1};
  response.o = {
      std::make_pair("one", dap::integer(1)),
      std::make_pair("two", dap::number(2)),
      std::make_pair("three", dap::string("3")),
  };
  response.s = "ROGER";
  response.o1 = 50;
  return response;
}

dap::TestEvent createEvent() {
  dap::TestEvent event;
  event.b = false;
  event.i = 72;
  event.n = 9.87;
  event.a = {2, 5, 7, 8};
  event.o = {
      std::make_pair("a", dap::integer(1)),
      std::make_pair("b", dap::number(2)),
      std::make_pair("c", dap::string("3")),
  };
  event.s = "event";
  event.o2 = 42;
  return event;
}

}  // anonymous namespace

class SessionTest : public testing::Test {
 public:
  void bind() {
    auto client2server = dap::pipe();
    auto server2client = dap::pipe();
    client->bind(server2client, client2server);
    server->bind(client2server, server2client);
  }

  std::unique_ptr<dap::Session> client = dap::Session::create();
  std::unique_ptr<dap::Session> server = dap::Session::create();
};

TEST_F(SessionTest, Request) {
  dap::TestRequest received;
  server->registerHandler([&](const dap::TestRequest& req) {
    received = req;
    return createResponse();
  });

  bind();

  auto request = createRequest();
  client->send(request).get();

  // Check request was received correctly.
  ASSERT_EQ(received.b, request.b);
  ASSERT_EQ(received.i, request.i);
  ASSERT_EQ(received.n, request.n);
  ASSERT_EQ(received.a, request.a);
  ASSERT_EQ(received.o.size(), 3U);
  ASSERT_EQ(received.o["a"].get<dap::integer>(),
            request.o["a"].get<dap::integer>());
  ASSERT_EQ(received.o["b"].get<dap::number>(),
            request.o["b"].get<dap::number>());
  ASSERT_EQ(received.o["c"].get<dap::string>(),
            request.o["c"].get<dap::string>());
  ASSERT_EQ(received.s, request.s);
  ASSERT_EQ(received.o1, request.o1);
  ASSERT_EQ(received.o2, request.o2);
}

TEST_F(SessionTest, RequestResponseSuccess) {
  server->registerHandler(
      [&](const dap::TestRequest&) { return createResponse(); });

  bind();

  auto request = createRequest();
  auto response = client->send(request);

  auto got = response.get();

  // Check response was received correctly.
  ASSERT_EQ(got.error, false);
  ASSERT_EQ(got.response.b, dap::boolean(true));
  ASSERT_EQ(got.response.i, dap::integer(99));
  ASSERT_EQ(got.response.n, dap::number(123.456));
  ASSERT_EQ(got.response.a, dap::array<dap::integer>({5, 4, 3, 2, 1}));
  ASSERT_EQ(got.response.o.size(), 3U);
  ASSERT_EQ(got.response.o["one"].get<dap::integer>(), dap::integer(1));
  ASSERT_EQ(got.response.o["two"].get<dap::number>(), dap::number(2));
  ASSERT_EQ(got.response.o["three"].get<dap::string>(), dap::string("3"));
  ASSERT_EQ(got.response.s, "ROGER");
  ASSERT_EQ(got.response.o1, dap::optional<dap::integer>(50));
  ASSERT_FALSE(got.response.o2.has_value());
}

TEST_F(SessionTest, BreakPointRequestResponseSuccess) {
  server->registerHandler([&](const dap::SetBreakpointsRequest&) {
    dap::SetBreakpointsResponse response;
    dap::Breakpoint bp;
    bp.line = 2;
    response.breakpoints.emplace_back(std::move(bp));
    return response;
  });

  bind();

  auto got = client->send(dap::SetBreakpointsRequest{}).get();

  // Check response was received correctly.
  ASSERT_EQ(got.error, false);
  ASSERT_EQ(got.response.breakpoints.size(), 1U);
}

TEST_F(SessionTest, RequestResponseOrError) {
  server->registerHandler(
      [&](const dap::TestRequest&) -> dap::ResponseOrError<dap::TestResponse> {
        return dap::Error("Oh noes!");
      });

  bind();

  auto response = client->send(createRequest());

  auto got = response.get();

  // Check response was received correctly.
  ASSERT_EQ(got.error, true);
  ASSERT_EQ(got.error.message, "Oh noes!");
}

TEST_F(SessionTest, RequestResponseError) {
  server->registerHandler(
      [&](const dap::TestRequest&) { return dap::Error("Oh noes!"); });

  bind();

  auto response = client->send(createRequest());

  auto got = response.get();

  // Check response was received correctly.
  ASSERT_EQ(got.error, true);
  ASSERT_EQ(got.error.message, "Oh noes!");
}

TEST_F(SessionTest, ResponseSentHandlerSuccess) {
  const auto response = createResponse();

  dap::Chan<dap::ResponseOrError<dap::TestResponse>> chan;
  server->registerHandler([&](const dap::TestRequest&) { return response; });
  server->registerSentHandler(
      [&](const dap::ResponseOrError<dap::TestResponse> r) { chan.put(r); });

  bind();

  client->send(createRequest());

  auto got = chan.take().value();
  ASSERT_EQ(got.error, false);
  ASSERT_EQ(got.response.b, dap::boolean(true));
  ASSERT_EQ(got.response.i, dap::integer(99));
  ASSERT_EQ(got.response.n, dap::number(123.456));
  ASSERT_EQ(got.response.a, dap::array<dap::integer>({5, 4, 3, 2, 1}));
  ASSERT_EQ(got.response.o.size(), 3U);
  ASSERT_EQ(got.response.o["one"].get<dap::integer>(), dap::integer(1));
  ASSERT_EQ(got.response.o["two"].get<dap::number>(), dap::number(2));
  ASSERT_EQ(got.response.o["three"].get<dap::string>(), dap::string("3"));
  ASSERT_EQ(got.response.s, "ROGER");
  ASSERT_EQ(got.response.o1, dap::optional<dap::integer>(50));
  ASSERT_FALSE(got.response.o2.has_value());
}

TEST_F(SessionTest, ResponseSentHandlerError) {
  dap::Chan<dap::ResponseOrError<dap::TestResponse>> chan;
  server->registerHandler(
      [&](const dap::TestRequest&) { return dap::Error("Oh noes!"); });
  server->registerSentHandler(
      [&](const dap::ResponseOrError<dap::TestResponse> r) { chan.put(r); });

  bind();

  client->send(createRequest());

  auto got = chan.take().value();
  ASSERT_EQ(got.error, true);
  ASSERT_EQ(got.error.message, "Oh noes!");
}

TEST_F(SessionTest, Event) {
  dap::Chan<dap::TestEvent> received;
  server->registerHandler([&](const dap::TestEvent& e) { received.put(e); });

  bind();

  auto event = createEvent();
  client->send(event);

  // Check event was received correctly.
  auto got = received.take().value();

  ASSERT_EQ(got.b, event.b);
  ASSERT_EQ(got.i, event.i);
  ASSERT_EQ(got.n, event.n);
  ASSERT_EQ(got.a, event.a);
  ASSERT_EQ(got.o.size(), 3U);
  ASSERT_EQ(got.o["a"].get<dap::integer>(), event.o["a"].get<dap::integer>());
  ASSERT_EQ(got.o["b"].get<dap::number>(), event.o["b"].get<dap::number>());
  ASSERT_EQ(got.o["c"].get<dap::string>(), event.o["c"].get<dap::string>());
  ASSERT_EQ(got.s, event.s);
  ASSERT_EQ(got.o1, event.o1);
  ASSERT_EQ(got.o2, event.o2);
}

TEST_F(SessionTest, RegisterHandlerFunction) {
  struct S {
    static dap::TestResponse requestA(const dap::TestRequest&) { return {}; }
    static dap::Error requestB(const dap::TestRequest&) { return {}; }
    static dap::ResponseOrError<dap::TestResponse> requestC(
        const dap::TestRequest&) {
      return dap::Error();
    }
    static void event(const dap::TestEvent&) {}
    static void sent(const dap::ResponseOrError<dap::TestResponse>&) {}
  };
  client->registerHandler(&S::requestA);
  client->registerHandler(&S::requestB);
  client->registerHandler(&S::requestC);
  client->registerHandler(&S::event);
  client->registerSentHandler(&S::sent);
}

TEST_F(SessionTest, SendRequestNoBind) {
  bool errored = false;
  client->onError([&](const std::string&) { errored = true; });
  auto res = client->send(createRequest()).get();
  ASSERT_TRUE(errored);
  ASSERT_TRUE(res.error);
}

TEST_F(SessionTest, SendEventNoBind) {
  bool errored = false;
  client->onError([&](const std::string&) { errored = true; });
  client->send(createEvent());
  ASSERT_TRUE(errored);
}

TEST_F(SessionTest, Concurrency) {
  std::atomic<int> numEventsHandled = {0};
  std::atomic<bool> done = {false};

  server->registerHandler(
      [](const dap::TestRequest&) { return dap::TestResponse(); });

  server->registerHandler([&](const dap::TestEvent&) {
    if (numEventsHandled++ > 10000) {
      done = true;
    }
  });

  bind();

  constexpr int numThreads = 32;
  std::array<std::thread, numThreads> threads;

  for (int i = 0; i < numThreads; i++) {
    threads[i] = std::thread([&] {
      while (!done) {
        client->send(createEvent());
        client->send(createRequest());
      }
    });
  }

  for (int i = 0; i < numThreads; i++) {
    threads[i].join();
  }

  client.reset();
  server.reset();
}
