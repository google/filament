// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

// TODO(eroman): put these methods into a namespace.

let createLogEntryTablePrinter;
let proxySettingsToString;

// Start of anonymous namespace.
(function() {
  'use strict';

  function canCollapseBeginWithEnd(beginEntry) {
    return beginEntry && beginEntry.isBegin() && beginEntry.end &&
        beginEntry.end.index === beginEntry.index + 1 &&
        (!beginEntry.orig.params || !beginEntry.end.orig.params);
  }

  /**
   * Creates a TablePrinter for use by the above two functions.  baseTime is
   * the time relative to which other times are displayed.
   */
  createLogEntryTablePrinter = function(logEntries, baseTime, logCreationTime,
                                        forSearch=false) {
    const entries = LogGroupEntry.createArrayFrom(logEntries);
    const tablePrinter = new TablePrinter();
    const parameterOutputter = new ParameterOutputter(tablePrinter, forSearch);

    if (entries.length === 0) {
      return tablePrinter;
    }

    const startTime = timeutil.convertTimeTicksToTime(
        (logEntries[0].source.start_time !== undefined) ?
          logEntries[0].source.start_time :
          entries[0].orig.time);

    for (let i = 0; i < entries.length; ++i) {
      const entry = entries[i];

      // Avoid printing the END for a BEGIN that was immediately before, unless
      // both have extra parameters.
      if (!entry.isEnd() || !canCollapseBeginWithEnd(entry.begin)) {
        const entryTime = timeutil.convertTimeTicksToTime(entry.orig.time);
        addRowWithTime(tablePrinter, entryTime - baseTime,
            startTime - baseTime);

        for (let j = entry.getDepth(); j > 0; --j) {
          tablePrinter.addCell('  ');
        }

        let eventText = getTextForEvent(entry);
        // Get the elapsed time, and append it to the event text.
        if (entry.isBegin()) {
          let dt = '?';
          // Definite time.
          if (entry.end) {
            dt = entry.end.orig.time - entry.orig.time;
          } else if (logCreationTime !== undefined) {
            dt = (logCreationTime - entryTime) + '+';
          }
          eventText += '  [dt=' + dt + ']';
        }

        const mainCell = tablePrinter.addCell(eventText);
        mainCell.allowOverflow = true;
      }

      // Output the extra parameters.
      if (typeof entry.orig.params === 'object') {
        // Those 5 skipped cells are: two for "t=", and three for "st=".
        tablePrinter.setNewRowCellIndent(5 + entry.getDepth());
        writeParameters(entry.orig, parameterOutputter);

        tablePrinter.setNewRowCellIndent(0);
      }
    }

    // If viewing a saved log file, add row with just the time the log was
    // created, if the event never completed.
    const lastEntry = entries[entries.length - 1];
    // If the last entry has a non-zero depth or is a begin event, the source is
    // still active.
    const isSourceActive = lastEntry.getDepth() !== 0 || lastEntry.isBegin();
    if (logCreationTime !== undefined && isSourceActive) {
      addRowWithTime(
          tablePrinter, logCreationTime - baseTime, startTime - baseTime);
    }

    return tablePrinter;
  };

  /**
   * Adds a new row to the given TablePrinter, and adds five cells containing
   * information about the time an event occured.
   * Format is '[t=<time of the event in ms>] [st=<ms since the source
   * started>]'.
   * @param {TablePrinter} tablePrinter The table printer to add the cells to.
   * @param {number} eventTime The time the event occured, in milliseconds,
   *     relative to some base time.
   * @param {number} startTime The time the first event for the source occured,
   *     relative to the same base time as eventTime.
   */
  function addRowWithTime(tablePrinter, eventTime, startTime) {
    tablePrinter.addRow();
    tablePrinter.addCell('t=');
    const tCell = tablePrinter.addCell(eventTime);
    tCell.alignRight = true;
    tablePrinter.addCell(' [st=');
    const stCell = tablePrinter.addCell(eventTime - startTime);
    stCell.alignRight = true;
    tablePrinter.addCell('] ');
  }

  /** Append an ASCII representation of the bytes for search filtering
   *  purposes. ASCII codes 32 though 126 display the corresponding
   *  characters, nulls are represented by spaces, and any other
   *  character is represented by a period.
  */
  function writeBytesAsSearchableString(bytes, out) {
    let result = ' ';
    bytes.forEach(curByte => {
      if (curByte >= 0x20 && curByte <= 0x7E) {
        result += String.fromCharCode(curByte);
      } else if (curByte === 0x00) {
        result += ' ';
      } else {
        result += '.';
      }
    });
    out.writeLine(result);
  }

  /**
   * |bytes| must be an array-like object of bytes.  Writes multiple lines to
   * |out| with the hexadecimal characters from |bytes| on the left, in
   * groups of two, and their corresponding ASCII characters on the
   * right.
   *
   * 16 bytes will be placed on each line of the output string, split into two
   * columns of 8.
   */
  function writeHexString(bytes, out) {
    function byteToPaddedHex(b) {
      let str = b.toString(16).toUpperCase();
      if (str.length < 2) {
        str = '0' + str;
      }
      return str;
    }

    const kBytesPerLine = 16;
    // Returns pretty printed kBytesPerLine bytes starting from
    // bytes[startIndex], in hexdump style format.
    function formatLine(startIndex) {
      let result = ' ';

      // Append a hex formatting of |bytes|.
      for (let i = 0; i < kBytesPerLine; ++i) {
        result += ' ';

        // Add a separator every 8 bytes.
        if ((i % 8) === 0) {
          result += ' ';
        }

        // Output hex for the byte, or space if no bytes remain.
        const byteIndex = startIndex + i;
        if (byteIndex < bytes.length) {
          result += byteToPaddedHex(bytes[byteIndex]);
        } else {
          result += '  ';
        }
      }

      result += '   ';

      // Append an ASCII formatting of the bytes, where ASCII codes 32
      // though 126 display the corresponding characters, nulls are
      // represented by spaces, and any other character is represented
      // by a period.
      for (let i = 0; i < kBytesPerLine; ++i) {
        const byteIndex = startIndex + i;
        if (byteIndex >= bytes.length) {
          break;
        }

        const curByte = bytes[byteIndex];

        if (curByte >= 0x20 && curByte <= 0x7E) {
          result += String.fromCharCode(curByte);
        } else if (curByte === 0x00) {
          result += ' ';
        } else {
          result += '.';
        }
      }

      return result;
    }

    // Output lines for each group of kBytesPerLine bytes.
    for (let i = 0; i < bytes.length; i += kBytesPerLine) {
      out.writeLine(formatLine(i));
    }
  }

  const kArrow = ' --> ';
  const kArrowIndentation = '     ';

  /**
   * Wrapper around a TablePrinter to simplify outputting lines of text for
   * event
   * parameters.
   */
  class ParameterOutputter {
    /**
     * @constructor
     */
    constructor(tablePrinter, forSearch=false) {
      this.tablePrinter_ = tablePrinter;
      this.forSearch_ = forSearch;
    }

    /**
     * Returns |true| if this ParameterOutputter is being used
     * to write output that will be used for search filtering
     * rather than display to the end-user.
    */
    isForSearch() {
      return this.forSearch_;
    }

    /**
     * Outputs a single line.
     */
    writeLine(line) {
      this.tablePrinter_.addRow();
      const cell = this.tablePrinter_.addCell(line);
      cell.allowOverflow = true;
      return cell;
    }

    /**
     * Outputs a key=value line which looks like:
     *
     *   --> key = value
     */
    writeArrowKeyValue(key, value, link) {
      const cell = this.writeLine(kArrow + key + ' = ' + value);
      cell.link = link;
    }

    /**
     * Outputs key and prettified value lines which looks like:
     *
     *   --> key = {
     *         "field1": [
     *         "field1_value1",
     *           "field1_value2"
     *         ],
     *         "field2": "field2_value"
     *       }
     */
    writeArrowKeyJSONValue(key, value) {
      const lines = JSON.stringify(value, null, 2).split('\n');
      this.writeArrowKeyValue(key, lines[0]);
      this.writeSpaceIndentedLines(5, lines.slice(1));
    }

    /**
     * Outputs a key= line which looks like:
     *
     *   --> key =
     */
    writeArrowKey(key) {
      this.writeLine(kArrow + key + ' =');
    }

    /**
     * Outputs multiple lines, each indented by numSpaces.
     * For instance if numSpaces=8 it might look like this:
     *
     *         line 1
     *         line 2
     *         line 3
     */
    writeSpaceIndentedLines(numSpaces, lines) {
      const prefix = makeRepeatedString(' ', numSpaces);
      for (let i = 0; i < lines.length; ++i) {
        this.writeLine(prefix + lines[i]);
      }
    }

    /**
     * Outputs multiple lines such that the first line has
     * an arrow pointing at it, and subsequent lines
     * align with the first one. For example:
     *
     *   --> line 1
     *       line 2
     *       line 3
     */
    writeArrowIndentedLines(lines) {
      if (lines.length === 0) {
        return;
      }

      this.writeLine(kArrow + lines[0]);

      for (let i = 1; i < lines.length; ++i) {
        this.writeLine(kArrowIndentation + lines[i]);
      }
    }
  }

  /**
   * Formats the parameters for |entry| and writes them to |out|.
   * Certain event types have custom pretty printers. Everything else will
   * default to a JSON-like format.
   */
  function writeParameters(entry, out) {
    // If headers are in an object, convert them to an array for better
    // display.
    entry = reformatHeaders(entry);

    // Use any parameter writer available for this event type.
    const paramsWriter = getParameterWriterForEventType(entry.type);
    const consumedParams = {};
    if (paramsWriter) {
      paramsWriter(entry, out, consumedParams);
    }

    // Write any un-consumed parameters.
    for (const k in entry.params) {
      if (consumedParams[k]) {
        continue;
      }
      defaultWriteParameter(k, entry.params[k], out);
    }
  }

  /**
   * Finds a writer to format the parameters for events of type |eventType|.
   *
   * @return {function} The returned function "writer" can be invoked
   *                    as |writer(entry, writer, consumedParams)|. It will
   *                    output the parameters of |entry| to |out|, and fill
   *                    |consumedParams| with the keys of the parameters
   *                    consumed. If no writer is available for |eventType| then
   *                    returns null.
   */
  function getParameterWriterForEventType(eventType) {
    switch (eventType) {
      case EventType.HTTP_TRANSACTION_SEND_REQUEST_HEADERS:
      case EventType.HTTP_TRANSACTION_SEND_TUNNEL_HEADERS:
      case EventType.TYPE_HTTP_CACHE_CALLER_REQUEST_HEADERS:
        return writeParamsForRequestHeaders;

      case EventType.PROXY_CONFIG_CHANGED:
        return writeParamsForProxyConfigChanged;

      case EventType.CERT_VERIFIER_JOB:
      case EventType.SSL_CERTIFICATES_RECEIVED:
        return writeParamsForCertificates;
      case EventType.CERT_VERIFY_PROC:
        return writeParamsForCertVerifyProc;
      case EventType.CERT_VERIFY_PROC_PATH_BUILD_ATTEMPT:
        return writeParamsForCertVerifyProcPathBuildAttempt;
      case EventType.CERT_VERIFY_PROC_PATH_BUILT:
        return writeParamsForCertVerifyProcPathBuilt;
      case EventType.CERT_CT_COMPLIANCE_CHECKED:
      case EventType.EV_CERT_CT_COMPLIANCE_CHECKED:
        return writeParamsForCheckedCertificates;
    }
    return null;
  }

  /**
   * Parses |hexStr| to an array of bytes, or returns null if the input is not a
   * valid hex string.
   */
  function tryParseHexToBytes(hexStr) {
    if ((hexStr.length % 2) !== 0) {
      return null;
    }

    const result = [];
    for (let i = 0; i < hexStr.length; i += 2) {
      const value = parseInt(hexStr.substr(i, 2), 16);
      if (isNaN(value)) {
        return null;
      }
      result.push(value);
    }

    return result;
  }

  /**
   * Parses a base64 encoded string to a Uint8Array of bytes.
   */
  function tryParseBase64ToBytes(b64Str) {
    let decodedStr;

    try {
      decodedStr = atob(b64Str);
    } catch (e) {
      return null;
    }

    return Uint8Array.from(decodedStr, c => c.charCodeAt(0));
  }

  /**
   * Default parameter writer that outputs a visualization of field named |key|
   * with value |value| to |out|.
   */
  function defaultWriteParameter(key, value, out) {
    if (key === 'headers' && value instanceof Array) {
      out.writeArrowIndentedLines(value);
      return;
    }

    if (key === 'bytes' && typeof value === 'string') {
      const bytes = tryParseBase64ToBytes(value);
      if (bytes) {
        if (out.isForSearch()) {
          writeBytesAsSearchableString(bytes, out);
        } else {
          out.writeArrowKey(key);
          writeHexString(bytes, out);
        }
        return;
      }
    }

    // Handle source_dependency entries - add link and map source type to
    // string.
    if (key === 'source_dependency' && typeof value === 'object') {
      const link = '#events&s=' + value.id;
      const valueStr = value.id + ' (' + EventSourceTypeNames[value.type] + ')';
      out.writeArrowKeyValue(key, valueStr, link);
      return;
    }

    if (key === 'net_error' && typeof value === 'number') {
      const valueStr = value + ' (' + netErrorToString(value) + ')';
      out.writeArrowKeyValue(key, valueStr);
      return;
    }

    if (key === 'quic_error' && typeof value === 'number') {
      const valueStr = value + ' (' + quicErrorToString(value) + ')';
      out.writeArrowKeyValue(key, valueStr);
      return;
    }

    if (key === 'quic_crypto_handshake_message' && typeof value === 'string') {
      const lines = value.split('\n');
      out.writeArrowIndentedLines(lines);
      return;
    }

    if (key === 'quic_rst_stream_error' && typeof value === 'number') {
      const valueStr = value + ' (' + quicRstStreamErrorToString(value) + ')';
      out.writeArrowKeyValue(key, valueStr);
      return;
    }

    if (key === 'load_flags' && typeof value === 'number') {
      const valueStr = value + ' (' + getLoadFlagSymbolicString(value) + ')';
      out.writeArrowKeyValue(key, valueStr);
      return;
    }

    if (key === 'load_state' && typeof value === 'number') {
      const valueStr = value + ' (' + getKeyWithValue(LoadState, value) + ')';
      out.writeArrowKeyValue(key, valueStr);
      return;
    }

    // Otherwise just default to JSON formatting of the value.
    out.writeArrowKeyJSONValue(key, value);
  }

  /**
   * Returns the set of LoadFlags that make up the integer |loadFlag|.
   * For example: getLoadFlagSymbolicString(
   */
  function getLoadFlagSymbolicString(loadFlag) {
    return getSymbolicString(
        loadFlag, LoadFlag, getKeyWithValue(LoadFlag, loadFlag));
  }

  /**
   * Returns the set of CertStatusFlags that make up the integer
   * |certStatusFlag|
   */
  function getCertStatusFlagSymbolicString(certStatusFlag) {
    return getSymbolicString(certStatusFlag, CertStatusFlag, '');
  }

  /**
   * Returns the set of CertVerifierFlags that make up the integer
   * |certVerifierFlags|
   */
  function getCertVerifierFlagsSymbolicString(certVerifierFlags) {
    return getSymbolicString(certVerifierFlags, CertVerifierFlags, '');
  }

  /**
   * Returns the set of CertVerifyFlags that make up the integer
   * |certVerifyFlags|
   */
  function getCertVerifyFlagsSymbolicString(certVerifyFlags) {
    return getSymbolicString(certVerifyFlags, CertVerifyFlags, '');
  }

  /**
   * Returns the symbolic name of the |digestPolicy| enum value.
   */
  function getCertPathBuilderDigestPolicySymbolicString(digestPolicy) {
    return getKeyWithValue(CertPathBuilderDigestPolicy, digestPolicy);
  }

  /**
   * Returns a string representing the flags composing the given bitmask.
   */
  function getSymbolicString(bitmask, valueToName, zeroName) {
    const matchingFlagNames = [];

    for (const k in valueToName) {
      if (bitmask & valueToName[k]) {
        matchingFlagNames.push(k);
      }
    }

    // If no flags were matched, returns a special value.
    if (matchingFlagNames.length === 0) {
      return zeroName;
    }

    return matchingFlagNames.join(' | ');
  }

  /**
   * TODO(eroman): get rid of this, as it is only used by 1 callsite.
   *
   * Indent |lines| by |start|.
   *
   * For example, if |start| = ' -> ' and |lines| = ['line1', 'line2', 'line3']
   * the output will be:
   *
   *   " -> line1\n" +
   *   "    line2\n" +
   *   "    line3"
   */
  function indentLines(start, lines) {
    return start + lines.join('\n' + makeRepeatedString(' ', start.length));
  }

  /**
   * If entry.param.headers exists and is an object other than an array,
   * converts
   * it into an array and returns a new entry.  Otherwise, just returns the
   * original entry.
   */
  function reformatHeaders(entry) {
    // If there are no headers, or it is not an object other than an array,
    // return |entry| without modification.
    if (!entry.params || entry.params.headers === undefined ||
        typeof entry.params.headers !== 'object' ||
        entry.params.headers instanceof Array) {
      return entry;
    }

    // Duplicate the top level object, and |entry.params|, so the original
    // object
    // will not be modified.
    entry = shallowCloneObject(entry);
    entry.params = shallowCloneObject(entry.params);

    // Convert headers to an array.
    const headers = [];
    for (const key in entry.params.headers) {
      headers.push(key + ': ' + entry.params.headers[key]);
    }
    entry.params.headers = headers;

    return entry;
  }

  /**
   * Outputs the request header parameters of |entry| to |out|.
   */
  function writeParamsForRequestHeaders(entry, out, consumedParams) {
    const params = entry.params;

    if (!(typeof params.line === 'string') ||
        !(params.headers instanceof Array)) {
      // Unrecognized params.
      return;
    }

    // Strip the trailing CRLF that params.line contains.
    const lineWithoutCRLF = params.line.replace(/\r\n$/g, '');
    out.writeArrowIndentedLines([lineWithoutCRLF].concat(params.headers));

    consumedParams.line = true;
    consumedParams.headers = true;
  }

  function writeIndentedMultiLineParam(lines, out, consumedParams, paramName) {
    out.writeArrowKey(paramName);
    out.writeSpaceIndentedLines(8, lines);
    consumedParams[paramName] = true;
  }

  function writeCertificateParam(
      certsContainer, out, consumedParams, paramName) {
    if (certsContainer.certificates instanceof Array) {
      const certs =
          certsContainer.certificates.reduce(function(previous, current) {
            return previous.concat(current.split('\n'));
          }, []);
      writeIndentedMultiLineParam(certs, out, consumedParams, paramName);
    }
  }

  /**
   * Outputs the certificate parameters of |entry| to |out|.
   */
  function writeParamsForCertificates(entry, out, consumedParams) {
    writeCertificateParam(entry.params, out, consumedParams, 'certificates');

    if (typeof(entry.params.verified_cert) === 'object') {
      writeCertificateParam(
          entry.params.verified_cert, out, consumedParams, 'verified_cert');
    }

    if (typeof(entry.params.ocsp_response) === 'string') {
      writeIndentedMultiLineParam(
          entry.params.ocsp_response.split('\n'), out, consumedParams,
          'ocsp_response');
    }

    if (typeof(entry.params.sct_list) === 'string') {
      writeIndentedMultiLineParam(
          entry.params.sct_list.split('\n'), out, consumedParams, 'sct_list');
    }

    if (typeof(entry.params.cert_status) === 'number') {
      const valueStr = entry.params.cert_status + ' (' +
          getCertStatusFlagSymbolicString(entry.params.cert_status) + ')';
      out.writeArrowKeyValue('cert_status', valueStr);
      consumedParams.cert_status = true;
    }

    if (typeof(entry.params.verifier_flags) === 'number') {
      const valueStr = entry.params.verifier_flags + ' (' +
          getCertVerifierFlagsSymbolicString(entry.params.verifier_flags) + ')';
      out.writeArrowKeyValue('verifier_flags', valueStr);
      consumedParams.verifier_flags = true;
    }
  }

  function writeParamsForCertVerifyProc(entry, out, consumedParams) {
    if (typeof(entry.params.cert_status) === 'number') {
      const valueStr = entry.params.cert_status + ' (' +
          getCertStatusFlagSymbolicString(entry.params.cert_status) + ')';
      out.writeArrowKeyValue('cert_status', valueStr);
      consumedParams.cert_status = true;
    }

    if (typeof(entry.params.verify_flags) === 'number') {
      const valueStr = entry.params.verify_flags + ' (' +
          getCertVerifyFlagsSymbolicString(entry.params.verify_flags) + ')';
      out.writeArrowKeyValue('verify_flags', valueStr);
      consumedParams.verify_flags = true;
    }
  }

  function writeParamsForCertVerifyProcPathBuildAttempt(entry, out,
                                                        consumedParams) {
    if (typeof(entry.params.digest_policy) === 'number') {
      const valueStr = entry.params.digest_policy + ' (' +
          getCertPathBuilderDigestPolicySymbolicString(
            entry.params.digest_policy) + ')';
      out.writeArrowKeyValue('digest_policy', valueStr);
      consumedParams.digest_policy = true;
    }
  }

  function writeParamsForCertVerifyProcPathBuilt(entry, out, consumedParams) {
    if (typeof(entry.params.errors) === 'string') {
      writeIndentedMultiLineParam(
          entry.params.errors.split('\n'), out, consumedParams,
          'errors');
    }
  }

  function writeParamsForCheckedCertificates(entry, out, consumedParams) {
    if (typeof(entry.params.certificate) === 'object') {
      writeCertificateParam(
          entry.params.certificate, out, consumedParams, 'certificate');
    }
  }

  function writeParamsForProxyConfigChanged(entry, out, consumedParams) {
    const params = entry.params;

    if (typeof params.new_config !== 'object') {
      // Unrecognized params.
      return;
    }

    if (typeof params.old_config === 'object') {
      const oldConfigString = proxySettingsToString(params.old_config);
      // The previous configuration may not be present in the case of
      // the initial proxy settings fetch.
      out.writeArrowKey('old_config');

      out.writeSpaceIndentedLines(8, oldConfigString.split('\n'));

      consumedParams.old_config = true;
    }

    const newConfigString = proxySettingsToString(params.new_config);
    out.writeArrowKey('new_config');
    out.writeSpaceIndentedLines(8, newConfigString.split('\n'));

    consumedParams.new_config = true;
  }

  function getTextForEvent(entry) {
    let text = '';

    if (entry.isBegin() && canCollapseBeginWithEnd(entry)) {
      // Don't prefix with '+' if we are going to collapse the END event.
      text = ' ';
    } else if (entry.isBegin()) {
      text = '+' + text;
    } else if (entry.isEnd()) {
      text = '-' + text;
    } else {
      text = ' ';
    }

    text += EventTypeNames[entry.orig.type];
    return text;
  }

  proxySettingsToString = function(config) {
    if (!config) {
      return '';
    }

    // TODO(eroman): if |config| has unexpected properties, print it as JSON
    //               rather than hide them.

    function getProxyListString(proxies) {
      // Older versions of Chrome would set these values as strings, whereas
      // newer
      // logs use arrays.
      // TODO(eroman): This behavior changed in M27. Support for older logs can
      //               safely be removed circa M29.
      if (Array.isArray(proxies)) {
        const listString = proxies.join(', ');
        if (proxies.length > 1) {
          return '[' + listString + ']';
        }
        return listString;
      }
      return proxies;
    }

    // The proxy settings specify up to three major fallback choices
    // (auto-detect, custom pac url, or manual settings).
    // We enumerate these to a list so we can later number them.
    const modes = [];

    // Output any automatic settings.
    if (config.auto_detect) {
      modes.push(['Auto-detect']);
    }
    if (config.pac_url) {
      modes.push(['PAC script: ' + config.pac_url]);
    }

    // Output any manual settings.
    if (config.single_proxy || config.proxy_per_scheme) {
      const lines = [];

      if (config.single_proxy) {
        lines.push('Proxy server: ' + getProxyListString(config.single_proxy));
      } else if (config.proxy_per_scheme) {
        for (const urlScheme in config.proxy_per_scheme) {
          if (urlScheme !== 'fallback') {
            lines.push(
                'Proxy server for ' + urlScheme.toUpperCase() + ': ' +
                getProxyListString(config.proxy_per_scheme[urlScheme]));
          }
        }
        if (config.proxy_per_scheme.fallback) {
          lines.push(
              'Proxy server for everything else: ' +
              getProxyListString(config.proxy_per_scheme.fallback));
        }
      }

      // Output any proxy bypass rules.
      if (config.bypass_list) {
        if (config.reverse_bypass) {
          lines.push('Reversed bypass list: ');
        } else {
          lines.push('Bypass list: ');
        }

        for (let i = 0; i < config.bypass_list.length; ++i) {
          lines.push('  ' + config.bypass_list[i]);
        }
      }

      modes.push(lines);
    }

    const result = [];
    if (modes.length < 1) {
      // If we didn't find any proxy settings modes, we are using DIRECT.
      result.push('Use DIRECT connections.');
    } else if (modes.length === 1) {
      // If there was just one mode, don't bother numbering it.
      result.push(modes[0].join('\n'));
    } else {
      // Otherwise concatenate all of the modes into a numbered list
      // (which correspond with the fallback order).
      for (let i = 0; i < modes.length; ++i) {
        result.push(indentLines('(' + (i + 1) + ') ', modes[i]));
      }
    }

    if (config.source !== undefined && config.source !== 'UNKNOWN') {
      result.push('Source: ' + config.source);
    }

    return result.join('\n');
  };

  // End of anonymous namespace.
})();

