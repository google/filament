// Copyright 2024 The langsvr Authors
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

#ifndef LANGSVR_SESSION_H_
#define LANGSVR_SESSION_H_

#include <functional>
#include <future>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "langsvr/json/builder.h"
#include "langsvr/json/value.h"
#include "langsvr/lsp/lsp.h"
#include "langsvr/lsp/message_kind.h"
#include "langsvr/one_of.h"
#include "langsvr/result.h"

namespace langsvr {

/// Session provides a message dispatch registry for LSP messages.
class Session {
    struct RequestHandler {
        // Returns a member of 'result' or 'error'
        std::function<Result<json::Builder::Member>(const json::Value&, json::Builder&)> function;
        std::function<void()> post_send;
    };
    struct NotificationHandler {
        std::function<Result<SuccessType>(const json::Value&)> function;
    };

  public:
    using Sender = std::function<Result<SuccessType>(std::string_view)>;

    /// SetSender sets the message send handler used by Session for sending request responses and
    /// notifications.
    /// @param sender the new sender for the session.
    void SetSender(Sender&& sender) { sender_ = std::move(sender); }

    /// Receive decodes the LSP message from the JSON string @p json, calling the appropriate
    /// registered message handler, and sending the response to the registered Sender if the message
    /// was an LSP request.
    /// @param json the incoming JSON message.
    /// @return success or failure
    Result<SuccessType> Receive(std::string_view json);

    /// Send dispatches to either SendRequest() or SetNotification based on the type of T.
    /// @param message the Request or Notification message
    /// @return the return value of either SendRequest() and SendNotification()
    template <typename T>
    auto Send(T&& message) {
        using Message = std::decay_t<T>;
        static constexpr bool kIsRequest = Message::kMessageKind == lsp::MessageKind::kRequest;
        static constexpr bool kIsNotification =
            Message::kMessageKind == lsp::MessageKind::kNotification;
        static_assert(kIsRequest || kIsNotification);
        if constexpr (kIsRequest) {
            return SendRequest(std::forward<T>(message));
        } else {
            return SendNotification(std::forward<T>(message));
        }
    }

    /// SendRequest encodes and sends the LSP request to the Sender registered with SetSender().
    /// @param request the request
    /// @return a Result holding a std::future which will hold the response value.
    template <typename T>
    Result<std::future<typename std::decay_t<T>::ResultType>> SendRequest(T&& request) {
        using Request = std::decay_t<T>;
        auto b = json::Builder::Create();
        auto id = next_request_id_++;
        std::vector<json::Builder::Member> members{
            json::Builder::Member{"id", b->I64(id)},
            json::Builder::Member{"method", b->String(Request::kMethod)},
        };
        if constexpr (Request::kHasParams) {
            auto params = Encode(request, *b.get());
            if (params != Success) {
                return params.Failure();
            }
            members.push_back(json::Builder::Member{"params", params.Get()});
        }

        using ResponseResultType = typename Request::ResultType;
        using ResponseSuccessType = typename Request::SuccessType;
        using ResponseFailureType = typename Request::FailureType;

        // TODO: Avoid the need for a shared pointer.
        auto promise = std::make_shared<std::promise<ResponseResultType>>();
        response_handlers_.emplace(
            id, [promise](const json::Value& response) -> Result<SuccessType> {
                if (auto result_json = response.Get(kResponseResult); result_json == Success) {
                    ResponseSuccessType result;
                    if (auto res = lsp::Decode(*result_json.Get(), result); res != Success) {
                        return res.Failure();
                    }
                    promise->set_value(ResponseResultType{std::move(result)});
                    return Success;
                }
                if constexpr (std::is_same_v<ResponseFailureType, void>) {
                    return Failure{"response missing 'result'"};
                } else {
                    ResponseFailureType error;
                    auto error_json = response.Get(kResponseError);
                    if (error_json != Success) {
                        return error_json.Failure();
                    }
                    if (auto res = lsp::Decode(*error_json.Get(), error); res != Success) {
                        return res.Failure();
                    }
                    promise->set_value(ResponseResultType{std::move(error)});
                }
                return Success;
            });

        auto send = SendJson(b->Object(members)->Json());
        if (send != Success) {
            return send.Failure();
        }

        return promise->get_future();
    }

    /// SendNotification encodes and sends the LSP notification to the Sender registered with
    /// SetSender().
    /// @param notification the notification
    /// @return success or failure.
    template <typename T>
    Result<SuccessType> SendNotification(T&& notification) {
        using Notification = std::decay_t<T>;
        auto b = json::Builder::Create();
        std::vector<json::Builder::Member> members{
            json::Builder::Member{"method", b->String(Notification::kMethod)},
        };
        if constexpr (Notification::kHasParams) {
            auto params = Encode(notification, *b.get());
            if (params != Success) {
                return params.Failure();
            }
            members.push_back(json::Builder::Member{"params", params.Get()});
        }
        return SendJson(b->Object(members)->Json());
    }

    /// RegisteredRequestHandler is the return type Register() when registering a Request handler.
    class RegisteredRequestHandler {
      public:
        /// OnPostSend registers @p callback to be called once the request response has been sent.
        /// @param callback the callback function to call when a response has been sent.
        void OnPostSend(std::function<void()>&& callback) {
            handler.post_send = std::move(callback);
        }

