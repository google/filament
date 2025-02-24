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

#ifndef dap_session_h
#define dap_session_h

#include "future.h"
#include "io.h"
#include "typeinfo.h"
#include "typeof.h"

#include <functional>

namespace dap {

// Forward declarations
struct Request;
struct Response;
struct Event;

// internal functionality
namespace detail {
template <typename T>
struct traits {
  static constexpr bool isRequest = std::is_base_of<dap::Request, T>::value;
  static constexpr bool isResponse = std::is_base_of<dap::Response, T>::value;
  static constexpr bool isEvent = std::is_base_of<dap::Event, T>::value;
};

// ArgTy<F>::type resolves to the first argument type of the function F.
// F can be a function, static member function, or lambda.
template <typename F>
struct ArgTy {
  using type = typename ArgTy<decltype(&F::operator())>::type;
};

template <typename R, typename Arg>
struct ArgTy<R (*)(Arg)> {
  using type = typename std::decay<Arg>::type;
};

template <typename R, typename C, typename Arg>
struct ArgTy<R (C::*)(Arg) const> {
  using type = typename std::decay<Arg>::type;
};
}  // namespace detail

////////////////////////////////////////////////////////////////////////////////
// Error
////////////////////////////////////////////////////////////////////////////////

// Error represents an error message in response to a DAP request.
struct Error {
  Error() = default;
  Error(const std::string& error);
  Error(const char* msg, ...);

  // operator bool() returns true if there is an error.
  inline operator bool() const { return message.size() > 0; }

  std::string message;  // empty represents success.
};

////////////////////////////////////////////////////////////////////////////////
// ResponseOrError<T>
////////////////////////////////////////////////////////////////////////////////

// ResponseOrError holds either the response to a DAP request or an error
// message.
template <typename T>
struct ResponseOrError {
  using Request = T;

  inline ResponseOrError() = default;
  inline ResponseOrError(const T& response);
  inline ResponseOrError(T&& response);
  inline ResponseOrError(const Error& error);
  inline ResponseOrError(Error&& error);
  inline ResponseOrError(const ResponseOrError& other);
  inline ResponseOrError(ResponseOrError&& other);

  inline ResponseOrError& operator=(const ResponseOrError& other);
  inline ResponseOrError& operator=(ResponseOrError&& other);

