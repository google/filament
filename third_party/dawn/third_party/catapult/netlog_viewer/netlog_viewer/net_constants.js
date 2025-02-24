// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


'use strict';

// TODO(rayraymond): This is a big hack that accounts for the
// following problem in the NetLog viewer code: There are
// several constants in Chromium that the web app does not have
// initial values for until a log is loaded. Originally,
// chrome://net-internals got values for these constants from
// the browser when about:net-internals was first loaded. This
// is no longer the case in this web app. However, certain test
// files in the NetLog viewer code
// (i.e. log_view_painter_test.html and events_view_test.html)
// rely on these constants to be defined globally, rather than
// them being passed around as input to the functions that need
// them. This file defines two functions which set and unset
// initial values for the constants that are used in the test
// files. Comments are provided for what specific files within
// Chromium these constants are taken from.

function setNetLogConstantsForTest() {
  // See net/log/net_log_util.cc in Chromium.
  Constants = {
    clientInfo: {
      numericDate: ''
    }
  };

  // See net/log/net_log.h in Chromium.
  EventPhase = {
    PHASE_BEGIN: 0,
    PHASE_END: 1,
    PHASE_NONE: 2
  };

  // See net/log/net_log_source_type_list.h in Chromium.
  EventSourceType = {
    NONE: 0,
    URL_REQUEST: 1,
    TRANSPORT_CONNECT_JOB: 2,
    SOCKET: 3,
    HOST_RESOLVER_IMPL_JOB: 4,
    HTTP_STREAM_JOB: 5,
    CERT_VERIFIER_JOB: 6,
    CERT_VERIFIER_TASK: 7
  };

  EventSourceTypeNames = makeInverseMap(EventSourceType);

  // See net/log/net_log_event_type_list.h in Chromium.
  EventType = {
    REQUEST_ALIVE: 0,
    HOST_RESOLVER_IMPL_JOB: 1,
    PROXY_CONFIG_CHANGED: 2,
    SOCKET_ALIVE: 3,
    TCP_CONNECT: 4,
    TCP_CONNECT_ATTEMPT: 5,
    SOCKET_IN_USE: 6,
    SSL_VERSION_FALLBACK: 7,
    SOCKET_BYTES_SENT: 8,
    SOCKET_BYTES_RECEIVED: 9,
    UDP_BYTES_SENT: 10,
    URL_REQUEST_START_JOB: 11,
    HTTP_CACHE_GET_BACKEND: 12,
    HTTP_CACHE_OPEN_ENTRY: 13,
    HTTP_CACHE_CREATE_ENTRY: 14,
    HTTP_CACHE_ADD_TO_ENTRY: 15,
    HTTP_CACHE_READ_INFO: 16,
    HTTP_CACHE_WRITE_INFO: 17,
    HTTP_CACHE_WRITE_DATA: 18,
    ENTRY_READ_DATA: 19,
    ENTRY_WRITE_DATA: 20,
    HTTP_STREAM_REQUEST: 21,
    HTTP_STREAM_REQUEST_BOUND_TO_JOB: 22,
    HTTP_TRANSACTION_SEND_REQUEST: 23,
    HTTP_TRANSACTION_SEND_REQUEST_HEADERS: 24,
    HTTP_TRANSACTION_HTTP2_SEND_REQUEST_HEADERS: 25,
    HTTP_TRANSACTION_READ_HEADERS: 26,
    HTTP_TRANSACTION_READ_RESPONSE_HEADERS: 27,
    HTTP_TRANSACTION_READ_BODY: 28,
    HTTP2_SESSION_SEND_HEADERS: 29,
    HTTP2_SESSION_RECV_HEADERS: 30,
    HTTP2_SESSION_GOAWAY: 31,
    QUIC_SESSION: 32,
    QUIC_SESSION_RST_STREAM_FRAME_RECEIVED: 33,
    QUIC_SESSION_CONNECTION_CLOSE_FRAME_RECEIVED: 34,
    QUIC_SESSION_CRYPTO_HANDSHAKE_MESSAGE_SENT: 35,
    HTTP_STREAM_PARSER_READ_HEADERS: 36,
    CERT_VERIFIER_JOB: 37,
    CERT_CT_COMPLIANCE_CHECKED: 38,
    CERT_VERIFIER_TASK: 39,
    CERT_VERIFY_PROC: 40,
    CERT_VERIFY_PROC_PATH_BUILD_ATTEMPT: 41,
    CERT_VERIFY_PROC_PATH_BUILT: 42
  };

  EventTypeNames = makeInverseMap(EventType);

  // These flags provide metadata about the type of the load request.
  // See net/base/load_flags.h in Chromium.
  LoadFlag = {
    NORMAL: 0,
    MAIN_FRAME_DEPRECATED: 1 << 12,
    VERIFY_EV_CERT: 1 << 8
  };

  // These states correspond to the lengthy periods of time that a
  // resource load may be blocked and unable to make progress.
  // See net/base/load_states.h in Chromium.
  LoadState = {
    READING_RESPONSE: 15
  };

  // Bitmask of status flags of a certificate, representing any
  // errors, as well as other non-error status information such
  // as whether the certificate is EV.
  // See net/cert/cert_status_flags.h in Chromium.
  CertStatusFlag = {
    AUTHORITY_INVALID: 1 << 2,
    DATE_INVALID: 1 << 1,
    COMMON_NAME_INVALID: 1 << 0
  };

  // See net/cert/cert_verifier.h in Chromium.
  CertVerifierFlags = {
    VERIFY_DISABLE_NETWORK_FETCHES: 1 << 0
  };

  // See net/cert/cert_verify_proc.h in Chromium.
  CertVerifyFlags = {
    VERIFY_REV_CHECKING_ENABLED: 1 << 0,
    VERIFY_DISABLE_NETWORK_FETCHES: 1 << 4
  };

  // See net/cert/pki/simple_path_builder_delegate.h in Chromium.
  CertPathBuilderDigestPolicy = {
    kStrong: 0
  };

  // See net/quic/core/quic_protocol.h in Chromium.
  QuicRstStreamError = {
    QUIC_BAD_APPLICATION_PAYLOAD: 0
  };

  // See net/quic/core/quic_protocol.h in Chromium.
  QuicError = {
    QUIC_NETWORK_IDLE_TIMEOUT: 25
  };

  // Error domain of the net module's error codes.
  // See net/base/net_errors.h in Chromium.
  NetError = {
    ERR_FAILED: -2,
    ERR_NAME_NOT_RESOLVED: -105,
    ERR_SSL_PROTOCOL_ERROR: -107,
    ERR_CERT_DATE_INVALID: -201
  };
}

function unsetNetLogConstantsForTest() {
  Constants = null;

  EventType = null;
  EventTypeNames = null;
  EventPhase = null;
  EventSourceType = null;
  EventSourceTypeNames = null;
  NetError = null;
  QuicError = null;
  QuicRstStreamError = null;
  LoadFlag = null;
  CertStatusFlag = null;
  CertVerifierFlags = null;
  LoadState = null;
}