      private:
        friend class Session;
        RegisteredRequestHandler(RequestHandler& h) : handler{h} {}
        RegisteredRequestHandler(const RegisteredRequestHandler&) = delete;
        RegisteredRequestHandler& operator=(const RegisteredRequestHandler&) = delete;
        RequestHandler& handler;
    };

    /// Register registers the LSP Request or Notification handler to be called when Receive() is
    /// called with a message of the appropriate type.
    /// @tparam F a function with the signature: `RESULT(const T&)`, where:
    /// `T` is a LSP request and `RESULT` is one of:
    ///   * `Result<T::Result, T::Failure>`
    ///   * `T::Result`
    ///   * `T::Failure`
    /// `T` is a LSP notification and `RESULT` is `Result<SuccessType>`.
    /// @return a RegisteredRequestHandler if the parameter type of F is a LSP request, otherwise
    /// void.
    template <typename F>
    auto Register(F&& callback) {
        // Examine the function signature to determine the message type
        using Sig = SignatureOf<F>;
        static_assert(Sig::parameter_count == 1);
        using Message = typename Sig::template parameter<0>;

        // Is the message a request or notification?
        static constexpr bool kIsRequest = Message::kMessageKind == lsp::MessageKind::kRequest;
        static constexpr bool kIsNotification =
            Message::kMessageKind == lsp::MessageKind::kNotification;
        static_assert(kIsRequest || kIsNotification);

        // The method is the identification string used in the JSON message.
        auto method = std::string(Message::kMethod);

        if constexpr (kIsRequest) {
            // Build the request handler function that deserializes the message and calls the
            // handler function. The result of the handler is then sent back as a 'result' or
            // 'error'.
            auto& handler = request_handlers_[method];
            handler.function = [f = std::forward<F>(callback)](
                                   const json::Value& object,
                                   json::Builder& json_builder) -> Result<json::Builder::Member> {
                Message request;
                if constexpr (Message::kHasParams) {
                    auto params = object.Get("params");
                    if (params != Success) {
                        return params.Failure();
                    }
                    if (auto res = Decode(*params.Get(), request); res != Success) {
                        return res.Failure();
                    }
                }
                auto res = f(request);
                using RES_TYPE = std::decay_t<decltype(res)>;
                using RequestSuccessType = typename Message::SuccessType;
                using RequestFailureType = typename Message::FailureType;
                if constexpr (IsResult<RES_TYPE>) {
                    using ResultSuccessType = typename RES_TYPE::ResultSuccess;
                    using ResultFailureType = typename RES_TYPE::ResultFailure;
                    static_assert(
                        std::is_same_v<ResultSuccessType, RequestSuccessType>,
                        "request handler Result<> success return type does not match Request's "
                        "Result type");
                    static_assert(std::is_same_v<ResultFailureType, RequestFailureType>,
                                  "request handler Result<> failure return type does not match "
                                  "Request's Failure type");
                    if (res == Success) {
                        auto enc = Encode(res.Get(), json_builder);
                        if (enc != Success) {
                            return enc.Failure();
                        }
                        return json::Builder::Member{std::string(kResponseResult), enc.Get()};
                    } else {
                        auto enc = Encode(res.Failure(), json_builder);
                        if (enc != Success) {
                            return enc.Failure();
                        }
                        return json::Builder::Member{std::string(kResponseError), enc.Get()};
                    }
                } else {
                    static_assert((std::is_same_v<RES_TYPE, RequestSuccessType> ||
                                   std::is_same_v<RES_TYPE, RequestFailureType>),
                                  "request handler return type is not supported");
                    auto enc = Encode(res, json_builder);
                    if (enc != Success) {
                        return enc.Failure();
                    }
                    return json::Builder::Member{
                        std::string(std::is_same_v<RES_TYPE, RequestSuccessType> ? kResponseResult
                                                                                 : kResponseError),
                        enc.Get()};
                }
            };
            return RegisteredRequestHandler{handler};
        } else if constexpr (kIsNotification) {
            auto& handler = notification_handlers_[method];
            handler.function =
                [f = std::forward<F>(callback)](const json::Value& object) -> Result<SuccessType> {
                Message notification;
                if constexpr (Message::kHasParams) {
                    auto params = object.Get("params");
                    if (params != Success) {
                        return params.Failure();
                    }
                    if (auto res = Decode(*params.Get(), notification); res != Success) {
                        return res.Failure();
                    }
                }
                return f(notification);
            };
            return;
        }
    }

  private:
    static constexpr std::string_view kResponseResult = "result";
    static constexpr std::string_view kResponseError = "error";

    Result<SuccessType> SendJson(std::string_view msg);

    Sender sender_;
    std::unordered_map<std::string, RequestHandler> request_handlers_;
    std::unordered_map<std::string, NotificationHandler> notification_handlers_;
    std::unordered_map<json::I64, std::function<Result<SuccessType>(const json::Value&)>>
        response_handlers_;
    json::I64 next_request_id_ = 1;
};

}  // namespace langsvr

#endif  // LANGSVR_SESSION_H_