  T response;
  Error error;  // empty represents success.
};

template <typename T>
ResponseOrError<T>::ResponseOrError(const T& resp) : response(resp) {}
template <typename T>
ResponseOrError<T>::ResponseOrError(T&& resp) : response(std::move(resp)) {}
template <typename T>
ResponseOrError<T>::ResponseOrError(const Error& err) : error(err) {}
template <typename T>
ResponseOrError<T>::ResponseOrError(Error&& err) : error(std::move(err)) {}
template <typename T>
ResponseOrError<T>::ResponseOrError(const ResponseOrError& other)
    : response(other.response), error(other.error) {}
template <typename T>
ResponseOrError<T>::ResponseOrError(ResponseOrError&& other)
    : response(std::move(other.response)), error(std::move(other.error)) {}
template <typename T>
ResponseOrError<T>& ResponseOrError<T>::operator=(
    const ResponseOrError& other) {
  response = other.response;
  error = other.error;
  return *this;
}
template <typename T>
ResponseOrError<T>& ResponseOrError<T>::operator=(ResponseOrError&& other) {
  response = std::move(other.response);
  error = std::move(other.error);
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
// Session
////////////////////////////////////////////////////////////////////////////////

// Session implements a DAP client or server endpoint.
// The general usage is as follows:
// (1) Create a session with Session::create().
// (2) Register request and event handlers with registerHandler().
// (3) Optionally register a protocol error handler with onError().
// (3) Bind the session to the remote endpoint with bind().
// (4) Send requests or events with send().
class Session {
  template <typename T>
  using IsRequest = typename std::enable_if<detail::traits<T>::isRequest>::type;

  template <typename T>
  using IsEvent = typename std::enable_if<detail::traits<T>::isEvent>::type;

  template <typename F>
  using ArgTy = typename detail::ArgTy<F>::type;

 public:
  virtual ~Session();

  // ErrorHandler is the type of callback function used for reporting protocol
  // errors.
  using ErrorHandler = std::function<void(const char*)>;

  // create() constructs and returns a new Session.
  static std::unique_ptr<Session> create();

  // onError() registers a error handler that will be called whenever a protocol
  // error is encountered.
  // Only one error handler can be bound at any given time, and later calls
  // will replace the existing error handler.
  virtual void onError(const ErrorHandler&) = 0;

  // registerHandler() registers a request handler for a specific request type.
  // The function F must have one of the following signatures:
  //   ResponseOrError<ResponseType>(const RequestType&)
  //   ResponseType(const RequestType&)
  //   Error(const RequestType&)
  template <typename F, typename RequestType = ArgTy<F>>
  inline IsRequest<RequestType> registerHandler(F&& handler);

  // registerHandler() registers a event handler for a specific event type.
  // The function F must have the following signature:
  //   void(const EventType&)
  template <typename F, typename EventType = ArgTy<F>>
  inline IsEvent<EventType> registerHandler(F&& handler);

  // registerSentHandler() registers the function F to be called when a response
  // of the specific type has been sent.
  // The function F must have the following signature:
  //   void(const ResponseOrError<ResponseType>&)
  template <typename F, typename ResponseType = typename ArgTy<F>::Request>
  inline void registerSentHandler(F&& handler);

  // send() sends the request to the connected endpoint and returns a
  // future that is assigned the request response or error.
  template <typename T, typename = IsRequest<T>>
  future<ResponseOrError<typename T::Response>> send(const T& request);

  // send() sends the event to the connected endpoint.
  template <typename T, typename = IsEvent<T>>
  void send(const T& event);

  // bind() connects this Session to an endpoint.
  // bind() can only be called once. Repeated calls will raise an error, but
  // otherwise will do nothing.
  virtual void bind(const std::shared_ptr<Reader>&,
                    const std::shared_ptr<Writer>&) = 0;
  inline void bind(const std::shared_ptr<ReaderWriter>&);

 protected:
  using RequestSuccessCallback =
      std::function<void(const TypeInfo*, const void*)>;

  using RequestErrorCallback =
      std::function<void(const TypeInfo*, const Error& message)>;

  using GenericResponseHandler = std::function<void(const void*, const Error*)>;

  using GenericRequestHandler =
      std::function<void(const void* args,
                         const RequestSuccessCallback& onSuccess,
                         const RequestErrorCallback& onError)>;

  using GenericEventHandler = std::function<void(const void* args)>;

  using GenericResponseSentHandler =
      std::function<void(const void* response, const Error* error)>;

  virtual void registerHandler(const TypeInfo* typeinfo,
                               const GenericRequestHandler& handler) = 0;

  virtual void registerHandler(const TypeInfo* typeinfo,
                               const GenericEventHandler& handler) = 0;

  virtual void registerHandler(const TypeInfo* typeinfo,
                               const GenericResponseSentHandler& handler) = 0;

  virtual bool send(const dap::TypeInfo* requestTypeInfo,
                    const dap::TypeInfo* responseTypeInfo,
                    const void* request,
                    const GenericResponseHandler& responseHandler) = 0;

  virtual bool send(const TypeInfo*, const void* event) = 0;
};

template <typename F, typename T>
Session::IsRequest<T> Session::registerHandler(F&& handler) {
  using ResponseType = typename T::Response;
  auto cb = [handler](const void* args, const RequestSuccessCallback& onSuccess,
                      const RequestErrorCallback& onError) {
    ResponseOrError<ResponseType> res =
        handler(*reinterpret_cast<const T*>(args));
    if (res.error) {
      onError(TypeOf<ResponseType>::type(), res.error);
    } else {
      onSuccess(TypeOf<ResponseType>::type(), &res.response);
    }
  };
  const TypeInfo* typeinfo = TypeOf<T>::type();
  registerHandler(typeinfo, cb);
}

template <typename F, typename T>
Session::IsEvent<T> Session::registerHandler(F&& handler) {
  auto cb = [handler](const void* args) {
    handler(*reinterpret_cast<const T*>(args));
  };
  const TypeInfo* typeinfo = TypeOf<T>::type();
  registerHandler(typeinfo, cb);
}

template <typename F, typename T>
void Session::registerSentHandler(F&& handler) {
  auto cb = [handler](const void* response, const Error* error) {
    if (error != nullptr) {
      handler(ResponseOrError<T>(*error));
    } else {
      handler(ResponseOrError<T>(*reinterpret_cast<const T*>(response)));
    }
  };
  const TypeInfo* typeinfo = TypeOf<T>::type();
  registerHandler(typeinfo, cb);
}

template <typename T, typename>
future<ResponseOrError<typename T::Response>> Session::send(const T& request) {
  using Response = typename T::Response;
  promise<ResponseOrError<Response>> promise;
  auto sent = send(TypeOf<T>::type(), TypeOf<Response>::type(), &request,
                   [=](const void* result, const Error* error) {
                     if (error != nullptr) {
                       promise.set_value(ResponseOrError<Response>(*error));
                     } else {
                       promise.set_value(ResponseOrError<Response>(
                           *reinterpret_cast<const Response*>(result)));
                     }
                   });
  if (!sent) {
    promise.set_value(Error("Failed to send request"));
  }
  return promise.get_future();
}

template <typename T, typename>
void Session::send(const T& event) {
  const TypeInfo* typeinfo = TypeOf<T>::type();
  send(typeinfo, &event);
}

void Session::bind(const std::shared_ptr<ReaderWriter>& rw) {
  bind(rw, rw);
}

}  // namespace dap

#endif  // dap_session_h
